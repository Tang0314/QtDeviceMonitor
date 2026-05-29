#include "ui/mainwindow.h"
#include "utils/LogManager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LogManager::installDefault();
    MainWindow w;
    w.show();
    return a.exec();
}
