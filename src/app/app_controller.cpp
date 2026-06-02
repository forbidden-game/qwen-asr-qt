#include "app/app_controller.h"

#include <QApplication>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QMessageBox>
#include <QStandardPaths>
#include <QVBoxLayout>

AppController::AppController(QObject *parent)
    : QObject(parent)
    , settings_(loadAppSettings())
    , backend_(settings_, this)
    , asr_(settings_, this)
{
    connect(&tray_, &TrayController::toggleRecordingRequested, this, &AppController::toggleRecording);
    connect(&tray_, &TrayController::backendCheckRequested, &backend_, &BackendMonitor::checkNow);
    connect(&tray_, &TrayController::shortcutEditRequested, this, &AppController::editShortcut);
    connect(&tray_, &TrayController::quitRequested, qApp, &QApplication::quit);

    connect(&backend_, &BackendMonitor::backendStateChanged, this, [this](BackendState state) {
        backendState_ = state;
        tray_.updateState(appState_, backendState_);
    });
    connect(&recorder_, &WavRecorder::errorOccurred, this, [this](const QString &message) {
        setState(AppState::Error);
        tray_.notifyError(message);
    });
    connect(&recorder_, &WavRecorder::levelChanged, &overlay_, &RecordingOverlay::setInputLevel);
    connect(&asr_, &AsrClient::finished, this, [this](const HistoryItem &item) {
        history_.add(item);
        setState(AppState::Done);
        if (clipboard_.setText(item.text)) {
            tray_.notifyCopied(item.text);
        } else {
            tray_.notifyError(QStringLiteral("转写完成，但写入剪贴板失败"));
        }
    });
    connect(&asr_, &AsrClient::errorOccurred, this, [this](const QString &message) {
        setState(AppState::Error);
        tray_.notifyError(message);
    });
    connect(&shortcut_, &GlobalShortcut::pressed, this, &AppController::onShortcutPressed);
    connect(&shortcut_, &GlobalShortcut::released, this, &AppController::onShortcutReleased);
    connect(&shortcut_, &GlobalShortcut::errorOccurred, this, [this](const QString &message) {
        tray_.notifyError(message);
    });
    connect(&history_, &HistoryStore::changed, &tray_, &TrayController::updateHistory);
}

void AppController::start()
{
    history_.load();
    shortcut_.registerShortcut(settings_.shortcut);
    tray_.updateShortcut(shortcut_.shortcut().isEmpty() ? settings_.shortcut : shortcut_.shortcut());
    tray_.updateHistory(history_.recent());
    tray_.updateState(appState_, backendState_);
    tray_.show();
    backend_.start();
}

void AppController::setState(AppState state)
{
    appState_ = state;
    tray_.updateState(appState_, backendState_);
}

void AppController::toggleRecording()
{
    if (appState_ == AppState::Transcribing) {
        tray_.notifyError(QStringLiteral("正在转写，稍等一下"));
        return;
    }
    if (appState_ == AppState::Recording) {
        stopRecording(false);
        return;
    }
    if (backendState_ == BackendState::Disconnected || backendState_ == BackendState::ModelMissing) {
        tray_.notifyError(QStringLiteral("后端未连接或模型未加载"));
        backend_.checkNow();
        return;
    }
    startRecording();
}

void AppController::onShortcutPressed()
{
    if (appState_ != AppState::Recording) {
        startRecording();
    }
}

void AppController::onShortcutReleased()
{
    if (appState_ == AppState::Recording) {
        stopRecording(true);
    }
}

void AppController::startRecording()
{
    if (appState_ == AppState::Transcribing) {
        tray_.notifyError(QStringLiteral("正在转写，稍等一下"));
        return;
    }
    if (backendState_ == BackendState::Disconnected || backendState_ == BackendState::ModelMissing) {
        tray_.notifyError(QStringLiteral("后端未连接或模型未加载"));
        backend_.checkNow();
        return;
    }
    if (recorder_.start(nextWavPath(), settings_.audio.sampleRate, settings_.audio.channels)) {
        recordingTimer_.restart();
        overlay_.showRecording();
        setState(AppState::Recording);
    }
}

void AppController::stopRecording(bool allowDiscard)
{
    const qint64 elapsedMs = recordingTimer_.isValid() ? recordingTimer_.elapsed() : 0;
    const QString path = recorder_.stop();
    overlay_.hideRecording();
    if (path.isEmpty()) {
        setState(AppState::Error);
        tray_.notifyError(QStringLiteral("没有可用录音文件"));
        return;
    }
    if (allowDiscard && elapsedMs < MinRecordingMs) {
        QFile::remove(path);
        setState(AppState::Idle);
        tray_.notifyError(QStringLiteral("录音太短，未发送"));
        return;
    }
    setState(AppState::Transcribing);
    asr_.transcribe(path);
}

void AppController::editShortcut()
{
    QDialog dialog;
    dialog.setWindowTitle(QStringLiteral("设置快捷键"));
    auto *layout = new QVBoxLayout(&dialog);
    layout->addWidget(new QLabel(QStringLiteral("设置开始/停止录音的全局快捷键："), &dialog));
    auto *edit = new QKeySequenceEdit(settings_.shortcut, &dialog);
    layout->addWidget(edit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted || edit->keySequence().isEmpty()) {
        return;
    }

    if (!shortcut_.registerShortcut(edit->keySequence())) {
        QMessageBox::warning(nullptr, QStringLiteral("快捷键失败"), QStringLiteral("无法注册这个全局快捷键，可能已被占用。"));
        return;
    }
    settings_.shortcut = shortcut_.shortcut();
    saveShortcutSetting(settings_.shortcut);
    tray_.updateShortcut(settings_.shortcut);
}

QString AppController::nextWavPath() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/qwen-asr-qt");
    QDir().mkpath(base);
    return base + QStringLiteral("/recording-%1.wav").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz")));
}
