#pragma once
#include <QWidget>
#include "Theme.h"

// The Polaris hour-angle position dial: a fixed crosshair marks the North
// Celestial Pole, and a small marker circle rides the rim, turning
// counter-clockwise as hour angle advances — matching the apparent motion
// of Polaris seen through a polar scope. The 0/24 mark can sit at the top
// or the bottom of the dial (a 180 degree flip, to match different
// reticle conventions).
class PolarisWidget : public QWidget {
    Q_OBJECT
public:
    explicit PolarisWidget(QWidget* parent = nullptr);

    void setTheme(const Theme& t);
    void setZeroBottom(bool bottom);
    void setHourAngleHours(double ha);

protected:
    void paintEvent(QPaintEvent*) override;
    QSize sizeHint() const override { return QSize(140, 140); }

private:
    Theme theme_ = Theme::day();
    bool zeroBottom_ = false;
    double ha_ = 0.0;
};
