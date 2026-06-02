#pragma once

#include <QAudioFormat>
#include <QAudioSource>
#include <QElapsedTimer>
#include <QFile>
#include <QIODevice>
#include <QObject>
#include <QString>

class WavRecorder : public QObject {
    Q_OBJECT

public:
    explicit WavRecorder(QObject *parent = nullptr);

    bool start(const QString &wavPath, int sampleRate, int channels);
    QString stop();
    bool isRecording() const;

signals:
    void errorOccurred(const QString &message);
    void levelChanged(qreal level);

private:
    bool writeHeader(quint32 dataBytes);
    int sampleBytes() const;
    quint16 wavFormatTag() const;

    QFile file_;
    QAudioFormat format_;
    QAudioSource *source_ = nullptr;
    QIODevice *captureDevice_ = nullptr;
    QString wavPath_;
};
