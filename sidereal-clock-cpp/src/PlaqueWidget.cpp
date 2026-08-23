#include "PlaqueWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

static QLabel* makeCaption(const QString& text) {
    auto* l = new QLabel(text);
    QFont f = l->font();
    f.setPointSize(9);
    f.setBold(true);
    f.setLetterSpacing(QFont::PercentageSpacing, 115);
    l->setFont(f);
    l->setAlignment(Qt::AlignCenter);
    return l;
}

PlaqueWidget::PlaqueWidget(QWidget* parent) : QWidget(parent) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(22, 16, 22, 14);
    outer->setSpacing(14);

    // sidereal block
    siderealBlock_ = new QWidget(this);
    auto* sLay = new QVBoxLayout(siderealBlock_);
    sLay->setContentsMargins(0, 0, 0, 0);
    sLay->setSpacing(6);
    auto* sCap = makeCaption("SIDEREAL TIME");
    siderealNum_ = new QLabel("00:00:00");
    siderealNum_->setAlignment(Qt::AlignCenter);
    sLay->addWidget(sCap);
    sLay->addWidget(siderealNum_);

    // local block
    localBlock_ = new QWidget(this);
    auto* lLay = new QVBoxLayout(localBlock_);
    lLay->setContentsMargins(0, 0, 0, 0);
    lLay->setSpacing(6);
    auto* lCap = makeCaption("LOCAL TIME");
    auto* lRow = new QWidget(localBlock_);
    auto* lRowLay = new QHBoxLayout(lRow);
    lRowLay->setContentsMargins(0, 0, 0, 0);
    lRowLay->setAlignment(Qt::AlignCenter);
    localNum_ = new QLabel("00:00:00");
    localPeriod_ = new QLabel("");
    lRowLay->addWidget(localNum_);
    lRowLay->addWidget(localPeriod_);
    lLay->addWidget(lCap);
    lLay->addWidget(lRow);

    subLabel_ = new QLabel("Greenwich Mean Sidereal Time · —");
    subLabel_->setAlignment(Qt::AlignCenter);

    outer->addWidget(siderealBlock_);
    outer->addWidget(localBlock_);
    outer->addWidget(subLabel_);

    restyle();
}

void PlaqueWidget::setTheme(const Theme& t) { theme_ = t; restyle(); }
void PlaqueWidget::setNumeralStyle(DigitalNumeral n) { numeral_ = n; restyle(); }
void PlaqueWidget::setShows(ShowMode m) { shows_ = m; restyle(); }

void PlaqueWidget::setSiderealText(const QString& s) { siderealNum_->setText(s); }
void PlaqueWidget::setLocalText(const QString& s, const QString& period) {
    localNum_->setText(s);
    localPeriod_->setText(period);
}
void PlaqueWidget::setSubText(const QString& s) { subLabel_->setText(s); }

void PlaqueWidget::restyle() {
    QString bg = QString("background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 %1, stop:1 %2); "
                          "border: 1px solid %3; border-radius: 10px;")
                     .arg(theme_.ink800.name(), theme_.ink900.name(), theme_.inkLine.name());
    setStyleSheet("PlaqueWidget { " + bg + " }");

    QFont numFont;
    if (numeral_ == DigitalNumeral::Engraved) {
        numFont = QFont("Georgia");
        numFont.setItalic(true);
        numFont.setWeight(QFont::DemiBold);
    } else {
        numFont = QFont("Monospace");
        numFont.setStyleHint(QFont::Monospace);
        numFont.setWeight(QFont::DemiBold);
    }
    numFont.setPointSize(shows_ == ShowMode::Both ? 26 : 40);

    QString numColor = QString("color: %1;").arg(theme_.brassBright.name());
    siderealNum_->setFont(numFont);
    siderealNum_->setStyleSheet(numColor);
    localNum_->setFont(numFont);
    localNum_->setStyleSheet(numColor);

    QFont periodFont("Georgia"); periodFont.setItalic(true); periodFont.setPointSize(12);
    localPeriod_->setFont(periodFont);
    localPeriod_->setStyleSheet(QString("color:%1;").arg(theme_.textLo.name()));

    QString capColor = QString("color:%1;").arg(theme_.textLo.name());
    for (QLabel* cap : findChildren<QLabel*>()) {
        if (cap == siderealNum_ || cap == localNum_ || cap == localPeriod_ || cap == subLabel_) continue;
        cap->setStyleSheet(capColor);
    }
    subLabel_->setStyleSheet(QString("color:%1; font-size:11px;").arg(theme_.textFaint.name()));

    siderealBlock_->setVisible(shows_ == ShowMode::Sidereal || shows_ == ShowMode::Both);
    localBlock_->setVisible(shows_ == ShowMode::Local || shows_ == ShowMode::Both);
}
