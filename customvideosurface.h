#ifndef CUSTOMVIDEOSURFACE_H
#define CUSTOMVIDEOSURFACE_H

#include <QAbstractVideoSurface>

class CustomVideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit CustomVideoSurface(QObject *parent = 0);
    
    virtual bool present( const QVideoFrame & frame );
    virtual QList<QVideoFrame::PixelFormat>	supportedPixelFormats(QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const;
signals:
    void imageSaved(const QByteArray& data);
public slots:
    
};

#endif // CUSTOMVIDEOSURFACE_H
