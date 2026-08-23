#include "MainWindow.h"
#include "Astro.h"
#include <QToolBar>
#include <QAction>
#include <QStatusBar>
#include <QDateTime>
#include <QTimeZone>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QEvent>
#include <QWheelEvent>
#include <cmath>
#include <algorithm>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    state_.load();
    perfClock_.start();
    prevCustomBaseMs_ = state_.customBaseMs;
    prevCustomPlaying_ = state_.customPlaying;
    customCapturedRealMs_ = perfClock_.elapsed();

    buildUi();
    buildToolbar();
    buildFooter();

    connect(dialSidereal_, &DialWidget::scaleRequested, this, &MainWindow::adjustDialScale);
    connect(dialLocal_, &DialWidget::scaleRequested, this, &MainWindow::adjustDialScale);
    connect(polarisDial_, &PolarisWidget::scaleRequested, this, &MainWindow::adjustPolarisScale);
    polarisPanel_->installEventFilter(this);

    settingsPanel_->refreshFromState();
    settingsPanel_->setEffectiveNowProvider([this]() { return effectiveNowMs(); });
    connect(settingsPanel_, &SettingsPanel::stateChanged, this, &MainWindow::onStateChanged);

    applyThemeAndStyles();
    rebuildStage();

    connect(&timer_, &QTimer::timeout, this, &MainWindow::tick);
    timer_.start(33);
    tick();

    resize(1200, 820);
    setWindowTitle("Sidereal Clock");
}

void MainWindow::buildUi() {
    centralWidget_ = new QWidget(this);
    setCentralWidget(centralWidget_);

    dialSidereal_ = new DialWidget;
    dialLocal_ = new DialWidget;
    dialSidereal_->setToolTip("Ctrl+scroll to resize");
    dialLocal_->setToolTip("Ctrl+scroll to resize");
    captionSidereal_ = new QLabel("SIDEREAL");
    captionLocal_ = new QLabel("LOCAL");
    captionSidereal_->setAlignment(Qt::AlignCenter);
    captionLocal_->setAlignment(Qt::AlignCenter);

    dialsContainer_ = new QWidget;

    plaque_ = new PlaqueWidget;

    stageWidget_ = new QWidget(centralWidget_);

    // Polaris inset — a small fixed panel in the corner, independent of the
    // main stage layout, so it never competes for space with the dial(s).
    polarisPanel_ = new QFrame(centralWidget_);
    auto* polarisLay = new QVBoxLayout(polarisPanel_);
    polarisCaption_ = new QLabel("POLARIS HOUR ANGLE");
    QFont capFont = polarisCaption_->font(); capFont.setBold(true); capFont.setPointSize(8);
    polarisCaption_->setFont(capFont);
    polarisDial_ = new PolarisWidget;
    polarisDial_->setFixedSize(120, 120);
    polarisDial_->setToolTip("Ctrl+scroll to resize");
    polarisPanel_->setToolTip("Ctrl+scroll to resize");
    polarisHaCaption_ = new QLabel("Hour angle");
    polarisHaLabel_ = new QLabel("00:00");
    QFont haFont; haFont.setStyleHint(QFont::Monospace); haFont.setPointSize(16); haFont.setBold(true);
    polarisHaLabel_->setFont(haFont);
    polarisHaLabel_->setAlignment(Qt::AlignCenter);
    polarisHaCaption_->setAlignment(Qt::AlignCenter);
    polarisLay->addWidget(polarisCaption_);
    polarisLay->addWidget(polarisDial_, 0, Qt::AlignCenter);
    polarisLay->addWidget(polarisHaCaption_);
    polarisLay->addWidget(polarisHaLabel_);
    polarisPanel_->setFrameShape(QFrame::StyledPanel);
    polarisPanel_->adjustSize();

    // The Polaris HUD gets its own reserved column in a real layout — not a
    // floating overlay — so it can never overlap the main dial(s) no matter
    // how the window is resized or how many dials are shown.
    auto* rootLay = new QHBoxLayout(centralWidget_);
    auto* sidebarLay = new QVBoxLayout();
    sidebarLay->addWidget(polarisPanel_, 0, Qt::AlignTop | Qt::AlignLeft);
    sidebarLay->addStretch(1);
    rootLay->addLayout(sidebarLay, 0);
    rootLay->addWidget(stageWidget_, 1);

    dock_ = new QDockWidget("Instrument settings", this);
    dock_->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    settingsPanel_ = new SettingsPanel(&state_, dock_);
    dock_->setWidget(settingsPanel_);
    dock_->setMinimumWidth(340);
    addDockWidget(Qt::RightDockWidgetArea, dock_);
    dock_->hide();
}

