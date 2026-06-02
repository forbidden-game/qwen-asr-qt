#include "ui/tray_controller.h"

#include <QApplication>
#include <QCursor>
#include <QDateTime>
#include <QHBoxLayout>
#include <QStyle>
#include <QVBoxLayout>

TrayController::TrayController(QObject *parent)
    : QObject(parent)
{
    toggleAction_ = menu_.addAction(QStringLiteral("开始录音"));
    statusAction_ = menu_.addAction(QStringLiteral("打开状态面板"));
    checkBackendAction_ = menu_.addAction(QStringLiteral("重试连接后端"));
    shortcutAction_ = menu_.addAction(QStringLiteral("设置快捷键..."));
    menu_.addSeparator();
    quitAction_ = menu_.addAction(QStringLiteral("退出"));

    connect(toggleAction_, &QAction::triggered, this, &TrayController::toggleRecordingRequested);
    connect(statusAction_, &QAction::triggered, this, &TrayController::statusPanelRequested);
    connect(checkBackendAction_, &QAction::triggered, this, &TrayController::backendCheckRequested);
    connect(shortcutAction_, &QAction::triggered, this, &TrayController::shortcutEditRequested);
    connect(quitAction_, &QAction::triggered, this, &TrayController::quitRequested);

    tray_.setContextMenu(&menu_);
    tray_.setToolTip(QStringLiteral("Qwen ASR"));
    connect(&tray_, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            showStatusPanel();
        }
    });
    connect(this, &TrayController::statusPanelRequested, this, &TrayController::showStatusPanel);
}

void TrayController::show()
{
    tray_.setIcon(iconForState());
    tray_.show();
}

void TrayController::updateState(AppState appState, BackendState backendState)
{
    appState_ = appState;
    backendState_ = backendState;
    toggleAction_->setText(appState_ == AppState::Recording ? QStringLiteral("停止并转写") : QStringLiteral("开始录音"));
    tray_.setIcon(iconForState());
    tray_.setToolTip(QStringLiteral("Qwen ASR - %1 / %2").arg(appStateText(appState_), backendStateText(backendState_)));
    refreshPanel();
}

void TrayController::updateHistory(const QVector<HistoryItem> &items)
{
    history_ = items;
    refreshPanel();
}

void TrayController::updateShortcut(const QKeySequence &shortcut)
{
    shortcut_ = shortcut;
    refreshPanel();
}

void TrayController::notifyCopied(const QString &text)
{
    const QString preview = text.left(80);
    tray_.showMessage(QStringLiteral("ASR 已复制"), preview, QSystemTrayIcon::Information, 2500);
}

void TrayController::notifyError(const QString &message)
{
    tray_.showMessage(QStringLiteral("Qwen ASR 错误"), message, QSystemTrayIcon::Warning, 3500);
}

void TrayController::createPanel()
{
    if (panel_) {
        return;
    }

    panel_ = new QWidget(nullptr, Qt::Tool | Qt::WindowStaysOnTopHint);
    panel_->setWindowTitle(QStringLiteral("Qwen ASR"));
    panel_->setMinimumWidth(360);

    auto *layout = new QVBoxLayout(panel_);
    appStateLabel_ = new QLabel(panel_);
    backendStateLabel_ = new QLabel(panel_);
    shortcutLabel_ = new QLabel(panel_);
    historyList_ = new QListWidget(panel_);
    historyList_->setSelectionMode(QAbstractItemView::NoSelection);
    historyList_->setFocusPolicy(Qt::NoFocus);

    layout->addWidget(appStateLabel_);
    layout->addWidget(backendStateLabel_);
    layout->addWidget(shortcutLabel_);
    layout->addWidget(new QLabel(QStringLiteral("最近转写历史"), panel_));
    layout->addWidget(historyList_);
    refreshPanel();
}

void TrayController::showStatusPanel()
{
    createPanel();
    panel_->move(QCursor::pos() - QPoint(panel_->width() / 2, panel_->height() + 12));
    panel_->show();
    panel_->raise();
    panel_->activateWindow();
}

void TrayController::refreshPanel()
{
    if (!panel_) {
        return;
    }

    appStateLabel_->setText(QStringLiteral("状态：%1").arg(appStateText(appState_)));
    backendStateLabel_->setText(QStringLiteral("后端：%1").arg(backendStateText(backendState_)));
    shortcutLabel_->setText(QStringLiteral("快捷键：%1").arg(shortcut_.toString(QKeySequence::NativeText)));

    historyList_->clear();
    for (const HistoryItem &item : history_) {
        QString text = item.text.simplified();
        if (text.size() > 80) {
            text = text.left(80) + QStringLiteral("...");
        }
        historyList_->addItem(QStringLiteral("%1  %2  (%3 ms)")
            .arg(item.createdAt.toString(QStringLiteral("HH:mm:ss")), text)
            .arg(item.elapsedMs));
    }
    if (history_.isEmpty()) {
        historyList_->addItem(QStringLiteral("暂无历史"));
    }
}

QIcon TrayController::iconForState() const
{
    QStyle *style = QApplication::style();
    switch (appState_) {
    case AppState::Recording:
        return style->standardIcon(QStyle::SP_MediaPlay);
    case AppState::Transcribing:
        return style->standardIcon(QStyle::SP_BrowserReload);
    case AppState::Error:
        return style->standardIcon(QStyle::SP_MessageBoxWarning);
    case AppState::Done:
    case AppState::Idle:
        return backendState_ == BackendState::Connected
            ? style->standardIcon(QStyle::SP_DialogApplyButton)
            : style->standardIcon(QStyle::SP_MessageBoxInformation);
    }
    return style->standardIcon(QStyle::SP_ComputerIcon);
}
