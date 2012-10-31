#ifndef CAMERAIMAGEPROVIDER_H
#define CAMERAIMAGEPROVIDER_H

#include <qcamera.h>
#include <qcameraimagecapture.h>
#include <qmediarecorder.h>
#include "customvideosurface.h"

class QVideoRendererControl;

class CameraImageProvider : public QObject
{
    Q_OBJECT
public:
    explicit CameraImageProvider(QObject *parent = 0);
    ~CameraImageProvider();

    bool init();

signals:
    void imageData(QByteArray data);
    void frameReceived(QVideoFrame frame);

public slots:
    void stateChanged(QCamera::State state);
    void cameraError(QCamera::Error error);

private:
    QCamera *m_camera;
    QMediaRecorder *m_mediaRecorder;
    QCameraImageCapture *m_imageCapture;
    CustomVideoSurface *m_videoSurface;
    QVideoRendererControl *m_control;
};

#endif // CAMERAIMAGEPROVIDER_H
