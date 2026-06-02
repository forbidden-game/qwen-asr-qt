#pragma once

#include <QString>

namespace TranscriptCleaner {

QString clean(const QString &raw);
QString removeFillers(const QString &text);
QString normalizePunctuation(const QString &text);

} // namespace TranscriptCleaner
