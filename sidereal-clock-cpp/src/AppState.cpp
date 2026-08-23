#include "AppState.h"
#include <QSettings>
#include <QTimeZone>

// --- enum <-> string helpers (used both for QSettings persistence and,
//     indirectly, to keep this file readable rather than sprinkling raw
//     ints through the rest of the app) ---

QString layoutToStr(Layout v){ switch(v){ case Layout::Both: return "both"; case Layout::Analog: return "analog"; default: return "digital"; } }
Layout strToLayout(const QString& s){ if (s=="analog") return Layout::Analog; if (s=="digital") return Layout::Digital; return Layout::Both; }

QString analogStyleToStr(AnalogStyle v){ switch(v){ case AnalogStyle::Classic: return "classic"; case AnalogStyle::Observatory: return "observatory"; case AnalogStyle::Skeleton: return "skeleton"; default: return "minimalist"; } }
AnalogStyle strToAnalogStyle(const QString& s){ if (s=="classic") return AnalogStyle::Classic; if (s=="observatory") return AnalogStyle::Observatory; if (s=="skeleton") return AnalogStyle::Skeleton; return AnalogStyle::Minimalist; }

QString digitalPositionToStr(DigitalPosition v){ switch(v){ case DigitalPosition::Right: return "right"; case DigitalPosition::Left: return "left"; default: return "beneath"; } }
DigitalPosition strToDigitalPosition(const QString& s){ if (s=="right") return DigitalPosition::Right; if (s=="left") return DigitalPosition::Left; return DigitalPosition::Beneath; }

QString showModeToStr(ShowMode v){ switch(v){ case ShowMode::Local: return "local"; case ShowMode::Both: return "both"; default: return "sidereal"; } }
ShowMode strToShowMode(const QString& s){ if (s=="local") return ShowMode::Local; if (s=="both") return ShowMode::Both; return ShowMode::Sidereal; }

QString digitalNumeralToStr(DigitalNumeral v){ return v==DigitalNumeral::Engraved ? "engraved" : "mechanical"; }
DigitalNumeral strToDigitalNumeral(const QString& s){ return s=="engraved" ? DigitalNumeral::Engraved : DigitalNumeral::Mechanical; }

QString clockSourceToStr(ClockSource v){ return v==ClockSource::Custom ? "custom" : "live"; }
ClockSource strToClockSource(const QString& s){ return s=="custom" ? ClockSource::Custom : ClockSource::Live; }

QString polarisZeroToStr(PolarisZero v){ return v==PolarisZero::Bottom ? "bottom" : "top"; }
PolarisZero strToPolarisZero(const QString& s){ return s=="bottom" ? PolarisZero::Bottom : PolarisZero::Top; }

qint64 zonedWallTimeToUtcMs(const QDateTime& localDateTime, const QString& tzId) {
    QTimeZone tz(tzId.toUtf8());
    if (!tz.isValid()) tz = QTimeZone::utc();
    QDateTime dt(localDateTime.date(), localDateTime.time(), tz);
    return dt.toMSecsSinceEpoch();
}

static QString systemTz() {
    QByteArray id = QTimeZone::systemTimeZoneId();
    return id.isEmpty() ? QStringLiteral("UTC") : QString::fromUtf8(id);
}

void AppState::resetToDefaults() {
    *this = AppState();
    tz = systemTz();
}

void AppState::load() {
    QSettings s(QStringLiteral("jkobierczynski"), QStringLiteral("SiderealClock"));

    locName = s.value("locName", locName).toString();
    lat = s.value("lat", lat).toDouble();
    lon = s.value("lon", lon).toDouble();
    tz = s.value("tz", systemTz()).toString();

    layout = strToLayout(s.value("layout", layoutToStr(layout)).toString());
    analogStyle = strToAnalogStyle(s.value("analogStyle", analogStyleToStr(analogStyle)).toString());
    digitalFormat = s.value("digitalFormat", digitalFormat).toString();
    digitalSeconds = s.value("digitalSeconds", digitalSeconds).toBool();
    digitalNumeral = strToDigitalNumeral(s.value("digitalNumeral", digitalNumeralToStr(digitalNumeral)).toString());
    digitalShows = strToShowMode(s.value("digitalShows", showModeToStr(digitalShows)).toString());
    digitalPosition = strToDigitalPosition(s.value("digitalPosition", digitalPositionToStr(digitalPosition)).toString());
    analogShows = strToShowMode(s.value("analogShows", showModeToStr(analogShows)).toString());
    dialHours = s.value("dialHours", dialHours).toString();
    dialScale = s.value("dialScale", dialScale).toDouble();

    polarisShowDial = s.value("polarisShowDial", polarisShowDial).toBool();
    polarisShowHA = s.value("polarisShowHA", polarisShowHA).toBool();
    polarisZero = strToPolarisZero(s.value("polarisZero", polarisZeroToStr(polarisZero)).toString());
    polarisScale = s.value("polarisScale", polarisScale).toDouble();

    night = s.value("night", night).toBool();
    autoNight = s.value("autoNight", autoNight).toBool();

    clockSource = strToClockSource(s.value("clockSource", clockSourceToStr(clockSource)).toString());
    customBaseMs = s.value("customBaseMs", (qlonglong)customBaseMs).toLongLong();
    customPlaying = s.value("customPlaying", customPlaying).toBool();
    syncOffsetMs = s.value("syncOffsetMs", (qlonglong)syncOffsetMs).toLongLong();

    ntpProfile = s.value("ntpProfile", ntpProfile).toString();
    ntpServers = s.value("ntpServers", ntpServers).toStringList();
    if (ntpServers.isEmpty()) ntpServers << QStringLiteral("pool.ntp.org");
}

void AppState::save() const {
    QSettings s(QStringLiteral("jkobierczynski"), QStringLiteral("SiderealClock"));

    s.setValue("locName", locName);
    s.setValue("lat", lat);
    s.setValue("lon", lon);
    s.setValue("tz", tz);

    s.setValue("layout", layoutToStr(layout));
    s.setValue("analogStyle", analogStyleToStr(analogStyle));
    s.setValue("digitalFormat", digitalFormat);
    s.setValue("digitalSeconds", digitalSeconds);
    s.setValue("digitalNumeral", digitalNumeralToStr(digitalNumeral));
    s.setValue("digitalShows", showModeToStr(digitalShows));
    s.setValue("digitalPosition", digitalPositionToStr(digitalPosition));
    s.setValue("analogShows", showModeToStr(analogShows));
    s.setValue("dialHours", dialHours);
    s.setValue("dialScale", dialScale);

    s.setValue("polarisShowDial", polarisShowDial);
    s.setValue("polarisShowHA", polarisShowHA);
    s.setValue("polarisZero", polarisZeroToStr(polarisZero));
    s.setValue("polarisScale", polarisScale);

    s.setValue("night", night);
    s.setValue("autoNight", autoNight);

    s.setValue("clockSource", clockSourceToStr(clockSource));
    s.setValue("customBaseMs", (qlonglong)customBaseMs);
    s.setValue("customPlaying", customPlaying);
    s.setValue("syncOffsetMs", (qlonglong)syncOffsetMs);

    s.setValue("ntpProfile", ntpProfile);
    s.setValue("ntpServers", ntpServers);
}
