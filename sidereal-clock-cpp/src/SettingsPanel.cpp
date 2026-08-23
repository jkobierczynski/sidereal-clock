#include "SettingsPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QScrollArea>
#include <QMessageBox>
#include <QTimeZone>
#include <QDateTime>
#include <QPair>
#include <algorithm>
#include <initializer_list>

SettingsPanel::SettingsPanel(AppState* state, QWidget* parent)
    : QWidget(parent), state_(state) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    tabs_ = new QTabWidget(this);
    tabs_->addTab(buildDisplayTab(), "Display");
    tabs_->addTab(buildLocationTab(), "Location");
    tabs_->addTab(buildTimeTab(), "Time && Sync");
    tabs_->addTab(buildPolarisTab(), "Polaris");
    outer->addWidget(tabs_, 1);

    bReset_ = new QPushButton("Reset to defaults");
    connect(bReset_, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, "Reset settings", "Reset all settings to defaults?") != QMessageBox::Yes) return;
        state_->resetToDefaults();
        refreshFromState();
        emitChanged();
    });
    auto* footer = new QWidget(this);
    auto* fLay = new QHBoxLayout(footer);
    fLay->addWidget(bReset_);
    fLay->addStretch();
    outer->addWidget(footer);

    connect(&sntp_, &SntpClient::finished, this, [this](bool ok, qint64 offsetMs, QString message) {
        bQueryNtp_->setEnabled(true);
        if (ok) {
            state_->syncOffsetMs = offsetMs;
            lNtpStatus_->setText(QString("Synced: offset %1 ms (%2)").arg(offsetMs).arg(message));
            emitChanged();
        } else {
            lNtpStatus_->setText("Sync failed: " + message);
        }
    });
}

