#pragma once
#include <QWidget>
#include "Theme.h"
#include "AppState.h"

// A single analog clock face — reused for both the sidereal and the
// local dial. hourFraction is always given in the 0..24 sidereal/local
// hour convention; is12_ decides whether the hour hand wraps at 12 or 24
// (minute/second hands always derive from the fractional part, exactly
// like the web version's setHands()).
class DialWidget : public QWidget {
    Q_OBJECT
public:
    explicit DialWidget(QWidget* parent = nullptr);

    void setTheme(const Theme& t);
    void setStyleMode(AnalogStyle s);
    void setIs12Hour(bool is12);
    void setHourFraction(double hf);

signals:
    // Ctrl+wheel over the dial — steps is +1/-1 per notch scrolled.
    void scaleRequested(int steps);

protected:
    void paintEvent(QPaintEvent*) override;
    void wheelEvent(QWheelEvent* event) override;
    QSize sizeHint() const override { return QSize(360, 360); }

private:
    Theme theme_ = Theme::day();
    AnalogStyle style_ = AnalogStyle::Minimalist;
    bool is12_ = false;
    double hourFraction_ = 0.0;
};
