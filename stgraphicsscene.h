#ifndef STGRAPHICSSCENE_H
#define STGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QMap>
#include <QMessageBox>
#include <tuple>
#include "tracktypes.h"

class StSceneItemWidget;

class MyMessageBox : public QMessageBox
{
public:
	MyMessageBox();

protected:
	bool event(QEvent *e);
};

class StGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit StGraphicsScene(QObject *parent = 0);
    bool operation_enabled();
    void enable_operation();

    void setScaleValue(qreal value);
    void updateTrackRects(TrackIdx frame_id);
    void clearItems();
    void setRectGeometry(int index);
    void updateFrameId(int frameid){m_nCurFrame = frameid;}
signals:
    void signal_Insertlistinfo(int);
	void signal_Updatelistinfo();
private slots:
    void slot_setSelected();
	void slot_trackidSelected(int,int);
    void slot_deleted();
    void slot_label_changed();
	
protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
    bool operatable;
    int selected_trackid;
    int m_nCurFrame;
    qreal m_scaleValue;

    QList<StSceneItemWidget*> m_pItemWidgetList;
    StSceneItemWidget* m_pSelectedItemWidget;

	QMap<QString, std::tuple<QString ,QString> > m_styleMap;
	QMap<int, QString> m_styleMapTo;
};
#endif // STGRAPHICSSCENE_H