// ---------------------------------------------------------------- Display
QWidget* SettingsPanel::buildDisplayTab() {
    auto* area = new QScrollArea; area->setWidgetResizable(true);
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);

    auto addRadioGroup = [&](const QString& title, std::initializer_list<QPair<QString, QRadioButton**>> items) {
        auto* gb = new QGroupBox(title);
        auto* h = new QHBoxLayout(gb);
        for (auto& item : items) {
            auto* rb = new QRadioButton(item.first);
            *item.second = rb;
            h->addWidget(rb);
        }
        v->addWidget(gb);
    };

    addRadioGroup("Layout", { {"Analog + digital", &rLayoutBoth_}, {"Analog only", &rLayoutAnalog_}, {"Digital only", &rLayoutDigital_} });
    addRadioGroup("Digital position", { {"Beneath dial", &rPosBeneath_}, {"Right of dial", &rPosRight_}, {"Left of dial", &rPosLeft_} });
    addRadioGroup("Analog shows", { {"Sidereal dial", &rAnalogShowsSid_}, {"Local dial", &rAnalogShowsLoc_}, {"Both dials", &rAnalogShowsBoth_} });
    addRadioGroup("Analog style", { {"Minimalist", &rStyleMinimal_}, {"Classic", &rStyleClassic_}, {"Observatory", &rStyleObservatory_}, {"Skeleton", &rStyleSkeleton_} });
    addRadioGroup("Analog dial", { {"24-hour face", &rDial24_}, {"12-hour face", &rDial12_} });
    addRadioGroup("Digital shows", { {"Sidereal time", &rDigShowsSid_}, {"Local time", &rDigShowsLoc_}, {"Both", &rDigShowsBoth_} });

    auto* fmtBox = new QGroupBox("Local time format");
    auto* fmtLay = new QVBoxLayout(fmtBox);
    auto* fmtRow = new QHBoxLayout;
    rFormat24_ = new QRadioButton("24-hour"); rFormat12_ = new QRadioButton("12-hour");
    fmtRow->addWidget(rFormat24_); fmtRow->addWidget(rFormat12_);
    fmtLay->addLayout(fmtRow);
    cSeconds_ = new QCheckBox("Show seconds");
    fmtLay->addWidget(cSeconds_);
    v->addWidget(fmtBox);

    addRadioGroup("Digital numerals", { {"Mechanical", &rNumMechanical_}, {"Engraved", &rNumEngraved_} });

    auto* nightBox = new QGroupBox("Night mode");
    auto* nightLay = new QVBoxLayout(nightBox);
    cNight_ = new QCheckBox("Red-on-black (preserves night vision)");
    cAutoNight_ = new QCheckBox("Switch automatically after astronomical dusk");
    nightLay->addWidget(cNight_);
    nightLay->addWidget(cAutoNight_);
    v->addWidget(nightBox);

    v->addStretch();
    area->setWidget(page);

    // wiring
    auto wireLayout = [this]() {
        rLayoutBoth_->setChecked(state_->layout == Layout::Both);
        rLayoutAnalog_->setChecked(state_->layout == Layout::Analog);
        rLayoutDigital_->setChecked(state_->layout == Layout::Digital);
    };
    connect(rLayoutBoth_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->layout=Layout::Both; emitChanged(); } });
    connect(rLayoutAnalog_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->layout=Layout::Analog; emitChanged(); } });
    connect(rLayoutDigital_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->layout=Layout::Digital; emitChanged(); } });
    wireLayout();

    connect(rPosBeneath_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->digitalPosition=DigitalPosition::Beneath; emitChanged(); } });
    connect(rPosRight_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->digitalPosition=DigitalPosition::Right; emitChanged(); } });
    connect(rPosLeft_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->digitalPosition=DigitalPosition::Left; emitChanged(); } });

    connect(rAnalogShowsSid_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->analogShows=ShowMode::Sidereal; emitChanged(); } });
    connect(rAnalogShowsLoc_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->analogShows=ShowMode::Local; emitChanged(); } });
    connect(rAnalogShowsBoth_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->analogShows=ShowMode::Both; emitChanged(); } });

    connect(rStyleMinimal_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->analogStyle=AnalogStyle::Minimalist; emitChanged(); } });
    connect(rStyleClassic_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->analogStyle=AnalogStyle::Classic; emitChanged(); } });
    connect(rStyleObservatory_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->analogStyle=AnalogStyle::Observatory; emitChanged(); } });
    connect(rStyleSkeleton_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->analogStyle=AnalogStyle::Skeleton; emitChanged(); } });

    connect(rDial24_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->dialHours="24"; emitChanged(); } });
    connect(rDial12_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->dialHours="12"; emitChanged(); } });

    connect(rDigShowsSid_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->digitalShows=ShowMode::Sidereal; emitChanged(); } });
    connect(rDigShowsLoc_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->digitalShows=ShowMode::Local; emitChanged(); } });
    connect(rDigShowsBoth_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->digitalShows=ShowMode::Both; emitChanged(); } });

    connect(rFormat24_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->digitalFormat="24"; emitChanged(); } });
    connect(rFormat12_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->digitalFormat="12"; emitChanged(); } });
    connect(cSeconds_, &QCheckBox::toggled, this, [this](bool on){ state_->digitalSeconds=on; emitChanged(); });

    connect(rNumMechanical_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->digitalNumeral=DigitalNumeral::Mechanical; emitChanged(); } });
    connect(rNumEngraved_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->digitalNumeral=DigitalNumeral::Engraved; emitChanged(); } });

    connect(cNight_, &QCheckBox::toggled, this, [this](bool on){ state_->night=on; if(on) { state_->autoNight=false; cAutoNight_->setChecked(false);} emitChanged(); });
    connect(cAutoNight_, &QCheckBox::toggled, this, [this](bool on){ state_->autoNight=on; emitChanged(); });

    return area;
}

// --------------------------------------------------------------- Location
QWidget* SettingsPanel::buildLocationTab() {
    auto* area = new QScrollArea; area->setWidgetResizable(true);
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);

    v->addWidget(new QLabel("Location name"));
    eLocName_ = new QLineEdit;
    v->addWidget(eLocName_);

    auto* row = new QHBoxLayout;
    auto* latCol = new QVBoxLayout; latCol->addWidget(new QLabel("Latitude (°, + north)"));
    eLat_ = new QDoubleSpinBox; eLat_->setRange(-90, 90); eLat_->setDecimals(4);
    latCol->addWidget(eLat_);
    auto* lonCol = new QVBoxLayout; lonCol->addWidget(new QLabel("Longitude (°, + east)"));
    eLon_ = new QDoubleSpinBox; eLon_->setRange(-180, 180); eLon_->setDecimals(4);
    lonCol->addWidget(eLon_);
    row->addLayout(latCol); row->addLayout(lonCol);
    v->addLayout(row);

    v->addWidget(new QLabel("Time zone"));
    cTz_ = new QComboBox;
    QList<QByteArray> ids = QTimeZone::availableTimeZoneIds();
    QStringList zoneNames;
    for (auto& id : ids) zoneNames << QString::fromUtf8(id);
    zoneNames.sort(Qt::CaseInsensitive);
    cTz_->addItems(zoneNames);
    v->addWidget(cTz_);
    auto* tzHint = new QLabel("Used to display local civil time and to interpret any date you set by hand.\n\n"
                               "Note: this desktop build sets your location by hand only — there's no\n"
                               "built-in geolocation lookup here (unlike a browser, a native app has no\n"
                               "standard cross-platform way to ask the OS for your position).");
    tzHint->setWordWrap(true);
    v->addWidget(tzHint);

    v->addStretch();
    area->setWidget(page);

    connect(eLocName_, &QLineEdit::textChanged, this, [this](const QString& t){ state_->locName = t; emitChanged(); });
    connect(eLat_, &QDoubleSpinBox::editingFinished, this, [this](){ state_->lat = eLat_->value(); emitChanged(); });
    connect(eLon_, &QDoubleSpinBox::editingFinished, this, [this](){ state_->lon = eLon_->value(); emitChanged(); });
    connect(cTz_, &QComboBox::currentTextChanged, this, [this](const QString& t){ if(!t.isEmpty()){ state_->tz = t; emitChanged(); } });

    return area;
}