void MainWindow::buildToolbar() {
    auto* tb = addToolBar("Main");
    tb->setMovable(false);
    tb->setIconSize(QSize(18, 18));

    auto* titleWidget = new QWidget;
    auto* titleLay = new QHBoxLayout(titleWidget);
    titleLay->setContentsMargins(6, 0, 6, 0);
    auto* brand = new QLabel("<i><b>Sidereal</b></i>");
    brand->setTextFormat(Qt::RichText);
    QFont bf = brand->font(); bf.setPointSize(14); brand->setFont(bf);
    auto* sub = new QLabel("  LOCAL MEAN SIDEREAL TIME");
    QFont sf = sub->font(); sf.setPointSize(8); sf.setBold(true); sub->setFont(sf);
    titleLay->addWidget(brand);
    titleLay->addWidget(sub);
    tb->addWidget(titleWidget);

    auto* spacer = new QWidget; spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacer);

    actNight_ = tb->addAction("☾ Night", this, [this]() {
        state_.night = !state_.night;
        if (state_.night) state_.autoNight = false;
        onStateChanged();
        settingsPanel_->refreshFromState();
    });
    actNight_->setCheckable(true);

    tb->addAction("⤢ Fullscreen", this, [this]() {
        if (isFullScreen()) showNormal(); else showFullScreen();
    });

    tb->addAction("⚙ Settings", this, [this]() {
        dock_->setVisible(!dock_->isVisible());
    });
}

