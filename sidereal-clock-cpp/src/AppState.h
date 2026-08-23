#pragma once
#include <QString>
#include <QStringList>
#include <QDateTime>

enum class Layout { Both, Analog, Digital };
enum class AnalogStyle { Minimalist, Classic, Observatory, Skeleton };
enum class DigitalPosition { Beneath, Right, Left };
enum class ShowMode { Sidereal, Local, Both };
enum class DigitalNumeral { Mechanical, Engraved };
enum class ClockSource { Live, Custom };
enum class PolarisZero { Top, Bottom };

class AppState {
public:
    QString locName = QStringLiteral("Brussels, Belgium");
    double lat = 50.8503;
    double lon = 4.3517;
    QString tz; // IANA id; defaulted to the system zone at load() time

    Layout layout = Layout::Both;
    AnalogStyle analogStyle = AnalogStyle::Minimalist;
    QString digitalFormat = QStringLiteral("24"); // local time format: "24" or "12"
    bool digitalSeconds = true;
    DigitalNumeral digitalNumeral = DigitalNumeral::Mechanical;
    ShowMode digitalShows = ShowMode::Sidereal;
    DigitalPosition digitalPosition = DigitalPosition::Beneath;
    ShowMode analogShows = ShowMode::Sidereal;
    QString dialHours = QStringLiteral("24"); // "24" or "12"
    double dialScale = 1.0; // Ctrl+wheel over a main dial adjusts this

    bool polarisShowDial = true;
    bool polarisShowHA = true;
    PolarisZero polarisZero = PolarisZero::Top;
    double polarisScale = 1.0; // Ctrl+wheel over the Polaris panel adjusts this

    bool night = false;
    bool autoNight = false;

    ClockSource clockSource = ClockSource::Live;
    qint64 customBaseMs = 0;
    bool customPlaying = true;
    qint64 syncOffsetMs = 0;

    QString ntpProfile = QStringLiteral("pool.ntp.org");
    QStringList ntpServers = { QStringLiteral("pool.ntp.org") };

    void load();
    void save() const;
    void resetToDefaults();
};

QString layoutToStr(Layout v);
Layout strToLayout(const QString& s);
QString analogStyleToStr(AnalogStyle v);
AnalogStyle strToAnalogStyle(const QString& s);
QString digitalPositionToStr(DigitalPosition v);
DigitalPosition strToDigitalPosition(const QString& s);
QString showModeToStr(ShowMode v);
ShowMode strToShowMode(const QString& s);
QString digitalNumeralToStr(DigitalNumeral v);
DigitalNumeral strToDigitalNumeral(const QString& s);
QString clockSourceToStr(ClockSource v);
ClockSource strToClockSource(const QString& s);
QString polarisZeroToStr(PolarisZero v);
PolarisZero strToPolarisZero(const QString& s);

// Interprets a wall-clock date/time as belonging to the given IANA zone
// and returns the corresponding UTC instant, in milliseconds since epoch.
qint64 zonedWallTimeToUtcMs(const QDateTime& localDateTime, const QString& tzId);