// -------------------------------------------------------------- Time&Sync
QWidget* SettingsPanel::buildTimeTab() {
    auto* area = new QScrollArea; area->setWidgetResizable(true);
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);

    auto* sourceBox = new QGroupBox("Clock source");
    auto* sourceLay = new QHBoxLayout(sourceBox);
    rSourceLive_ = new QRadioButton("System clock");
    rSourceCustom_ = new QRadioButton("Custom start time");
    sourceLay->addWidget(rSourceLive_); sourceLay->addWidget(rSourceCustom_);
    v->addWidget(sourceBox);

    auto* customBox = new QGroupBox("Custom date && time (in selected time zone)");
    auto* customLay = new QVBoxLayout(customBox);
    eCustom_ = new QDateTimeEdit(QDateTime::currentDateTime());
    eCustom_->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    eCustom_->setCalendarPopup(true);
    customLay->addWidget(eCustom_);
    auto* customBtnRow = new QHBoxLayout;
    bApplyCustom_ = new QPushButton("Set");
    bPlayPause_ = new QPushButton("Pause");
    customBtnRow->addWidget(bApplyCustom_); customBtnRow->addWidget(bPlayPause_);
    customLay->addLayout(customBtnRow);
    auto* customHint = new QLabel("The clock keeps flowing forward in real time from whatever moment you set.");
    customHint->setWordWrap(true);
    customLay->addWidget(customHint);
    v->addWidget(customBox);

    auto* syncBox = new QGroupBox("Sync correction");
    auto* syncLay = new QVBoxLayout(syncBox);
    lSyncOffset_ = new QLabel("Offset applied to the system clock: +0.00 s");
    syncLay->addWidget(lSyncOffset_);
    bClearSync_ = new QPushButton("Clear correction");
    syncLay->addWidget(bClearSync_);
    v->addWidget(syncBox);

    auto* ntpBox = new QGroupBox("NTP servers");
    auto* ntpLay = new QVBoxLayout(ntpBox);
    ntpLay->addWidget(new QLabel("Preferred server"));
    cNtpProfile_ = new QComboBox;
    cNtpProfile_->setEditable(false);
    ntpLay->addWidget(cNtpProfile_);
    auto* addRow = new QHBoxLayout;
    eNtpCustom_ = new QLineEdit; eNtpCustom_->setPlaceholderText("custom.server.tld");
    bAddNtp_ = new QPushButton("Add");
    addRow->addWidget(eNtpCustom_); addRow->addWidget(bAddNtp_);
    ntpLay->addLayout(addRow);
    lwNtpServers_ = new QListWidget; lwNtpServers_->setMaximumHeight(100);
    ntpLay->addWidget(lwNtpServers_);
    auto* removeBtn = new QPushButton("Remove selected");
    ntpLay->addWidget(removeBtn);

    bQueryNtp_ = new QPushButton("Query this server now (real UDP/NTP)");
    ntpLay->addWidget(bQueryNtp_);
    lNtpStatus_ = new QLabel(" ");
    lNtpStatus_->setWordWrap(true);
    ntpLay->addWidget(lNtpStatus_);
    auto* ntpHint = new QLabel("Unlike a browser (which can't open raw UDP sockets), this native app can query\n"
                                "an NTP server directly and compute the offset from your system clock.");
    ntpHint->setWordWrap(true);
    ntpLay->addWidget(ntpHint);
    v->addWidget(ntpBox);

    v->addStretch();
    area->setWidget(page);

    connect(rSourceLive_, &QRadioButton::toggled, this, [this](bool on){
        if (!on) return;
        state_->clockSource = ClockSource::Live;
        emitChanged();
    });
    connect(rSourceCustom_, &QRadioButton::toggled, this, [this](bool on){
        if (!on) return;
        state_->clockSource = ClockSource::Custom;
        if (state_->customBaseMs == 0) {
            // seed from the live clock, not effectiveNowMsFn_ — the clock
            // source is switching *to* custom right now, so there's no
            // established custom reference point to read from yet.
            state_->customBaseMs = QDateTime::currentMSecsSinceEpoch() + state_->syncOffsetMs;
            state_->customPlaying = true;
        }
        emitChanged();
    });

    connect(bApplyCustom_, &QPushButton::clicked, this, [this]() {
        QDateTime v = eCustom_->dateTime();
        qint64 ms = zonedWallTimeToUtcMs(v, state_->tz);
        state_->customBaseMs = ms;
        state_->customPlaying = true;
        bPlayPause_->setText("Pause");
        emitChanged();
    });
    connect(bPlayPause_, &QPushButton::clicked, this, [this]() {
        if (state_->customPlaying) {
            if (effectiveNowMsFn_) state_->customBaseMs = effectiveNowMsFn_();
            state_->customPlaying = false;
        } else {
            state_->customPlaying = true;
        }
        bPlayPause_->setText(state_->customPlaying ? "Pause" : "Play");
        emitChanged();
    });

    connect(bClearSync_, &QPushButton::clicked, this, [this]() {
        state_->syncOffsetMs = 0;
        emitChanged();
    });

    connect(cNtpProfile_, &QComboBox::currentTextChanged, this, [this](const QString& t){
        if (!t.isEmpty()) { state_->ntpProfile = t; emitChanged(); }
    });
    connect(bAddNtp_, &QPushButton::clicked, this, [this]() {
        QString v = eNtpCustom_->text().trimmed();
        if (v.isEmpty() || state_->ntpServers.contains(v)) return;
        state_->ntpServers << v;
        eNtpCustom_->clear();
        refreshFromState();
        emitChanged();
    });
    connect(removeBtn, &QPushButton::clicked, this, [this]() {
        auto* item = lwNtpServers_->currentItem();
        if (!item) return;
        state_->ntpServers.removeAll(item->text());
        if (state_->ntpServers.isEmpty()) state_->ntpServers << "pool.ntp.org";
        refreshFromState();
        emitChanged();
    });
    connect(bQueryNtp_, &QPushButton::clicked, this, [this]() {
        bQueryNtp_->setEnabled(false);
        lNtpStatus_->setText("Querying " + state_->ntpProfile + " …");
        sntp_.query(state_->ntpProfile);
    });

    return area;
}

