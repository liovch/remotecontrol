#include "httpdaemon.h"

HttpDaemon::HttpDaemon(quint16 port, QObject* parent)
    : QTcpServer(parent), disabled(false)
{
    m_camera = new CameraImageProvider(this);
    listen(QHostAddress::Any, port);
    m_camera->init();
    connect(m_camera, SIGNAL(imageData(QByteArray)), this, SLOT(imageData(QByteArray)));
}

void HttpDaemon::incomingConnection(int socket)
{
    if (disabled)
        return;

    qDebug() << Q_FUNC_INFO;

    // When a new client connects, the server constructs a QTcpSocket and all
    // communication with the client is done over this QTcpSocket. QTcpSocket
    // works asynchronously, this means that all the communication is done
    // in the two slots readClient() and discardClient().
    QTcpSocket* s = new QTcpSocket(this);
    connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
    s->setSocketDescriptor(socket);

    qDebug() << "New Connection";
}

void HttpDaemon::readClient()
{
    if (disabled)
        return;

    // This slot is called when the client sent data to the server. The
    // server looks if it was a get request and sends a very simple HTML
    // document back.
    QTcpSocket* socket = (QTcpSocket*)sender();
    do {
        if (!socket->canReadLine())
            break;

        QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
        qDebug() << tokens;

        if (tokens.size() < 2)
            break;

        if (tokens[0] == "GET" && tokens[1] == "/") {
            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            os << "HTTP/1.0 200 Ok\r\n"
                "Content-Type: text/html; charset=\"utf-8\"\r\n"
                "\r\n"
                "<h1>Live stream:</h1>\n"
                "<img src=\"image.jpg\" alt=\"Live stream image\" height=\"480\" width=\"640\"><br>\r\n"
                << QDateTime::currentDateTime().toString() << "\n";

            socket->close();
        }

        if (tokens[0] == "GET" && tokens[1] == "/image.jpg") {
            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            os << "HTTP/1.0 200 Ok\r\n"
                "Content-type: multipart/x-mixed-replace;"
                "boundary=magicalboundarystring\n\n"
                "--magicalboundarystring\n";
            m_imageSockets.append(socket);
        }

    } while (false);

    qDebug() << "Wrote to client";

    if (socket->state() == QTcpSocket::UnconnectedState) {
        m_imageSockets.removeOne(socket);
        delete socket;
        qDebug() << "Connection closed";
    }
}

void HttpDaemon::discardClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    m_imageSockets.removeOne(socket);
    socket->deleteLater();

    qDebug() << "Connection closed";
}

void HttpDaemon::imageData(QByteArray data)
{
    foreach (QTcpSocket* socket, m_imageSockets) {
        {
            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            os << "Content-type: image/jpeg\n\n";
        }

        socket->write(data);

        {
            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            os << "\n--magicalboundarystring\n";
        }
    }
}
