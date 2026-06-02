#pragma once

#include "asr_client.h"
#include "backend_monitor.h"
#include "clipboard_writer.h"
#include "global_shortcut.h"
#include "history_store.h"
#include "recording_overlay.h"
#include "tray_controller.h"
#include "wav_recorder.h"

#include <QElapsedTimer>
#include <QSettings>

class AppController : public QObject {
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);
    void start();

private:
    static constexpr qint64 MinRecordingMs = 700;

    void setState(AppState state);
    void toggleRecording();
    void onShortcutPressed();
    void onShortcutReleased();
    void startRecording();
    void stopRecording(bool allowDiscard);
    void editShortcut();
    QString nextWavPath() const;
    AsrConfig loadConfig() const;
    void saveShortcut(const QKeySequence &shortcut);

    AsrConfig config_;
    AppState appState_ = AppState::Idle;
    BackendState backendState_ = BackendState::Unknown;
    TrayController tray_;
    BackendMonitor backend_;
    WavRecorder recorder_;
    AsrClient asr_;
    ClipboardWriter clipboard_;
    GlobalShortcut shortcut_;
    HistoryStore history_;
    RecordingOverlay overlay_;
    QElapsedTimer recordingTimer_;
};