// ---------------------------------------------------------------- Polaris
QWidget* SettingsPanel::buildPolarisTab() {
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);

    auto* box = new QGroupBox("Polaris hour angle");
    auto* boxLay = new QVBoxLayout(box);
    cPolarisDial_ = new QCheckBox("Show position dial");
    cPolarisHA_ = new QCheckBox("Show hour:minute readout");
    boxLay->addWidget(cPolarisDial_);
    boxLay->addWidget(cPolarisHA_);
    auto* hint = new QLabel("Shows where Polaris currently sits around the North Celestial Pole, for\n"
                             "aligning a polar scope. The small circle rides the rim and turns\n"
                             "counter-clockwise as hour angle advances.");
    hint->setWordWrap(true);
    boxLay->addWidget(hint);
    v->addWidget(box);

    auto* zeroBox = new QGroupBox("Zero position");
    auto* zeroLay = new QHBoxLayout(zeroBox);
    rZeroTop_ = new QRadioButton("0 at top");
    rZeroBottom_ = new QRadioButton("0 at bottom");
    zeroLay->addWidget(rZeroTop_); zeroLay->addWidget(rZeroBottom_);
    v->addWidget(zeroBox);
    auto* zeroHint = new QLabel("Match this to your polar scope's reticle — a 180° flip, typical when the\n"
                                 "reticle is viewed through a right-angle finder on a German equatorial mount.");
    zeroHint->setWordWrap(true);
    v->addWidget(zeroHint);

    v->addStretch();

    connect(cPolarisDial_, &QCheckBox::toggled, this, [this](bool on){ state_->polarisShowDial = on; emitChanged(); });
    connect(cPolarisHA_, &QCheckBox::toggled, this, [this](bool on){ state_->polarisShowHA = on; emitChanged(); });
    connect(rZeroTop_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->polarisZero=PolarisZero::Top; emitChanged(); } });
    connect(rZeroBottom_, &QRadioButton::toggled, this, [this](bool on){ if(on){ state_->polarisZero=PolarisZero::Bottom; emitChanged(); } });

    return page;
}

