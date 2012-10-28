#ifndef HTTPDAEMON_H
#define HTTPDAEMON_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>
#include <QDateTime>
#include <QDebug>

#include "cameraimageprovider.h"

#include <QtMobility/qmobilityglobal.h>
#include <qbluetoothsocket.h>
QTM_BEGIN_NAMESPACE
class QBluetoothSocket;
QTM_END_NAMESPACE

QTM_USE_NAMESPACE

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

     void bluetoothConnected();
     void bluetoothError(QBluetoothSocket::SocketError error);
     void bluetoothDataReceived();

 private:
     void mainPage(QTcpSocket *socket);
     bool disabled;

    CameraImageProvider *m_camera;
    QList<QTcpSocket*> m_imageSockets;

    QBluetoothSocket *m_bluetoothSocket;
};

#endif // HTTPDAEMON_H
