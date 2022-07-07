#include "main_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    QObject::connect(&a, &QCoreApplication::aboutToQuit, &w,
                     &MainWindow::encode_before_quit);
    w.show();
    return a.exec();
}
