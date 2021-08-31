#include <QReadWriteLock>
#include <QApplication>
#include <QDebug>
#include <cstdio>
#include <memory>
#include <QTime>
#include <opencv2/opencv.hpp>

#include "videomanagement.h"
#include "stgraphicsview.h"
#include "videopropertydialog.h"
#include "trackio.h"

extern Frames g_frames;
extern QReadWriteLock* g_pTrackInfosLock;

VideoManagement::VideoManagement(StGraphicsView* pWidget, QLabel* pLabelFrame, QLabel* pLabelTime, QObject *parent) :
    QObject(parent), m_pVideoWidget(pWidget), m_pLabelCurFrame(pLabelFrame), m_pLabelCurTime(pLabelTime)
{
    m_pCapture = NULL;
    dest_image = NULL;
    source_image = NULL;
    m_pQtImg = NULL;
}

bool VideoManagement::setFileName(const QString strFileName)
{
    if (m_pCapture != NULL)
    {
        cvReleaseCapture(&m_pCapture);
        m_pCapture = NULL;
    }
    if (m_pQtImg != NULL)
    {
        delete m_pQtImg;
        m_pQtImg = NULL;
    }
    dest_image = NULL;
    source_image = NULL;

    m_strFileName = strFileName;

    QByteArray byteArray = m_strFileName.toLocal8Bit();
    const char* strName = byteArray.data();
    m_pCapture = cvCaptureFromFile(strName);

    if (m_pCapture != NULL)
    {
        m_nFps = (int) cvGetCaptureProperty(m_pCapture, CV_CAP_PROP_FPS);

        bool cond1 = m_nFps <= 0;
        bool cond2 = m_nFps >= 100;
        if(cond1|cond2)
            m_nFps = 25;

        m_nFrames = (int) cvGetCaptureProperty(m_pCapture, CV_CAP_PROP_FRAME_COUNT);
        source_image = cvQueryFrame(m_pCapture);

        if (source_image != NULL)
        {
            m_pQtImg = new QImage(QSize(source_image->width, source_image->height), QImage::Format_RGB888);
            dest_image = cvCreateImageHeader(cvSize(source_image->width, source_image->height), 8, 3);
            dest_image->imageData = (char*)m_pQtImg->bits();

            m_nFrameW = source_image->width;
            m_nFrameH = source_image->height;

            qDebug() << "Resolution:" << source_image->width << "*" << source_image->height;
            qDebug() << "Fps:" << m_nFps << " Total Frames:" <<m_nFrames;

            return true;
        }
    }
    return false;
}

void VideoManagement::setCurFrame(int frameId)
{
    if (frameId < 0 || frameId >= m_nFrames)
        return;

    auto cvframe = frameId + 1;
    auto curFrame = (int)cvGetCaptureProperty(m_pCapture, CV_CAP_PROP_POS_FRAMES);

    if (cvframe != curFrame){
        cvSetCaptureProperty(m_pCapture, CV_CAP_PROP_POS_FRAMES, frameId);
        source_image = cvQueryFrame(m_pCapture);
    }
    
    if (source_image != NULL)
    {
        if (source_image->origin == IPL_ORIGIN_TL)
        {
            cvCopy(source_image, dest_image, 0);
        }
        else
        {
            cvFlip(source_image, dest_image, 0);
        }
        cvCvtColor(dest_image, dest_image, CV_BGR2RGB);
        updatewidget(frameId);
    }
}

void VideoManagement::updatewidget(int frameid)
{
    if (m_pVideoWidget != NULL)
        m_pVideoWidget->updateTracks(QPixmap::fromImage(*m_pQtImg), frameid);

    int msecs = frameid * 1000 / m_nFps;
    QTime totalTime = QTime::fromMSecsSinceStartOfDay(msecs);
    QString strTime = totalTime.toString("mm:ss");
    if (totalTime.hour() > 0)
        strTime = totalTime.toString("hh:mm:ss");

    if (m_pLabelCurFrame != NULL)
        m_pLabelCurFrame->setText(QString("%1").arg(frameid));
    if (m_pLabelCurTime != NULL)
        m_pLabelCurTime->setText(strTime);

    if (m_pVideoWidget != NULL)
        m_pVideoWidget->updateFrameId(frameid);
}

void VideoManagement::showVideoProperty()
{
    int msecs = m_nFrames * 1000 / m_nFps;
    QTime totalTime = QTime::fromMSecsSinceStartOfDay(msecs);
    QString strTime = totalTime.toString("hh:mm:ss");

    QSize resolutionSize(m_nFrameW, m_nFrameH);

    VideoPropertyDialog dlg(m_strFileName, m_nFrames, m_nFps, strTime, resolutionSize, qApp->activeWindow());
    dlg.exec();
}
