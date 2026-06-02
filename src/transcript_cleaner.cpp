#include "transcript_cleaner.h"

#include <QList>
#include <QPair>
#include <QRegularExpression>

namespace {

using PlaceholderPair = QPair<QString, QString>;

QRegularExpression::PatternOptions regexOptions()
{
    return QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption;
}

QList<PlaceholderPair> protectQuotedText(QString &text)
{
    const QRegularExpression quotePattern(
        QStringLiteral(R"qasar(([“「『《〈][^”」』》〉]{0,80}[”」』》〉]|"[^"]{0,80}"))qasar"),
        regexOptions());

    QList<PlaceholderPair> placeholders;
    QRegularExpressionMatchIterator matches = quotePattern.globalMatch(text);
    int offset = 0;
    while (matches.hasNext()) {
        const QRegularExpressionMatch match = matches.next();
        const QString quoted = match.captured(0);
        const QString placeholder = QStringLiteral("__QASR_QUOTE_%1__").arg(placeholders.size());
        const int start = match.capturedStart() + offset;
        text.replace(start, match.capturedLength(), placeholder);
        offset += placeholder.size() - match.capturedLength();
        placeholders.append({placeholder, quoted});
    }
    return placeholders;
}

void restoreQuotedText(QString &text, const QList<PlaceholderPair> &placeholders)
{
    for (const PlaceholderPair &placeholder : placeholders) {
        text.replace(placeholder.first, placeholder.second);
    }
}

void replaceRepeated(QString &text, const QRegularExpression &pattern, const QString &replacement)
{
    QRegularExpressionMatch match;
    while ((match = pattern.match(text)).hasMatch()) {
        text.replace(match.capturedStart(), match.capturedLength(), replacement);
    }
}

void replaceRepeatedKeepingPrefix(QString &text, const QRegularExpression &pattern)
{
    QRegularExpressionMatch match;
    while ((match = pattern.match(text)).hasMatch()) {
        text.replace(match.capturedStart(), match.capturedLength(), match.captured(1));
    }
}

} // namespace

namespace TranscriptCleaner {

QString clean(const QString &raw)
{
    const QString marker = QStringLiteral("<asr_text>");
    const int idx = raw.indexOf(marker);
    if (idx >= 0) {
        return removeFillers(raw.mid(idx + marker.size()).trimmed());
    }

    const QString text = raw.trimmed();
    if (text.startsWith(QStringLiteral("Transcribe audio to text"), Qt::CaseInsensitive)) {
        return {};
    }
    return removeFillers(text);
}

QString removeFillers(const QString &text)
{
    QString cleaned = text.simplified();
    const QList<PlaceholderPair> quotedText = protectQuotedText(cleaned);

    const QRegularExpression strongPrefixSticky(
        QStringLiteral(R"(([。！？!?；;：:])\s*([嗯呃额唔啊呐欸诶哎嗳]+|u+h+|u+m+|e+r+)\s*(?=[\p{L}\p{N}]))"),
        regexOptions());
    replaceRepeatedKeepingPrefix(cleaned, strongPrefixSticky);

    const QRegularExpression weakPrefixSticky(
        QStringLiteral(R"((^|[\s,，、])\s*([嗯呃额唔啊呐欸诶哎嗳]+|u+h+|u+m+|e+r+)\s*(?=[\p{L}\p{N}]))"),
        regexOptions());
    replaceRepeated(cleaned, weakPrefixSticky, QString());

    const QRegularExpression standaloneFiller(
        QStringLiteral(R"((^|[\s,，、。！？!?；;：:])\s*([嗯呃额唔啊呐欸诶哎嗳]+|u+h+|u+m+|e+r+)\s*(?=$|[\s,，、。！？!?；;：:]))"),
        regexOptions());
    replaceRepeatedKeepingPrefix(cleaned, standaloneFiller);

    const QRegularExpression stickyChineseFiller(
        QStringLiteral(R"((?<=[\p{Han}])([嗯呃额唔]{1,3})(?=[\p{Han}\p{L}\p{N}]))"),
        regexOptions());
    replaceRepeated(cleaned, stickyChineseFiller, QString());

    cleaned = normalizePunctuation(cleaned);
    restoreQuotedText(cleaned, quotedText);
    return cleaned.trimmed();
}

QString normalizePunctuation(const QString &text)
{
    QString cleaned = text.simplified();

    cleaned.replace(QRegularExpression(QStringLiteral(R"(\s+([，、。！？；：]))")), QStringLiteral("\\1"));
    cleaned.replace(QRegularExpression(QStringLiteral(R"(([，、。！？；：])\s+)")), QStringLiteral("\\1"));
    cleaned.replace(QRegularExpression(QStringLiteral(R"([,，、]\s*[,，、]+)")), QStringLiteral("，"));
    cleaned.replace(QRegularExpression(QStringLiteral(R"([,，、]\s*([。！？!?；;：:]))")), QStringLiteral("\\1"));
    cleaned.replace(QRegularExpression(QStringLiteral(R"(([。！？!?；;：:])\s*[,，、]+)")), QStringLiteral("\\1"));
    cleaned.replace(QRegularExpression(QStringLiteral(R"(([。！？!?；;：:])\s*\1+)")), QStringLiteral("\\1"));
    cleaned.replace(QRegularExpression(QStringLiteral(R"(^[\s,，、。！？!?；;：:]+)")), QString());
    cleaned.replace(QRegularExpression(QStringLiteral(R"([\s,，、；;：:]+$)")), QString());
    return cleaned.trimmed();
}

} // namespace TranscriptCleaner
