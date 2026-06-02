#include "platform/global_shortcut.h"

#include <KGlobalAccel>

GlobalShortcut::GlobalShortcut(QObject *parent)
    : QObject(parent)
    , action_(this)
{
    action_.setObjectName(QStringLiteral("toggle-recording"));
    action_.setText(QStringLiteral("Hold to record Qwen ASR"));

    connect(KGlobalAccel::self(), &KGlobalAccel::globalShortcutActiveChanged, this, [this](QAction *action, bool active) {
        if (action != &action_) {
            return;
        }
        if (active) {
            emit pressed();
        } else {
            emit released();
        }
    });
}

bool GlobalShortcut::registerShortcut(const QKeySequence &shortcut)
{
    const QList<QKeySequence> shortcuts{shortcut};
    KGlobalAccel *accel = KGlobalAccel::self();
    accel->setDefaultShortcut(&action_, shortcuts, KGlobalAccel::NoAutoloading);
    const bool ok = accel->setShortcut(&action_, shortcuts, KGlobalAccel::NoAutoloading);
    const QList<QKeySequence> actual = accel->shortcut(&action_);
    if (!ok || actual.isEmpty()) {
        emit errorOccurred(QStringLiteral("全局快捷键注册失败：%1").arg(shortcut.toString(QKeySequence::NativeText)));
        return false;
    }
    shortcut_ = actual.first();
    return true;
}

QKeySequence GlobalShortcut::shortcut() const
{
    return shortcut_;
}
