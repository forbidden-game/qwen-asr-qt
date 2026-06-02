#pragma once

#include <QAction>
#include <QKeySequence>
#include <QObject>

class GlobalShortcut : public QObject {
    Q_OBJECT

public:
    explicit GlobalShortcut(QObject *parent = nullptr);

    bool registerShortcut(const QKeySequence &shortcut);
    QKeySequence shortcut() const;

signals:
    void activated();
    void pressed();
    void released();
    void errorOccurred(const QString &message);

private:
    QAction action_;
    QKeySequence shortcut_;
};
