#include "cameraimageprovider.h"
#include <QVideoRendererControl>
#include <QCameraControl>
#include <QDebug>

CameraImageProvider::CameraImageProvider(QObject *parent) :
    QObject(parent),
    m_camera(0),
    m_mediaRecorder(0),
    m_imageCapture(0),
    m_control(0)
{
    m_videoSurface = new CustomVideoSurface(this);
}

CameraImageProvider::~CameraImageProvider()
{
    if (m_control) {
        m_camera->service()->releaseControl(m_control);
    }
    delete m_mediaRecorder;
    delete m_imageCapture;
    delete m_camera;
}

bool CameraImageProvider::init()
{
    m_camera = new QCamera();
    m_mediaRecorder = new QMediaRecorder(m_camera);
    m_imageCapture = new QCameraImageCapture(m_camera);
    if (!m_camera->isCaptureModeSupported(QCamera::CaptureStillImage)) {
        qDebug() << "Still image capture mode is not supported";
        return false;
    }

    QCameraControl *cameraControl = qobject_cast<QCameraControl *>(
                m_camera->service()->requestControl("com.nokia.Qt.QCameraControl/1.0"));
    if (cameraControl)
        cameraControl->setProperty("viewfinderColorSpaceConversion", true);

    m_control = qobject_cast<QVideoRendererControl *>(
                                         m_camera->service()->requestControl("com.nokia.Qt.QVideoRendererControl/1.0"));
    if (m_control) {
        m_control->setSurface(m_videoSurface);
    }

    QImageEncoderSettings imageSettings;
    imageSettings.setCodec("image/jpeg");
    imageSettings.setResolution(320, 240);
    m_imageCapture->setEncodingSettings(imageSettings);

    // Note: QCameraFocus::HyperfocalFocus and QCameraFocus::FocusPointCenter are not supported
    //focus->setFocusPointMode(QCameraFocus::FocusPointCenter);
    //focus->setFocusMode(QCameraFocus::ManualFocus);

    // TODO: Connect to camera error signal
    connect(m_camera, SIGNAL(error(QCamera::Error)), this, SLOT(cameraError(QCamera::Error)));
    connect(m_camera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(stateChanged(QCamera::State)));

    m_camera->setCaptureMode(QCamera::CaptureStillImage);

    connect(m_videoSurface, SIGNAL(frameReceived(QVideoFrame)), this, SIGNAL(frameReceived(QVideoFrame)));
    return true;
}

void CameraImageProvider::start()
{
    qDebug() << "Starting camera";
    if (m_camera)
        m_camera->start();
}

void CameraImageProvider::stop()
{
    qDebug() << "Stopping camera";
    if (m_camera)
        m_camera->stop();
}

void CameraImageProvider::stateChanged(QCamera::State state)
{
    qDebug() << "Camera state:" << state;
}

void CameraImageProvider::cameraError(QCamera::Error error)
{
    qDebug() << "Camera error:" << error;
}
