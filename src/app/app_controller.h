#pragma once

#include "app/app_settings.h"
#include "app/app_state.h"
#include "platform/clipboard_writer.h"
#include "platform/global_shortcut.h"
#include "services/asr_client.h"
#include "services/backend_monitor.h"
#include "services/history_store.h"
#include "services/wav_recorder.h"
#include "ui/recording_overlay.h"
#include "ui/tray_controller.h"

#include <QElapsedTimer>

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

    AppSettings settings_;
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
