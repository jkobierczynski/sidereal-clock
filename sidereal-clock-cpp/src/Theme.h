#pragma once
#include <QColor>

// Mirrors the CSS custom-property palette from the web version, in both
// day and night (red-on-black, astronomer-friendly) variants.
struct Theme {
    QColor ink950, ink900, ink800, ink700, inkLine, inkLineSoft;
    QColor face, faceShade, faceDeep;
    QColor brass, brassBright, brassDim;
    QColor textHi, textLo, textFaint;
    QColor good, warn, crit;

    static Theme day() {
        Theme t;
        t.ink950 = QColor("#07090f"); t.ink900 = QColor("#0d1120");
        t.ink800 = QColor("#161d35"); t.ink700 = QColor("#202a4d");
        t.inkLine = QColor("#2c3660"); t.inkLineSoft = QColor("#1d2542");
        t.face = QColor("#e9e0c8"); t.faceShade = QColor("#cabb8e"); t.faceDeep = QColor("#7a6a3f");
        t.brass = QColor("#c8a248"); t.brassBright = QColor("#f0cf72"); t.brassDim = QColor("#8a6d2e");
        t.textHi = QColor("#ece8da"); t.textLo = QColor("#93a0c0"); t.textFaint = QColor("#5b6688");
        t.good = QColor("#7fae6a"); t.warn = QColor("#d69a4e"); t.crit = QColor("#c85a4a");
        return t;
    }
    static Theme night() {
        Theme t;
        t.ink950 = QColor("#050301"); t.ink900 = QColor("#0b0503");
        t.ink800 = QColor("#170905"); t.ink700 = QColor("#26100a");
        t.inkLine = QColor("#3d1712"); t.inkLineSoft = QColor("#28100c");
        t.face = QColor("#1a0906"); t.faceShade = QColor("#2a100b"); t.faceDeep = QColor("#4a1912");
        t.brass = QColor("#e2382b"); t.brassBright = QColor("#ff7160"); t.brassDim = QColor("#8f231a");
        t.textHi = QColor("#ff8a7a"); t.textLo = QColor("#b3453a"); t.textFaint = QColor("#7a2b23");
        t.good = QColor("#c0463a"); t.warn = QColor("#c0463a"); t.crit = QColor("#ff5a48");
        return t;
    }
};
