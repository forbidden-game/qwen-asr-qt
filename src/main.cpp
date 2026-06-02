#include "app_controller.h"
#include "recording_overlay.h"
#include "transcript_cleaner.h"

#include <QApplication>
#include <QSystemTrayIcon>
#include <QTextStream>
#include <QTimer>
#include <QVector>

#include <cmath>

namespace {

struct CleanerCase {
    QString input;
    QString expected;
};

int runCleanerSelfTest()
{
    const QVector<CleanerCase> cases = {
        {
            QStringLiteral("language Chinese<asr_text>嗯，那后面我如果想把整个的仓库哈，就是我把我现在这一套直接，呃公开到GitHub上，嗯，这个大家怎么用起来比较简单啊？"),
            QStringLiteral("那后面我如果想把整个的仓库哈，就是我把我现在这一套直接公开到GitHub上，这个大家怎么用起来比较简单啊？"),
        },
        {
            QStringLiteral("language Chinese<asr_text>你有没有给嗯模型提示词呀？"),
            QStringLiteral("你有没有给模型提示词呀？"),
        },
        {
            QStringLiteral("language Chinese<asr_text>另外，你再检查一下你当前过滤语气词的方案。你看这个“呃”又出来了。"),
            QStringLiteral("另外，你再检查一下你当前过滤语气词的方案。你看这个“呃”又出来了。"),
        },
        {
            QStringLiteral("language Chinese<asr_text>现在这个波形，嗯，我觉得还是要跟音量有关。你现在这个特效有点，呃，太固定了。"),
            QStringLiteral("现在这个波形，我觉得还是要跟音量有关。你现在这个特效有点，太固定了。"),
        },
    };

    QTextStream err(stderr);
    for (const CleanerCase &testCase : cases) {
        const QString actual = TranscriptCleaner::clean(testCase.input);
        if (actual != testCase.expected) {
            err << "Cleaner self-test failed.\n"
                << "Input: " << testCase.input << "\n"
                << "Expected: " << testCase.expected << "\n"
                << "Actual: " << actual << "\n";
            return 2;
        }
    }
    return 0;
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("qwen-asr-qt"));
    QApplication::setApplicationDisplayName(QStringLiteral("Qwen ASR"));
    QApplication::setOrganizationName(QStringLiteral("XiezhaoPan"));
    QApplication::setQuitOnLastWindowClosed(false);

    const QStringList args = QApplication::arguments();
    if (args.contains(QStringLiteral("--self-test-cleaner"))) {
        return runCleanerSelfTest();
    }

    if (args.contains(QStringLiteral("--preview-overlay"))) {
        RecordingOverlay overlay;
        overlay.showRecording();
        QTimer meter;
        qreal phase = 0.0;
        QObject::connect(&meter, &QTimer::timeout, &overlay, [&overlay, &phase]() {
            phase += 0.18;
            const qreal wave = (std::sin(phase) + 1.0) / 2.0;
            overlay.setInputLevel(0.08 + wave * wave * 0.82);
        });
        meter.start(33);
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