void MainWindow::buildFooter() {
    auto* sb = statusBar();
    fUtc_ = new QLabel("UTC —");
    fLocal_ = new QLabel("Local —");
    fGmst_ = new QLabel("GMST —");
    fSun_ = new QLabel("Sun alt. —");
    fLoc_ = new QLabel("—");
    for (QLabel* l : { fUtc_, fLocal_, fGmst_, fSun_ }) {
        QFont f; f.setStyleHint(QFont::Monospace); l->setFont(f);
        sb->addWidget(l);
    }
    sb->addPermanentWidget(fLoc_);
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == polarisPanel_ && event->type() == QEvent::Wheel) {
        auto* we = static_cast<QWheelEvent*>(event);
        if (we->modifiers() & Qt::ControlModifier) {
            adjustPolarisScale(we->angleDelta().y() > 0 ? 1 : -1);
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::adjustDialScale(int steps) {
    state_.dialScale = std::clamp(state_.dialScale + steps * 0.05, 0.5, 2.5);
    rebuildStage();
    state_.save();
}

void MainWindow::adjustPolarisScale(int steps) {
    state_.polarisScale = std::clamp(state_.polarisScale + steps * 0.05, 0.5, 2.5);
    applyThemeAndStyles();
    state_.save();
}

void MainWindow::onStateChanged() {
    bool resumedPlaying = (!prevCustomPlaying_ && state_.customPlaying);
    bool baseChanged = (state_.customBaseMs != prevCustomBaseMs_);
    if (baseChanged || resumedPlaying) customCapturedRealMs_ = perfClock_.elapsed();
    prevCustomBaseMs_ = state_.customBaseMs;
    prevCustomPlaying_ = state_.customPlaying;

    applyThemeAndStyles();
    rebuildStage();
    state_.save();
}

void MainWindow::applyThemeAndStyles() {
    Theme theme = state_.night ? Theme::night() : Theme::day();

    dialSidereal_->setTheme(theme);
    dialLocal_->setTheme(theme);
    polarisDial_->setTheme(theme);
    plaque_->setTheme(theme);

    dialSidereal_->setStyleMode(state_.analogStyle);
    dialLocal_->setStyleMode(state_.analogStyle);
    dialSidereal_->setIs12Hour(state_.dialHours == "12");
    dialLocal_->setIs12Hour(state_.dialHours == "12");

    plaque_->setNumeralStyle(state_.digitalNumeral);
    plaque_->setShows(state_.digitalShows);

    polarisDial_->setZeroBottom(state_.polarisZero == PolarisZero::Bottom);

    actNight_->setChecked(state_.night);

    QString bg = theme.ink950.name();
    QString text = theme.textHi.name();
    QString panelBg = theme.ink800.name();
    QString line = theme.inkLine.name();
    setStyleSheet(QString(
        "QMainWindow, QDialog { background: %1; color: %2; }"
        "QToolBar { background: %1; border: none; }"
        "QStatusBar { background: %1; color: %3; }"
        "QStatusBar::item { border: none; }"
        "QLabel { color: %3; background: transparent; }"
        "QDockWidget { color: %2; background: %1; }"
        "QDockWidget::title { background: %6; color: %2; padding: 6px; }"
        "QGroupBox { color: %2; background: %6; border: 1px solid %4; border-radius: 6px; margin-top: 8px; padding-top: 6px;}"
        "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; color: %2; }"
        "QScrollArea, QTabWidget::pane { background: %1; border: none; }"
        "QScrollArea > QWidget > QWidget { background: %1; }"
        "QTabBar::tab { background: %1; color: %3; padding: 6px 10px; }"
        "QTabBar::tab:selected { color: %5; border-bottom: 2px solid %5; }"
        "QRadioButton, QCheckBox { color: %2; background: transparent; spacing: 6px; }"
        "QLineEdit, QDoubleSpinBox, QComboBox, QDateTimeEdit, QListWidget { background: %1; color: %2; border: 1px solid %4; border-radius: 4px; padding: 4px; }"
        "QPushButton { background: %6; color: %2; border: 1px solid %4; border-radius: 4px; padding: 6px 10px; }"
        "QPushButton:hover { border-color: %5; color: %5; }"
        "QListWidget { color: %2; }"
    ).arg(bg, text, theme.textLo.name(), line, theme.brassBright.name(), panelBg));

    polarisPanel_->setStyleSheet(QString(
        "QFrame { background: %1; border: 1px solid %2; border-radius: 10px; }"
        "QLabel { color: %3; background: transparent; border: none; }"
    ).arg(panelBg, line, theme.textLo.name()));
    polarisHaLabel_->setStyleSheet(QString("color:%1; background: transparent;").arg(theme.brassBright.name()));

    QString capStyle = QString("color:%1; background: transparent;").arg(theme.textLo.name());
    captionSidereal_->setStyleSheet(capStyle);
    captionLocal_->setStyleSheet(capStyle);

    polarisPanel_->setVisible(state_.polarisShowDial || state_.polarisShowHA);
    polarisDial_->setVisible(state_.polarisShowDial);
    polarisHaLabel_->setVisible(state_.polarisShowHA);
    polarisHaCaption_->setVisible(state_.polarisShowHA);

    int polarisDim = int(120 * state_.polarisScale);
    polarisDial_->setFixedSize(polarisDim, polarisDim);
    polarisPanel_->adjustSize();
}

void MainWindow::rebuildStage() {
    // detach persistent widgets from whatever layout currently holds them
    if (dialsLayout_) {
        dialsLayout_->removeWidget(dialSidereal_);
        dialsLayout_->removeWidget(dialLocal_);
        dialsLayout_->removeWidget(captionSidereal_);
        dialsLayout_->removeWidget(captionLocal_);
        delete dialsContainer_->layout();
    }
    bool showSid = (state_.analogShows == ShowMode::Sidereal || state_.analogShows == ShowMode::Both);
    bool showLoc = (state_.analogShows == ShowMode::Local || state_.analogShows == ShowMode::Both);
    bool showBoth = (state_.analogShows == ShowMode::Both);

    auto* newDialsLayout = new QHBoxLayout();
    newDialsLayout->setSpacing(28);
    if (showSid) {
        if (showBoth) {
            auto* col = new QVBoxLayout(); col->addWidget(captionSidereal_); col->addWidget(dialSidereal_);
            newDialsLayout->addLayout(col);
        } else {
            newDialsLayout->addWidget(dialSidereal_);
        }
    }
    if (showLoc) {
        if (showBoth) {
            auto* col = new QVBoxLayout(); col->addWidget(captionLocal_); col->addWidget(dialLocal_);
            newDialsLayout->addLayout(col);
        } else {
            newDialsLayout->addWidget(dialLocal_);
        }
    }
    dialsContainer_->setLayout(newDialsLayout);
    dialsLayout_ = newDialsLayout;
    captionSidereal_->setVisible(showBoth);
    captionLocal_->setVisible(showBoth);

    int baseDim = showBoth ? 300 : 420;
    int dialDim = int(baseDim * state_.dialScale);
    dialSidereal_->setMinimumSize(dialDim, dialDim);
    dialLocal_->setMinimumSize(dialDim, dialDim);

    if (stageLayout_) {
        stageLayout_->removeWidget(dialsContainer_);
        stageLayout_->removeWidget(plaque_);
        delete stageWidget_->layout();
    }
    QBoxLayout* newStageLayout = nullptr;
    bool reverse = (state_.digitalPosition == DigitalPosition::Left);
    switch (state_.digitalPosition) {
        case DigitalPosition::Beneath: newStageLayout = new QVBoxLayout(); break;
        case DigitalPosition::Right:
        case DigitalPosition::Left: newStageLayout = new QHBoxLayout(); break;
    }
    newStageLayout->setAlignment(Qt::AlignCenter);
    newStageLayout->setSpacing(24);
    if (reverse) {
        newStageLayout->addWidget(plaque_);
        newStageLayout->addWidget(dialsContainer_);
    } else {
        newStageLayout->addWidget(dialsContainer_);
        newStageLayout->addWidget(plaque_);
    }
    stageWidget_->setLayout(newStageLayout);
    stageLayout_ = newStageLayout;

    dialsContainer_->setVisible(state_.layout != Layout::Digital);
    plaque_->setVisible(state_.layout != Layout::Analog);
    plaque_->setMinimumWidth(state_.layout == Layout::Digital ? 520 : 340);
}

qint64 MainWindow::effectiveNowMs() const {
    if (state_.clockSource == ClockSource::Custom) {
        if (state_.customPlaying) return state_.customBaseMs + (perfClock_.elapsed() - customCapturedRealMs_);
        return state_.customBaseMs;
    }
    return QDateTime::currentMSecsSinceEpoch() + state_.syncOffsetMs;
}

QTimeZone MainWindow::currentTz() const {
    QTimeZone tz(state_.tz.toUtf8());
    return tz.isValid() ? tz : QTimeZone::utc();
}

double MainWindow::localHourFraction(qint64 nowMs) const {
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(nowMs, currentTz());
    QTime t = dt.time();
    return t.hour() + t.minute() / 60.0 + t.second() / 3600.0;
}

QString MainWindow::formatLocalTime(qint64 nowMs, bool* isPmOut) const {
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(nowMs, currentTz());
    QTime t = dt.time();
    bool is12 = (state_.digitalFormat == "12");
    int h = t.hour();
    if (isPmOut) *isPmOut = h >= 12;
    if (is12) { h = h % 12; if (h == 0) h = 12; }
    QString s = QString("%1:%2").arg(h, 2, 10, QChar('0')).arg(t.minute(), 2, 10, QChar('0'));
    if (state_.digitalSeconds) s += QString(":%1").arg(t.second(), 2, 10, QChar('0'));
    return s;
}

void MainWindow::tick() {
    qint64 nowMs = effectiveNowMs();
    double jd = Astro::toJD(nowMs);
    double gmst = Astro::gmstHours(jd);
    double lst = gmst + state_.lon / 15.0;
    lst = std::fmod(std::fmod(lst, 24.0) + 24.0, 24.0);

    dialSidereal_->setHourFraction(lst);
    dialLocal_->setHourFraction(localHourFraction(nowMs));

    int h = int(std::floor(lst));
    int m = int(std::floor((lst - h) * 60));
    int s = int(std::floor((((lst - h) * 60) - m) * 60));
    QString sidStr = QString("%1:%2:%3").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    plaque_->setSiderealText(sidStr);

    bool isPm = false;
    QString localStr = formatLocalTime(nowMs, &isPm);
    QString period = (state_.digitalFormat == "12") ? (isPm ? "PM" : "AM") : "";
    plaque_->setLocalText(localStr, period);

    double alt = Astro::sunAltitude(state_.lat, state_.lon, jd, lst);
    Twilight twi = Astro::twilightLabel(alt);

    if (state_.autoNight) {
        bool shouldNight = alt < -12.0;
        if (shouldNight != state_.night) {
            state_.night = shouldNight;
            actNight_->setChecked(shouldNight);
            applyThemeAndStyles();
        }
    }

    int gh = int(std::floor(gmst)), gm = int(std::floor((gmst - gh) * 60));
    QString gmstStr = QString("%1:%2").arg(gh, 2, 10, QChar('0')).arg(gm, 2, 10, QChar('0'));
    plaque_->setSubText(QString("GMST %1 · %2").arg(gmstStr, state_.locName.isEmpty() ? "unset location" : state_.locName));

    // Polaris hour angle
    double polarisHa = Astro::polarisHourAngleHours(lst, jd);
    polarisDial_->setHourAngleHours(polarisHa);
    int ph = int(std::floor(polarisHa));
    int pm = int(std::floor((polarisHa - ph) * 60));
    QString polarisStr = QString("%1:%2").arg(ph, 2, 10, QChar('0')).arg(pm, 2, 10, QChar('0'));
    if (polarisStr != lastPolarisStr_) { polarisHaLabel_->setText(polarisStr); lastPolarisStr_ = polarisStr; }

    // footer
    QDateTime utcDt = QDateTime::fromMSecsSinceEpoch(nowMs, QTimeZone::utc());
    QDateTime localDt = QDateTime::fromMSecsSinceEpoch(nowMs, currentTz());
    fUtc_->setText("UTC " + utcDt.time().toString("HH:mm:ss"));
    fLocal_->setText("Local " + localDt.time().toString("HH:mm:ss"));
    fGmst_->setText("GMST " + gmstStr);
    fSun_->setText(QString("Sun alt. %1° %2").arg(alt, 0, 'f', 1).arg(twi.text));
    fLoc_->setText(state_.locName.isEmpty() ? QString("%1°, %2°").arg(state_.lat, 0, 'f', 2).arg(state_.lon, 0, 'f', 2) : state_.locName);
}
