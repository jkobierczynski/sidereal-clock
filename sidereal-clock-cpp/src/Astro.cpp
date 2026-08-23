#include "Astro.h"
#include <cmath>

static const double D2R = M_PI / 180.0;
static const double R2D = 180.0 / M_PI;

// Polaris J2000.0 coordinates: RA 2h31m49.09s, Dec +89°15'51.0"
static const double POLARIS_RA0_DEG = (2.0 + 31.0 / 60.0 + 49.09 / 3600.0) * 15.0;
static const double POLARIS_DEC0_DEG = 89.0 + 15.0 / 60.0 + 51.0 / 3600.0;

double Astro::toJD(qint64 ms) {
    return static_cast<double>(ms) / 86400000.0 + 2440587.5;
}

double Astro::gmstHours(double jd) {
    double d = jd - 2451545.0;
    double T = d / 36525.0;
    double g = 280.46061837 + 360.98564736629 * d + 0.000387933 * T * T - (T * T * T) / 38710000.0;
    g = std::fmod(g, 360.0);
    if (g < 0) g += 360.0;
    return g / 15.0;
}

SunEq Astro::sunEquatorial(double jd) {
    double n = jd - 2451545.0;
    double L = std::fmod(280.460 + 0.9856474 * n, 360.0); if (L < 0) L += 360.0;
    double g = std::fmod(357.528 + 0.9856003 * n, 360.0); if (g < 0) g += 360.0;
    double gr = g * D2R;
    double lambda = L + 1.915 * std::sin(gr) + 0.020 * std::sin(2 * gr);
    double lr = lambda * D2R;
    double eps = (23.439 - 0.0000004 * n) * D2R;
    double ra = std::atan2(std::cos(eps) * std::sin(lr), std::cos(lr)) * R2D;
    double dec = std::asin(std::sin(eps) * std::sin(lr)) * R2D;
    double raH = std::fmod(ra / 15.0, 24.0);
    if (raH < 0) raH += 24.0;
    return { raH, dec };
}

double Astro::sunAltitude(double latDeg, double lonDeg, double jd, double lstHours) {
    Q_UNUSED(lonDeg);
    SunEq sun = sunEquatorial(jd);
    double H = (lstHours - sun.raHours) * 15.0;
    H = std::fmod(H + 540.0, 360.0) - 180.0;
    double lat = latDeg * D2R, dec = sun.decDeg * D2R, h = H * D2R;
    double alt = std::asin(std::sin(lat) * std::sin(dec) + std::cos(lat) * std::cos(dec) * std::cos(h));
    return alt * R2D;
}

Twilight Astro::twilightLabel(double alt) {
    if (alt > 0)   return { "day", 0 };
    if (alt > -6)  return { "civil twilight", 1 };
    if (alt > -12) return { "nautical twilight", 2 };
    if (alt > -18) return { "astro twilight", 3 };
    return { "night", 4 };
}

// Rigorous Meeus / IAU-1976 precession (Astronomical Algorithms, Ch. 21).
// Starting epoch is J2000 itself, so the "T" term is zero and only the
// "t" (centuries from J2000 to the target date) terms remain. Verified
// against Astropy's FK5-equinox-of-date transform to within ~0.1 arcsec.
RaDec Astro::polarisRaDecOfDate(double jd) {
    double t = (jd - 2451545.0) / 36525.0;
    double t2 = t * t, t3 = t2 * t;
    double zeta  = 2306.2181 * t + 0.30188 * t2 + 0.017998 * t3;  // arcsec
    double z     = 2306.2181 * t + 1.09468 * t2 + 0.018203 * t3;  // arcsec
    double theta = 2004.3109 * t - 0.42665 * t2 - 0.041833 * t3;  // arcsec
    zeta *= D2R / 3600.0;
    z    *= D2R / 3600.0;
    theta *= D2R / 3600.0;

    double ra0 = POLARIS_RA0_DEG * D2R, dec0 = POLARIS_DEC0_DEG * D2R;
    double A = std::cos(dec0) * std::sin(ra0 + zeta);
    double B = std::cos(theta) * std::cos(dec0) * std::cos(ra0 + zeta) - std::sin(theta) * std::sin(dec0);
    double C = std::sin(theta) * std::cos(dec0) * std::cos(ra0 + zeta) + std::cos(theta) * std::sin(dec0);
    double ra = std::atan2(A, B) + z;
    double dec = std::asin(C);
    ra *= R2D;
    double raH = std::fmod(ra / 15.0, 24.0);
    if (raH < 0) raH += 24.0;
    return { raH, dec * R2D };
}

double Astro::polarisHourAngleHours(double lstHours, double jd) {
    RaDec p = polarisRaDecOfDate(jd);
    double ha = lstHours - p.raHours;
    ha = std::fmod(std::fmod(ha, 24.0) + 24.0, 24.0);
    return ha;
}
