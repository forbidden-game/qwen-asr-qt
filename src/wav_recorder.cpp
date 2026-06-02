#include "wav_recorder.h"

#include <QAudioDevice>
#include <QDataStream>
#include <QDir>
#include <QMetaObject>
#include <QMediaDevices>
#include <QPointer>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>

namespace {

double sampleValue(const char *data, qsizetype index, QAudioFormat::SampleFormat sampleFormat)
{
    switch (sampleFormat) {
    case QAudioFormat::UInt8:
        return (static_cast<unsigned char>(data[index]) - 128) / 128.0;
    case QAudioFormat::Int16: {
        qint16 sample = 0;
        std::memcpy(&sample, data + index * 2, sizeof(sample));
        return sample / 32768.0;
    }
    case QAudioFormat::Int32: {
        qint32 sample = 0;
        std::memcpy(&sample, data + index * 4, sizeof(sample));
        return sample / 2147483648.0;
    }
    case QAudioFormat::Float: {
        float sample = 0.0F;
        std::memcpy(&sample, data + index * 4, sizeof(sample));
        return std::clamp(static_cast<double>(sample), -1.0, 1.0);
    }
    case QAudioFormat::Unknown:
        return 0.0;
    }
    return 0.0;
}

qreal audioLevel(const char *data, qint64 len, const QAudioFormat &format)
{
    const int bytesPerSample = std::max(1, format.bytesPerSample());
    const qint64 sampleCount = len / bytesPerSample;
    if (sampleCount <= 0) {
        return 0.0;
    }

    double sumSquares = 0.0;
    for (qint64 i = 0; i < sampleCount; ++i) {
        const double value = sampleValue(data, i, format.sampleFormat());
        sumSquares += value * value;
    }

    const double rms = std::sqrt(sumSquares / static_cast<double>(sampleCount));
    const double perceptual = std::sqrt(std::clamp(rms, 0.0, 1.0)) * 1.7;
    return qBound<qreal>(0.0, perceptual, 1.0);
}

class LevelCaptureDevice : public QIODevice {
public:
    LevelCaptureDevice(QFile *file, QAudioFormat format, std::function<void(qreal)> levelCallback, QObject *parent = nullptr)
        : QIODevice(parent)
        , file_(file)
        , format_(std::move(format))
        , levelCallback_(std::move(levelCallback))
    {
        levelTimer_.start();
    }

protected:
    qint64 readData(char *data, qint64 maxSize) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxSize);
        return 0;
    }

    qint64 writeData(const char *data, qint64 len) override
    {
        if (!file_) {
            return -1;
        }

        const qint64 written = file_->write(data, len);
        if (written > 0 && levelTimer_.elapsed() >= 30) {
            levelCallback_(audioLevel(data, written, format_));
            levelTimer_.restart();
        }
        return written;
    }

private:
    QFile *file_;
    QAudioFormat format_;
    std::function<void(qreal)> levelCallback_;
    QElapsedTimer levelTimer_;
};

} // namespace

WavRecorder::WavRecorder(QObject *parent)
    : QObject(parent)
{
}

bool WavRecorder::start(const QString &wavPath, int sampleRate, int channels)
{
    if (source_) {
        return false;
    }

    QAudioDevice device = QMediaDevices::defaultAudioInput();
    if (device.isNull()) {
        emit errorOccurred(QStringLiteral("没有可用的录音设备"));
        return false;
    }

    format_.setSampleRate(sampleRate);
    format_.setChannelCount(channels);
    format_.setSampleFormat(QAudioFormat::Int16);
    if (!device.isFormatSupported(format_)) {
        format_ = device.preferredFormat();
    }

    if (format_.sampleFormat() == QAudioFormat::Unknown) {
        emit errorOccurred(QStringLiteral("录音设备返回了不支持的音频格式"));
        return false;
    }

    QDir().mkpath(QFileInfo(wavPath).absolutePath());
    file_.setFileName(wavPath);
    if (!file_.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        emit errorOccurred(QStringLiteral("无法创建录音文件：%1").arg(file_.errorString()));
        return false;
    }

    wavPath_ = wavPath;
    writeHeader(0);

    source_ = new QAudioSource(device, format_, this);
    connect(source_, &QAudioSource::stateChanged, this, [this](QAudio::State state) {
        if (state == QAudio::StoppedState && source_ && source_->error() != QAudio::NoError) {
            emit errorOccurred(QStringLiteral("录音失败：%1").arg(static_cast<int>(source_->error())));
        }
    });
    QPointer<WavRecorder> recorder(this);
    captureDevice_ = new LevelCaptureDevice(&file_, format_, [recorder](qreal level) {
        if (!recorder) {
            return;
        }
        QMetaObject::invokeMethod(recorder, [recorder, level]() {
            if (recorder) {
                emit recorder->levelChanged(level);
            }
        }, Qt::QueuedConnection);
    }, this);
    captureDevice_->open(QIODevice::WriteOnly);
    source_->start(captureDevice_);
    return true;
}

QString WavRecorder::stop()
{
    if (!source_) {
        return {};
    }

    source_->stop();
    source_->deleteLater();
    source_ = nullptr;
    if (captureDevice_) {
        captureDevice_->close();
        delete captureDevice_;
        captureDevice_ = nullptr;
    }
    emit levelChanged(0.0);

    const quint32 dataBytes = static_cast<quint32>(qMax<qint64>(0, file_.size() - 44));
    file_.seek(0);
    writeHeader(dataBytes);
    file_.close();
    return wavPath_;
}

bool WavRecorder::isRecording() const
{
    return source_ != nullptr;
}

bool WavRecorder::writeHeader(quint32 dataBytes)
{
    QDataStream out(&file_);
    out.setByteOrder(QDataStream::LittleEndian);

    const quint16 channels = static_cast<quint16>(format_.channelCount());
    const quint32 sampleRate = static_cast<quint32>(format_.sampleRate());
    const quint16 bitsPerSample = static_cast<quint16>(sampleBytes() * 8);
    const quint16 blockAlign = static_cast<quint16>(channels * sampleBytes());
    const quint32 byteRate = sampleRate * blockAlign;

    file_.write("RIFF", 4);
    out << static_cast<quint32>(36 + dataBytes);
    file_.write("WAVE", 4);
    file_.write("fmt ", 4);
    out << static_cast<quint32>(16);
    out << wavFormatTag();
    out << channels;
    out << sampleRate;
    out << byteRate;
    out << blockAlign;
    out << bitsPerSample;
    file_.write("data", 4);
    out << dataBytes;
    return true;
}

int WavRecorder::sampleBytes() const
{
    switch (format_.sampleFormat()) {
    case QAudioFormat::UInt8:
        return 1;
    case QAudioFormat::Int16:
        return 2;
    case QAudioFormat::Int32:
    case QAudioFormat::Float:
        return 4;
    case QAudioFormat::Unknown:
        return 2;
    }
    return 2;
}

quint16 WavRecorder::wavFormatTag() const
{
    return format_.sampleFormat() == QAudioFormat::Float ? 3 : 1;
}
