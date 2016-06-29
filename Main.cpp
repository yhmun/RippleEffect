#include "Window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Ripple Effect");
    app.setApplicationVersion("0.1");

    Window window;
    window.show();

    return app.exec();
}
