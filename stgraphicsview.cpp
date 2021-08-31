#include "stgraphicsview.h"
#include "stgraphicsscene.h"
#include <QMouseEvent>
#include <QDebug>

StGraphicsView::StGraphicsView(QWidget *parent):
    QGraphicsView(parent)
{
    this->setFrameShape(QFrame::NoFrame);
    setDragMode(QGraphicsView::ScrollHandDrag);
    m_pGraphicsScene = new StGraphicsScene(this);
    m_pGraphicsScene->setBackgroundBrush(QBrush(QColor(49,49,49)));
    setScene(m_pGraphicsScene);

    m_pCurPixmapItem = NULL;
    m_scaleValue = 1;
}

bool StGraphicsView::operation_enabled(){
    return m_pGraphicsScene->operation_enabled();
}

void StGraphicsView::enable_operation(){
    m_pGraphicsScene->enable_operation();
}

void StGraphicsView::adjustPictureSize()
{
    this->resetMatrix();
    m_scaleValue = 1;

    int iPixmapWidth = m_pixmap.width();
    int iPixmapHeight = m_pixmap.height();

    int iViewWidth = this->width();
    int iViewHeight = this->height();

    float fViewRatio = float(iViewWidth) / iViewHeight;
    float fPixmapRatio = float(iPixmapWidth) / iPixmapHeight;

    if (iPixmapWidth > iViewWidth || iPixmapHeight > iViewHeight)
    {

        if (fPixmapRatio > fViewRatio)
        {
            m_scaleValue = qreal(iViewWidth) / iPixmapWidth * 0.99;
        }
        else
        {
            m_scaleValue = qreal(iViewHeight) / iPixmapHeight * 0.99;
        }
        this->scale(m_scaleValue, m_scaleValue);
        m_pGraphicsScene->setScaleValue(m_scaleValue);
    }
}

void StGraphicsView::updateTracks(QPixmap srcPixmap, TrackIdx frame_id)
{
    if(srcPixmap.isNull())
        return;

    if (m_pixmap.isNull())
    {
        m_pixmap = srcPixmap;
        adjustPictureSize();
    }
    else
    {
        m_pixmap = srcPixmap;
    }
    m_pGraphicsScene->clearItems();
    m_pGraphicsScene->clear();

    m_pCurPixmapItem = m_pGraphicsScene->addPixmap(srcPixmap);
    m_pGraphicsScene->updateTrackRects(frame_id);

    this->update();
}

void StGraphicsView::setRectgeometry(int index)
{
    m_pGraphicsScene->setRectGeometry(index);
    this->update();
}

void StGraphicsView::updateFrameId(int frameid)
{
    m_pGraphicsScene->updateFrameId(frameid);
}


