#pragma once
#include <QtGlobal>
#include <QString>

// Astronomy math — a direct C++ port of the same formulas used (and
// validated against Astropy) in the web version of this clock:
//   - GMST via the standard IAU polynomial
//   - low-precision solar position (Meeus), used for the twilight indicator
//   - Polaris precessed to the date's equinox via the rigorous Meeus /
//     IAU-1976 formulas, for the hour-angle position dial

struct SunEq { double raHours; double decDeg; };
struct RaDec { double raHours; double decDeg; };
struct Twilight { QString text; int level; }; // 0=day 1=civil 2=nautical 3=astro 4=night

namespace Astro {
    double toJD(qint64 ms);
    double gmstHours(double jd);
    SunEq sunEquatorial(double jd);
    double sunAltitude(double latDeg, double lonDeg, double jd, double lstHours);
    Twilight twilightLabel(double alt);
    RaDec polarisRaDecOfDate(double jd);
    double polarisHourAngleHours(double lstHours, double jd);
}
