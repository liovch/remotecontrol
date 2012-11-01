#include "httpdaemon.h"
#include <QBuffer>

HttpDaemon::HttpDaemon(quint16 port, QObject* parent)
    : QTcpServer(parent), disabled(false)
{
    m_camera = new CameraImageProvider(this);
    listen(QHostAddress::Any, port);
    m_camera->init();
    connect(m_camera, SIGNAL(frameReceived(QVideoFrame)), this, SLOT(frameReceived(QVideoFrame)));

    QBluetoothAddress address("00:12:02:28:03:34");
    m_bluetoothSocket = new QBluetoothSocket(QBluetoothSocket::RfcommSocket, this);
    connect(m_bluetoothSocket, SIGNAL(connected()), this, SLOT(bluetoothConnected()));
    connect(m_bluetoothSocket, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(bluetoothError(QBluetoothSocket::SocketError)));
    connect(m_bluetoothSocket, SIGNAL(readyRead()), this, SLOT(bluetoothDataReceived()));
    m_bluetoothSocket->connectToService(address, QBluetoothUuid::SerialPort);
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

                "<img src=\"image.jpg\" alt=\"Live stream image\" height=\"480\" width=\"640\"><br>\r\n"

                 "<script type=\"text/javascript\">\n"
                 "var ajaxObject = new XMLHttpRequest();\n"

                 "function keypressed(e){\n"
                    "var unicode=e.keyCode? e.keyCode : e.charCode;\n"
//                  "ajaxObject.abort();\n"
                    "ajaxObject.open(\"GET\",\"keypressed.php?\" + unicode,true);\n"
                    "ajaxObject.send(null);\n"
                 "}\n"

                  "function mousemoved(e){\n"
 //                 "ajaxObject.abort();\n"
                    "ajaxObject.open(\"GET\",\"mousemoved.php?\" + e.clientX + '&' + e.clientY, true);\n"
                    "ajaxObject.send(null);\n"
                  "}\n"

                 "document.onkeypress=keypressed;\n"
                 "document.onmousemove=mousemoved;\n"
                 "</script>\r\n";
            socket->close();
        } else if (tokens[0] == "GET" && tokens[1] == "/image.jpg") {
            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            os << "HTTP/1.0 200 Ok\r\n"
                "Content-Type: multipart/x-mixed-replace;"
                "boundary=magicalboundarystring\r\n"
                "\r\n--magicalboundarystring\r\n";
            if (m_imageSockets.isEmpty())
                m_camera->start();
            m_imageSockets.append(socket);
            qDebug() << "m_imageSockets size" << m_imageSockets.size();
        } else if (tokens[0] == "GET" && tokens[1].startsWith("/keypressed.php")) {
            int index = tokens[1].lastIndexOf("?");
            QString value = tokens[1].right(tokens[1].size() - index - 1);
            QString character = QString((char)value.toInt());
            character = character.toLower();
            if (character == "w") character = "s";
            else if (character == "s") character = "w";
            if (character.size() > 0) {
                qDebug() << value << character.at(0).toLatin1();
                m_bluetoothSocket->putChar(character.at(0).toLatin1());
            }
            socket->close();
        } else if (tokens[0] == "GET" && tokens[1].startsWith("/mousemoved.php")) {
            int index = tokens[1].lastIndexOf("?");
            QString values = tokens[1].right(tokens[1].size() - index - 1);
            index = values.lastIndexOf("&");
            QString x = values.left(index);
            QString y = values.right(values.size() - index - 1);
            qDebug() << x << "," << y << " Ints:" << x.toInt() << "," << y.toInt();
            // Control servos
            m_bluetoothSocket->putChar('-');
            int angleX = x.toInt() / 4;
            if (angleX > 180)
                angleX = 180;
            int angleY = y.toInt() / 3;
            if (angleY > 180)
                angleY = 180;
            m_bluetoothSocket->putChar((char)angleX);
            m_bluetoothSocket->putChar((char)angleY);
            socket->close();
        }

    } while (false);

//    if (socket->state() == QTcpSocket::UnconnectedState) {
//        m_imageSockets.removeOne(socket);
//        if (m_imageSockets.isEmpty())
//            m_camera->stop();
//        delete socket;
//        qDebug() << "Socket is unconnected. Connection closed.";
//    }
}

void HttpDaemon::discardClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    m_imageSockets.removeOne(socket);
    if (m_imageSockets.isEmpty())
        m_camera->stop();
    socket->deleteLater();

    qDebug() << "Connection closed";
}

void HttpDaemon::frameReceived(QVideoFrame frame)
{
//    qDebug() << "FrameReceived";

    static int counter = 0;
    if (counter < 3) {
        counter++;
        return;
    }
    counter = 0;

    if (!const_cast<QVideoFrame&>(frame).map(QAbstractVideoBuffer::ReadOnly)) {
        qDebug() << "Failed to map frame";
    } else {
//        qDebug() << frame.bits() << frame.width() << frame.height() << frame.bytesPerLine() << frame.imageFormatFromPixelFormat(frame.pixelFormat());
        QImage image(frame.bits(), frame.width(), frame.height(), frame.bytesPerLine(), frame.imageFormatFromPixelFormat(frame.pixelFormat()));

        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        if (!image.save(&buffer, "JPG", 30)) {
            qDebug() << "Failed to save image";
            const_cast<QVideoFrame&>(frame).unmap();
        } else {
            const_cast<QVideoFrame&>(frame).unmap();
//            qDebug() << "Image size:" << data.size();

            foreach (QTcpSocket* socket, m_imageSockets) {

                {
                    QTextStream os(socket);
                    os.setAutoDetectUnicode(true);
                    os << "Content-Type: image/jpeg\r\n"
                          "Cache-Control: no-cache\r\n"
                          "\r\n";
                }

                socket->write(data);

                {
                    QTextStream os(socket);
                    os.setAutoDetectUnicode(true);
                    os << "\r\n--magicalboundarystring\r\n";
                }
            }

        }
    }
}

void HttpDaemon::bluetoothConnected()
{
    qDebug() << "Bluetooth has connected successfully";
}

void HttpDaemon::bluetoothError(QBluetoothSocket::SocketError error)
{
    qDebug() << "Bluetooth Error:" << error;
}

void HttpDaemon::bluetoothDataReceived()
{
    qint64 bytesAvailable = m_bluetoothSocket->bytesAvailable();
    QByteArray data = m_bluetoothSocket->read(bytesAvailable);
    Q_UNUSED(data);
}
