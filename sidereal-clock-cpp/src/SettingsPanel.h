#pragma once
#include <functional>
#include <QWidget>
#include <QTabWidget>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include "AppState.h"
#include "SntpClient.h"

// The settings drawer: four tabs (Display / Location / Time & Sync /
// Polaris) mirroring the web version's settings panel. Owns no state of
// its own beyond widget wiring — it mutates the AppState it's given and
// emits stateChanged() so MainWindow can re-apply everything and persist.
class SettingsPanel : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPanel(AppState* state, QWidget* parent = nullptr);

    void refreshFromState();
    void setEffectiveNowProvider(std::function<qint64()> fn) { effectiveNowMsFn_ = std::move(fn); }

signals:
    void stateChanged();

private:
    QWidget* buildDisplayTab();
    QWidget* buildLocationTab();
    QWidget* buildTimeTab();
    QWidget* buildPolarisTab();

    void emitChanged() { emit stateChanged(); }

    AppState* state_;
    QTabWidget* tabs_;
    std::function<qint64()> effectiveNowMsFn_;
    qint64 customCapturedRealMs_ = 0;

    // Display tab
    QRadioButton *rLayoutBoth_, *rLayoutAnalog_, *rLayoutDigital_;
    QRadioButton *rPosBeneath_, *rPosRight_, *rPosLeft_;
    QRadioButton *rAnalogShowsSid_, *rAnalogShowsLoc_, *rAnalogShowsBoth_;
    QRadioButton *rStyleMinimal_, *rStyleClassic_, *rStyleObservatory_, *rStyleSkeleton_;
    QRadioButton *rDial24_, *rDial12_;
    QRadioButton *rDigShowsSid_, *rDigShowsLoc_, *rDigShowsBoth_;
    QRadioButton *rFormat24_, *rFormat12_;
    QCheckBox *cSeconds_;
    QRadioButton *rNumMechanical_, *rNumEngraved_;
    QCheckBox *cNight_, *cAutoNight_;

    // Location tab
    QLineEdit* eLocName_;
    QDoubleSpinBox *eLat_, *eLon_;
    QComboBox* cTz_;

    // Time & Sync tab
    QRadioButton *rSourceLive_, *rSourceCustom_;
    QDateTimeEdit* eCustom_;
    QPushButton *bApplyCustom_, *bPlayPause_;
    QLabel* lSyncOffset_;
    QPushButton* bClearSync_;
    QComboBox* cNtpProfile_;
    QLineEdit* eNtpCustom_;
    QPushButton* bAddNtp_;
    QListWidget* lwNtpServers_;
    QPushButton* bQueryNtp_;
    QLabel* lNtpStatus_;
    SntpClient sntp_;

    // Polaris tab
    QCheckBox *cPolarisDial_, *cPolarisHA_;
    QRadioButton *rZeroTop_, *rZeroBottom_;

    QPushButton* bReset_;
};