// --------------------------------------------------------------- refresh
void SettingsPanel::refreshFromState() {
    rLayoutBoth_->setChecked(state_->layout == Layout::Both);
    rLayoutAnalog_->setChecked(state_->layout == Layout::Analog);
    rLayoutDigital_->setChecked(state_->layout == Layout::Digital);

    rPosBeneath_->setChecked(state_->digitalPosition == DigitalPosition::Beneath);
    rPosRight_->setChecked(state_->digitalPosition == DigitalPosition::Right);
    rPosLeft_->setChecked(state_->digitalPosition == DigitalPosition::Left);

    rAnalogShowsSid_->setChecked(state_->analogShows == ShowMode::Sidereal);
    rAnalogShowsLoc_->setChecked(state_->analogShows == ShowMode::Local);
    rAnalogShowsBoth_->setChecked(state_->analogShows == ShowMode::Both);

    rStyleMinimal_->setChecked(state_->analogStyle == AnalogStyle::Minimalist);
    rStyleClassic_->setChecked(state_->analogStyle == AnalogStyle::Classic);
    rStyleObservatory_->setChecked(state_->analogStyle == AnalogStyle::Observatory);
    rStyleSkeleton_->setChecked(state_->analogStyle == AnalogStyle::Skeleton);

    rDial24_->setChecked(state_->dialHours == "24");
    rDial12_->setChecked(state_->dialHours == "12");

    rDigShowsSid_->setChecked(state_->digitalShows == ShowMode::Sidereal);
    rDigShowsLoc_->setChecked(state_->digitalShows == ShowMode::Local);
    rDigShowsBoth_->setChecked(state_->digitalShows == ShowMode::Both);

    rFormat24_->setChecked(state_->digitalFormat == "24");
    rFormat12_->setChecked(state_->digitalFormat == "12");
    cSeconds_->setChecked(state_->digitalSeconds);

    rNumMechanical_->setChecked(state_->digitalNumeral == DigitalNumeral::Mechanical);
    rNumEngraved_->setChecked(state_->digitalNumeral == DigitalNumeral::Engraved);

    cNight_->setChecked(state_->night);
    cAutoNight_->setChecked(state_->autoNight);

    eLocName_->setText(state_->locName);
    eLat_->setValue(state_->lat);
    eLon_->setValue(state_->lon);
    int tzIdx = cTz_->findText(state_->tz);
    if (tzIdx < 0 && !state_->tz.isEmpty()) {
        // The system's own zone (e.g. "Etc/UTC" in a minimal container) isn't
        // always among Qt's enumerated IANA ids — add it rather than silently
        // falling back to whatever the combo box defaults to.
        cTz_->addItem(state_->tz);
        tzIdx = cTz_->count() - 1;
    }
    if (tzIdx >= 0) cTz_->setCurrentIndex(tzIdx);

    rSourceLive_->setChecked(state_->clockSource == ClockSource::Live);
    rSourceCustom_->setChecked(state_->clockSource == ClockSource::Custom);
    bPlayPause_->setText(state_->customPlaying ? "Pause" : "Play");

    double s = state_->syncOffsetMs / 1000.0;
    lSyncOffset_->setText(QString("Offset applied to the system clock: %1%2 s").arg(s >= 0 ? "+" : "").arg(s, 0, 'f', 2));

    cNtpProfile_->blockSignals(true);
    cNtpProfile_->clear();
    cNtpProfile_->addItems(state_->ntpServers);
    int profIdx = cNtpProfile_->findText(state_->ntpProfile);
    if (profIdx < 0 && !state_->ntpServers.isEmpty()) profIdx = 0;
    if (profIdx >= 0) cNtpProfile_->setCurrentIndex(profIdx);
    cNtpProfile_->blockSignals(false);

    lwNtpServers_->clear();
    lwNtpServers_->addItems(state_->ntpServers);

    cPolarisDial_->setChecked(state_->polarisShowDial);
    cPolarisHA_->setChecked(state_->polarisShowHA);
    rZeroTop_->setChecked(state_->polarisZero == PolarisZero::Top);
    rZeroBottom_->setChecked(state_->polarisZero == PolarisZero::Bottom);
}
