#ifndef STGRAPHICSVIEW_H
#define STGRAPHICSVIEW_H

#include <QGraphicsView>
#include "tracktypes.h"

class StGraphicsScene;

class StGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
	explicit StGraphicsView(QWidget *parent = 0);

    bool operation_enabled();
    void enable_operation();

    void updateTracks(QPixmap srcPixmap, TrackIdx frame_id);
	void setRectgeometry(int index);
	void updateFrameId(int frameid);
signals:
	void signal_itemWidgetSelected(Object);
	void signal_Insertlistinfo(int);
    void signal_Updatelistinfo(int);
	void signal_Updatelistinfo();
	void signal_trackidselected(int, int);
public slots:
    void adjustPictureSize();

private:
    StGraphicsScene *m_pGraphicsScene;
    QPixmap m_pixmap;
    QGraphicsPixmapItem* m_pCurPixmapItem;
    qreal m_scaleValue;
};

#endif // STGRAPHICSVIEW_H
