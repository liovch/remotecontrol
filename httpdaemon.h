#ifndef HTTPDAEMON_H
#define HTTPDAEMON_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>
#include <QDateTime>
#include <QDebug>

#include "cameraimageprovider.h"

class HttpDaemon : public QTcpServer
{
    Q_OBJECT
public:
     HttpDaemon(quint16 port, QObject* parent = 0);

     void incomingConnection(int socket);

     void pause()
     {
         disabled = true;
     }

     void resume()
     {
         disabled = false;
     }

 private slots:
     void readClient();
     void discardClient();
     void imageData(QByteArray data);

 private:
     bool disabled;

    CameraImageProvider *m_camera;
    QList<QTcpSocket*> m_imageSockets;
};

#endif // HTTPDAEMON_H
