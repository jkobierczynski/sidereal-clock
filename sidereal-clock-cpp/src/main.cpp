#include <QApplication>
#include "MainWindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName("jkobierczynski");
    QApplication::setApplicationName("SiderealClock");
    QApplication::setApplicationDisplayName("Sidereal Clock");

    MainWindow w;
    w.show();
    return app.exec();
}
