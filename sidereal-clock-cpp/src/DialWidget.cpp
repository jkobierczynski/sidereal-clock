#include "DialWidget.h"
#include <QPainter>
#include <QtMath>

DialWidget::DialWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(120, 120);
}

void DialWidget::setTheme(const Theme& t) { theme_ = t; update(); }
void DialWidget::setStyleMode(AnalogStyle s) { style_ = s; update(); }
void DialWidget::setIs12Hour(bool is12) { is12_ = is12; update(); }
void DialWidget::setHourFraction(double hf) { hourFraction_ = hf; update(); }

// Angle convention throughout: 0 = straight up, positive = clockwise —
// same as the web version's SVG rotate()/sin()/cos() convention, and
// conveniently the same sign Qt's QPainter::rotate() uses.
static QPointF pointAt(double cx, double cy, double r, double angleRad) {
    return QPointF(cx + r * std::sin(angleRad), cy - r * std::cos(angleRad));
}

void DialWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const double side = std::min(width(), height());
    p.translate((width() - side) / 2.0, (height() - side) / 2.0);
    const double scale = side / 400.0;
    p.scale(scale, scale);

    const double cx = 200, cy = 200;
    const bool skeleton = (style_ == AnalogStyle::Skeleton);
    const bool observatory = (style_ == AnalogStyle::Observatory);
    const bool showNumerals = (style_ == AnalogStyle::Classic || observatory);
    const bool showTicks = !(style_ == AnalogStyle::Minimalist || skeleton);

    // outer rim + face
    p.setPen(QPen(theme_.inkLine, 1));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QPointF(cx, cy), 196, 196);

    if (!skeleton) {
        QColor faceColor = observatory ? theme_.ink800 : theme_.face;
        p.setBrush(faceColor);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(cx, cy), 188, 188);
    }

    {
        double rimW = observatory ? 3 : (style_ == AnalogStyle::Minimalist ? 1 : 2);
        double rimOpacity = (style_ == AnalogStyle::Minimalist) ? 0.6 : 1.0;
        QColor rimColor = theme_.brass; rimColor.setAlphaF(rimOpacity);
        p.setBrush(Qt::NoBrush);
        if (skeleton) {
            QPen pen(rimColor, 1.5);
            QVector<qreal> dashes; dashes << 2 << 6;
            pen.setDashPattern(dashes);
            p.setPen(pen);
        } else {
            p.setPen(QPen(rimColor, rimW));
        }
        p.drawEllipse(QPointF(cx, cy), 188, 188);
    }

    // ticks + numerals
    if (showTicks || showNumerals) {
        QFont numFont("Georgia", observatory ? 15 : 14);
        numFont.setItalic(true);
        numFont.setStyleName("Italic");
        numFont.setWeight(QFont::DemiBold);
        p.setFont(numFont);

        if (!is12_) {
            for (int i = 0; i < 24; ++i) {
                double angle = i * 15.0 * M_PI / 180.0;
                bool isCardinal = (i % 6 == 0);
                bool isMajor = (i % 2 == 0);
                double rOuter = 188, rInner = isCardinal ? 160 : (isMajor ? 168 : 176);
                if (showTicks) {
                    QColor tc = isCardinal ? theme_.brassDim : (isMajor ? theme_.faceDeep : theme_.faceShade);
                    double tw = isCardinal ? 3 : (isMajor ? 2.5 : 1);
                    p.setPen(QPen(tc, tw));
                    p.drawLine(pointAt(cx, cy, rOuter, angle), pointAt(cx, cy, rInner, angle));
                }
                if (showNumerals && isMajor) {
                    QColor nc = observatory ? theme_.brassDim : theme_.faceDeep;
                    p.setPen(nc);
                    QPointF tp = pointAt(cx, cy, 140, angle);
                    p.drawText(QRectF(tp.x() - 20, tp.y() - 12, 40, 24), Qt::AlignCenter, QString::number(i));
                }
            }
        } else {
            for (int i = 0; i < 60; ++i) {
                double angle = i * 6.0 * M_PI / 180.0;
                bool isHour = (i % 5 == 0);
                bool isCardinal = (i % 15 == 0);
                double rOuter = 188, rInner = isCardinal ? 160 : (isHour ? 168 : 180);
                if (showTicks) {
                    QColor tc = isCardinal ? theme_.brassDim : (isHour ? theme_.faceDeep : theme_.faceShade);
                    double tw = isCardinal ? 3 : (isHour ? 2.5 : 1);
                    p.setPen(QPen(tc, tw));
                    p.drawLine(pointAt(cx, cy, rOuter, angle), pointAt(cx, cy, rInner, angle));
                }
                if (showNumerals && isHour) {
                    int label = (i == 0) ? 12 : i / 5;
                    QColor nc = observatory ? theme_.brassDim : theme_.faceDeep;
                    p.setPen(nc);
                    QPointF tp = pointAt(cx, cy, 140, angle);
                    p.drawText(QRectF(tp.x() - 20, tp.y() - 12, 40, 24), Qt::AlignCenter, QString::number(label));
                }
            }
        }
    }

    // hands — hourDeg respects the 12/24 face; minute/second always derive
    // from the continuous fractional part, exactly like the web version.
    double hourDeg = is12_ ? (std::fmod(hourFraction_, 12.0) / 12.0) * 360.0 : (hourFraction_ / 24.0) * 360.0;
    double minDeg = std::fmod(hourFraction_ * 60.0, 60.0) / 60.0 * 360.0;
    double secDeg = std::fmod(hourFraction_ * 3600.0, 60.0) / 60.0 * 360.0;

    double hourW, minW, secW, hourLen, minLen, secLen, secBackLen;
    switch (style_) {
        case AnalogStyle::Classic:     hourW=5;   minW=3;   secW=1.4; hourLen=108; minLen=142; secLen=160; secBackLen=12; break;
        case AnalogStyle::Observatory: hourW=7;   minW=4.5; secW=2;   hourLen=108; minLen=142; secLen=160; secBackLen=12; break;
        case AnalogStyle::Skeleton:    hourW=2;   minW=1.4; secW=0.8; hourLen=108; minLen=142; secLen=160; secBackLen=12; break;
        default:                       hourW=3;   minW=2;   secW=1;   hourLen=108; minLen=142; secLen=160; secBackLen=12; break; // minimalist
    }

    auto drawHand = [&](double deg, double len, double backLen, double w, const QColor& color) {
        p.save();
        p.translate(cx, cy);
        p.rotate(deg);
        p.setPen(QPen(color, w, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(0, backLen), QPointF(0, -len));
        p.restore();
    };

    QColor glow = theme_.brass; glow.setAlphaF(0.35);
    if (observatory) {
        // a soft halo behind hour/minute hands, echoing the web version's drop-shadow glow
        drawHand(hourDeg, hourLen + 2, 0, hourW + 4, glow);
        drawHand(minDeg, minLen + 2, 0, minW + 4, glow);
    }
    drawHand(hourDeg, hourLen, 0, hourW, theme_.brassDim);
    drawHand(minDeg, minLen, 0, minW, theme_.brass);
    drawHand(secDeg, secLen, -secBackLen, secW, theme_.brassBright);

    // hub
    p.setPen(QPen(theme_.ink950, 2));
    p.setBrush(theme_.brass);
    double hubR = skeleton ? 4 : 7;
    p.drawEllipse(QPointF(cx, cy), hubR, hubR);
    p.setPen(Qt::NoPen);
    p.setBrush(theme_.ink950);
    p.drawEllipse(QPointF(cx, cy), 2, 2);
}
