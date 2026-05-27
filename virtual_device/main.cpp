#include <QApplication>
#include "VirtualDeviceWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("QtDeviceMonitor - VirtualDevice");

    VirtualDeviceWindow window;
    window.show();

    return app.exec();
}
