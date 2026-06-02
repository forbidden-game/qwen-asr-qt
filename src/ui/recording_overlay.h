#pragma once

#include <QElapsedTimer>
#include <QTimer>
#include <QVector>
#include <QWidget>

class RecordingOverlay : public QWidget {
    Q_OBJECT

public:
    explicit RecordingOverlay(QWidget *parent = nullptr);

    void showRecording();
    void hideRecording();
    void setInputLevel(qreal level);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void positionOnActiveScreen();
    void tick();

    QTimer timer_;
    QElapsedTimer clock_;
    QVector<qreal> levels_;
    qreal targetLevel_ = 0.0;
    qreal displayLevel_ = 0.0;
};
