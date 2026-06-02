#include "platform/clipboard_writer.h"

#include <QClipboard>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QGuiApplication>

ClipboardWriter::ClipboardWriter(QObject *parent)
    : QObject(parent)
{
}

bool ClipboardWriter::setText(const QString &text)
{
    bool wrote = false;

    if (QClipboard *clipboard = QGuiApplication::clipboard()) {
        clipboard->setText(text, QClipboard::Clipboard);
        if (clipboard->supportsSelection()) {
            clipboard->setText(text, QClipboard::Selection);
        }
        wrote = !clipboard->text(QClipboard::Clipboard).isEmpty();
    }

    QDBusInterface klipper(
        QStringLiteral("org.kde.klipper"),
        QStringLiteral("/klipper"),
        QStringLiteral("org.kde.klipper.klipper"),
        QDBusConnection::sessionBus());
    if (klipper.isValid()) {
        const QDBusMessage reply = klipper.call(QStringLiteral("setClipboardContents"), text);
        wrote = wrote || reply.type() != QDBusMessage::ErrorMessage;
    }

    return wrote;
}
