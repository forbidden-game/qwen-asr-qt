#include "wav_recorder.h"

#include <QAudioDevice>
#include <QDataStream>
#include <QDir>
#include <QMediaDevices>

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
    source_->start(&file_);
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
