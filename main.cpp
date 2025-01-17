#include "mainwindow.h"

int main(int argc, char *argv[]){
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

    QApplication app(argc, argv);

    MainWindow w;

    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(APPLICATION_VERSION);

    w.setFixedSize(0, 0);
    w.show();

    return app.exec();
}
