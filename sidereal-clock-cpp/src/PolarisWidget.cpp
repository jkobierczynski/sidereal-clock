#include "PolarisWidget.h"
#include <QPainter>
#include <QWheelEvent>
#include <QtMath>

PolarisWidget::PolarisWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(80, 80);
}

void PolarisWidget::setTheme(const Theme& t) { theme_ = t; update(); }
void PolarisWidget::setZeroBottom(bool bottom) { zeroBottom_ = bottom; update(); }
void PolarisWidget::setHourAngleHours(double ha) { ha_ = ha; update(); }

void PolarisWidget::wheelEvent(QWheelEvent* event) {
    if (event->modifiers() & Qt::ControlModifier) {
        int steps = event->angleDelta().y() > 0 ? 1 : -1;
        emit scaleRequested(steps);
        event->accept();
        return;
    }
    QWidget::wheelEvent(event);
}

static QPointF pointAt(double cx, double cy, double r, double angleRad) {
    return QPointF(cx + r * std::sin(angleRad), cy - r * std::cos(angleRad));
}

void PolarisWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const double side = std::min(width(), height());
    p.translate((width() - side) / 2.0, (height() - side) / 2.0);
    const double scale = side / 400.0;
    p.scale(scale, scale);

    const double cx = 200, cy = 200;

    p.setPen(QPen(theme_.inkLine, 1));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QPointF(cx, cy), 196, 196);

    p.setBrush(theme_.face);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(cx, cy), 188, 188);

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(theme_.brass, 2));
    p.drawEllipse(QPointF(cx, cy), 188, 188);

    // 24 hourly ticks, counter-clockwise from the zero position, numerals
    // baked in at the correct angle (not via a rotated group) so they stay
    // upright regardless of the zero-position setting.
    double offsetRad = (zeroBottom_ ? 180.0 : 0.0) * M_PI / 180.0;
    QFont numFont("Georgia", 20);
    numFont.setItalic(true);
    numFont.setWeight(QFont::DemiBold);
    p.setFont(numFont);

    for (int i = 0; i < 24; ++i) {
        double angle = offsetRad - i * 15.0 * M_PI / 180.0;
        bool isCardinal = (i % 6 == 0);
        bool isMajor = (i % 2 == 0);
        double rOuter = 188, rInner = isCardinal ? 160 : (isMajor ? 172 : 180);
        QColor tc = isCardinal ? theme_.brassDim : (isMajor ? theme_.faceDeep : theme_.faceShade);
        double tw = isCardinal ? 3 : (isMajor ? 2.5 : 1);
        p.setPen(QPen(tc, tw));
        p.drawLine(pointAt(cx, cy, rOuter, angle), pointAt(cx, cy, rInner, angle));
        if (isCardinal) {
            p.setPen(theme_.faceDeep);
            QPointF tp = pointAt(cx, cy, 142, angle);
            p.drawText(QRectF(tp.x() - 22, tp.y() - 14, 44, 28), Qt::AlignCenter, QString::number(i));
        }
    }

    // crosshair marking the North Celestial Pole
    p.setPen(QPen(theme_.brassDim, 1.6));
    p.drawLine(QPointF(cx, cy - 22), QPointF(cx, cy + 22));
    p.drawLine(QPointF(cx - 22, cy), QPointF(cx + 22, cy));
    p.setBrush(theme_.brassDim);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(cx, cy), 3, 3);

    // Polaris marker — rides the rim, turning counter-clockwise as hour
    // angle increases, offset by the chosen zero position.
    double markerAngle = offsetRad - (ha_ / 24.0) * 2.0 * M_PI;
    QPointF mp = pointAt(cx, cy, 188, markerAngle);
    QColor glow = theme_.brass; glow.setAlphaF(0.35);
    p.setPen(QPen(glow, 8));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(mp, 11, 11);
    p.setPen(QPen(theme_.brassBright, 4));
    p.setBrush(theme_.ink950);
    p.drawEllipse(mp, 11, 11);
}
