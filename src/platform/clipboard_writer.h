#pragma once

#include <QObject>
#include <QString>

class ClipboardWriter : public QObject {
    Q_OBJECT

public:
    explicit ClipboardWriter(QObject *parent = nullptr);

    bool setText(const QString &text);
};
