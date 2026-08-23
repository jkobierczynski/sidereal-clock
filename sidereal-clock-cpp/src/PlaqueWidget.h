#pragma once
#include <QWidget>
#include <QLabel>
#include "Theme.h"
#include "AppState.h"

// The digital readout block — sidereal time (always 24h), local time
// (12h/24h with AM/PM), or both stacked, in "Mechanical" (monospace) or
// "Engraved" (serif italic) numerals.
class PlaqueWidget : public QWidget {
    Q_OBJECT
public:
    explicit PlaqueWidget(QWidget* parent = nullptr);

    void setTheme(const Theme& t);
    void setNumeralStyle(DigitalNumeral n);
    void setShows(ShowMode m);

    void setSiderealText(const QString& s);
    void setLocalText(const QString& s, const QString& period);
    void setSubText(const QString& s);

private:
    void restyle();

    Theme theme_ = Theme::day();
    DigitalNumeral numeral_ = DigitalNumeral::Mechanical;
    ShowMode shows_ = ShowMode::Sidereal;

    QWidget* siderealBlock_;
    QWidget* localBlock_;
    QLabel* siderealNum_;
    QLabel* localNum_;
    QLabel* localPeriod_;
    QLabel* subLabel_;
};
