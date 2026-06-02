#include "app_controller.h"
#include "recording_overlay.h"

#include <QApplication>
#include <QSystemTrayIcon>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("qwen-asr-qt"));
    QApplication::setApplicationDisplayName(QStringLiteral("Qwen ASR"));
    QApplication::setOrganizationName(QStringLiteral("XiezhaoPan"));
    QApplication::setQuitOnLastWindowClosed(false);

    const QStringList args = QApplication::arguments();
    if (args.contains(QStringLiteral("--preview-overlay"))) {
        RecordingOverlay overlay;
        overlay.showRecording();
        QTimer::singleShot(3000, &app, &QApplication::quit);
        return app.exec();
    }

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return 1;
    }

    AppController controller;
    controller.start();
    return app.exec();
}
