#ifndef VIDEOMANAGEMENT_H
#define VIDEOMANAGEMENT_H

#include <QObject>
#include <QSlider>
#include <QLabel>
#include <opencv2/highgui.hpp>

class StGraphicsView;

class VideoManagement : public QObject
{
    Q_OBJECT
public:
    explicit VideoManagement(StGraphicsView* pWidget, QLabel* pLabelFrame, QLabel* pLabelTime, QObject *parent = 0);
    bool setFileName(const QString strFileName);
    void setCurFrame(int value);
    void showVideoProperty();

    int m_nFrames;
    int m_nFps;
    int m_nFrameW;
    int m_nFrameH;

private:
	void updatewidget(int frameid);

private:
    StGraphicsView* m_pVideoWidget;
    QLabel* m_pLabelCurFrame;
    QLabel* m_pLabelCurTime;

    QString m_strFileName;

    CvCapture *m_pCapture;
    IplImage *source_image;
    IplImage *dest_image;
    QImage *m_pQtImg;

	const static int maxstoresnums = 5;
	std::map<int, IplImage *> m_imageStoreMap;
};

#endif // VIDEOMANAGEMENT_H
