#include "customvideosurface.h"
#include <QDebug>
#include <QVideoSurfaceFormat>
#include <QBuffer>

CustomVideoSurface::CustomVideoSurface(QObject *parent) :
    QAbstractVideoSurface(parent)
{
}

bool CustomVideoSurface::present(const QVideoFrame &frame)
{
    if (!const_cast<QVideoFrame&>(frame).map(QAbstractVideoBuffer::ReadOnly)) {
        qDebug() << "Failed to map frame";
    } else {
//        qDebug() << frame.bits() << frame.width() << frame.height() << frame.bytesPerLine() << frame.imageFormatFromPixelFormat(frame.pixelFormat());
        QImage image(frame.bits(), frame.width(), frame.height(), frame.bytesPerLine(), frame.imageFormatFromPixelFormat(frame.pixelFormat()));

        QByteArray array;
        QBuffer buffer(&array);
        buffer.open(QIODevice::WriteOnly);
        if (!image.save(&buffer, "JPG", 70))
            qDebug() << "Failed to save image";
        else {
            emit imageSaved(array);
//            qDebug() << "Image size:" << array.size();
        }
        const_cast<QVideoFrame&>(frame).unmap();
    }
    return true;
}

QList<QVideoFrame::PixelFormat> CustomVideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const
{
    QList<QVideoFrame::PixelFormat> list;
    if (type != QAbstractVideoBuffer::NoHandle) {
        qDebug() << "Unsupported handle type:" << type;
        return list;
    }

    qDebug() << Q_FUNC_INFO;

    list.append(QVideoFrame::Format_ARGB32);
//    list.append(QVideoFrame::Format_ARGB32_Premultiplied);
    list.append(QVideoFrame::Format_RGB32);
    list.append(QVideoFrame::Format_RGB24);
//    list.append(QVideoFrame::Format_RGB565);
//    list.append(QVideoFrame::Format_RGB555);
//    list.append(QVideoFrame::Format_ARGB8565_Premultiplied);
    list.append(QVideoFrame::Format_BGRA32);
//    list.append(QVideoFrame::Format_BGRA32_Premultiplied);
    list.append(QVideoFrame::Format_BGR32);
    list.append(QVideoFrame::Format_BGR24);
//    list.append(QVideoFrame::Format_BGR565);
//    list.append(QVideoFrame::Format_BGR555);
//    list.append(QVideoFrame::Format_BGRA5658_Premultiplied);
//    list.append(QVideoFrame::Format_AYUV444);
//    list.append(QVideoFrame::Format_AYUV444_Premultiplied);
//    list.append(QVideoFrame::Format_YUV444);
//    list.append(QVideoFrame::Format_YUV420P);
//    list.append(QVideoFrame::Format_YV12);
//    list.append(QVideoFrame::Format_UYVY);
//    list.append(QVideoFrame::Format_YUYV);
//    list.append(QVideoFrame::Format_NV12);
//    list.append(QVideoFrame::Format_NV21);
    return list;
}
