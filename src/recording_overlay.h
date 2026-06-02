#pragma once

#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>

class RecordingOverlay : public QWidget {
    Q_OBJECT

public:
    explicit RecordingOverlay(QWidget *parent = nullptr);

    void showRecording();
    void hideRecording();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void positionOnActiveScreen();

    QTimer timer_;
    QElapsedTimer clock_;
};
