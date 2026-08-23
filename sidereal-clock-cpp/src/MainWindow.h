#pragma once
#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QLabel>
#include <QFrame>
#include <QDockWidget>
#include <QBoxLayout>
#include <QTimeZone>
#include "AppState.h"
#include "DialWidget.h"
#include "PolarisWidget.h"
#include "PlaqueWidget.h"
#include "SettingsPanel.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);

private slots:
    void tick();
    void onStateChanged();
    void adjustDialScale(int steps);
    void adjustPolarisScale(int steps);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void buildUi();
    void buildToolbar();
    void buildFooter();
    void rebuildStage();
    void applyThemeAndStyles();
    qint64 effectiveNowMs() const;
    QTimeZone currentTz() const;
    double localHourFraction(qint64 nowMs) const;
    QString formatLocalTime(qint64 nowMs, bool* isPmOut) const;

    AppState state_;

    // persistent widgets, reparented as needed by rebuildStage()
    DialWidget* dialSidereal_;
    DialWidget* dialLocal_;
    QLabel* captionSidereal_;
    QLabel* captionLocal_;
    QWidget* dialsContainer_;
    PlaqueWidget* plaque_;

    QWidget* centralWidget_;
    QWidget* stageWidget_;
    QBoxLayout* stageLayout_ = nullptr;
    QBoxLayout* dialsLayout_ = nullptr;

    QFrame* polarisPanel_;
    PolarisWidget* polarisDial_;
    QLabel* polarisHaLabel_;
    QLabel* polarisHaCaption_;
    QLabel* polarisCaption_;

    QDockWidget* dock_;
    SettingsPanel* settingsPanel_;

    QAction* actNight_;

    // footer labels
    QLabel *fUtc_, *fLocal_, *fGmst_, *fSun_, *fLoc_;

    QTimer timer_;
    QElapsedTimer perfClock_;
    qint64 customCapturedRealMs_ = 0;
    qint64 prevCustomBaseMs_ = 0;
    bool prevCustomPlaying_ = true;

    QString lastSiderealStr_, lastLocalStr_, lastPolarisStr_;
};
