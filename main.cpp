#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"

#include "httpdaemon.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    HttpDaemon *daemon = new HttpDaemon(8080, app.data());
    if (!daemon->isListening()) {
        qDebug() << QString("Failed to bind to port %1").arg(daemon->serverPort());
        app->quit();
    }

    QmlApplicationViewer viewer;
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/remotecontrol/main.qml"));
    viewer.showExpanded();

    return app->exec();
}
