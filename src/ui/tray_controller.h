#pragma once

#include "app/app_state.h"
#include "domain/asr_types.h"

#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>
#include <QWidget>

class TrayController : public QObject {
    Q_OBJECT

public:
    explicit TrayController(QObject *parent = nullptr);

    void show();
    void updateState(AppState appState, BackendState backendState);
    void updateHistory(const QVector<HistoryItem> &items);
    void updateShortcut(const QKeySequence &shortcut);
    void notifyCopied(const QString &text);
    void notifyError(const QString &message);

signals:
    void toggleRecordingRequested();
    void statusPanelRequested();
    void backendCheckRequested();
    void shortcutEditRequested();
    void quitRequested();

private:
    void createPanel();
    void showStatusPanel();
    void refreshPanel();
    QIcon iconForState() const;

    QSystemTrayIcon tray_;
    QMenu menu_;
    QAction *toggleAction_ = nullptr;
    QAction *statusAction_ = nullptr;
    QAction *checkBackendAction_ = nullptr;
    QAction *shortcutAction_ = nullptr;
    QAction *quitAction_ = nullptr;

    QWidget *panel_ = nullptr;
    QLabel *appStateLabel_ = nullptr;
    QLabel *backendStateLabel_ = nullptr;
    QLabel *shortcutLabel_ = nullptr;
    QListWidget *historyList_ = nullptr;

    AppState appState_ = AppState::Idle;
    BackendState backendState_ = BackendState::Unknown;
    QKeySequence shortcut_;
    QVector<HistoryItem> history_;
};
