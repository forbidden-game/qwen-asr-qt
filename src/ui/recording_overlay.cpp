#include "ui/recording_overlay.h"

#include <QCursor>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

#include <cmath>

RecordingOverlay::RecordingOverlay(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFixedSize(300, 86);
    levels_.fill(0.0, 13);

    timer_.setInterval(33);
    connect(&timer_, &QTimer::timeout, this, &RecordingOverlay::tick);
}

void RecordingOverlay::showRecording()
{
    positionOnActiveScreen();
    clock_.restart();
    targetLevel_ = 0.0;
    displayLevel_ = 0.0;
    levels_.fill(0.0, levels_.size());
    timer_.start();
    show();
    raise();
}

void RecordingOverlay::hideRecording()
{
    timer_.stop();
    hide();
}

void RecordingOverlay::setInputLevel(qreal level)
{
    targetLevel_ = qBound<qreal>(0.0, level, 1.0);
}

void RecordingOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    const qreal t = clock_.isValid() ? clock_.elapsed() / 1000.0 : 0.0;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRectF panelRect(10, 10, width() - 20, height() - 20);
    QPainterPath shadow;
    shadow.addRoundedRect(panelRect.translated(0, 4), 18, 18);
    painter.fillPath(shadow, QColor(0, 0, 0, 70));

    QPainterPath panel;
    panel.addRoundedRect(panelRect, 18, 18);
    painter.fillPath(panel, QColor(28, 32, 36, 224));
    painter.setPen(QPen(QColor(86, 94, 105, 180), 1));
    painter.drawPath(panel);

    const QColor accent(61, 174, 233);
    const QPointF center(48, 43);
    const qreal pulse = qBound<qreal>(0.0, displayLevel_ + 0.08 * ((std::sin(t * 5.2) + 1.0) / 2.0), 1.0);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 30 + int(72 * pulse)));
    painter.drawEllipse(center, 16 + 12 * pulse, 16 + 12 * pulse);
    painter.setBrush(accent);
    painter.drawEllipse(center, 8 + 3 * pulse, 8 + 3 * pulse);
    painter.setBrush(QColor(255, 255, 255, 210));
    painter.drawEllipse(center, 3.5, 3.5);

    QFont titleFont = font();
    titleFont.setPointSize(13);
    titleFont.setWeight(QFont::DemiBold);
    painter.setFont(titleFont);
    painter.setPen(QColor(238, 242, 246));
    painter.drawText(QRectF(76, 22, 120, 24), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("录音中"));

    QFont smallFont = font();
    smallFont.setPointSize(9);
    painter.setFont(smallFont);
    painter.setPen(QColor(178, 188, 199));
    painter.drawText(QRectF(76, 48, 130, 20), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("松开后转写"));

    painter.setPen(Qt::NoPen);
    const int barCount = levels_.size();
    const qreal startX = 186;
    for (int i = 0; i < barCount; ++i) {
        const qreal shimmer = 0.035 * ((std::sin(t * 3.2 + i * 0.73) + 1.0) / 2.0);
        const qreal level = qBound<qreal>(0.0, levels_.at(i) + shimmer, 1.0);
        const qreal h = 6 + level * 40;
        QRectF bar(startX + i * 6.2, 43 - h / 2, 3.8, h);
        painter.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 88 + int(level * 132)));
        painter.drawRoundedRect(bar, 1.9, 1.9);
    }
}

void RecordingOverlay::tick()
{
    displayLevel_ = displayLevel_ * 0.70 + targetLevel_ * 0.30;
    targetLevel_ *= 0.90;
    if (!levels_.isEmpty()) {
        levels_.pop_front();
        levels_.push_back(displayLevel_);
    }
    update();
}

void RecordingOverlay::positionOnActiveScreen()
{
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return;
    }

    const QRect geometry = screen->availableGeometry();
    const int x = geometry.x() + (geometry.width() - width()) / 2;
    const int y = geometry.y() + qMax(32, geometry.height() / 10);
    move(x, y);
}
