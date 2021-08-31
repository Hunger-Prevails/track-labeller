#ifndef STSCENEITEMWIDGET_H
#define STSCENEITEMWIDGET_H

#include <QWidget>
#include <QStatusBar>
#include "tracktypes.h"

namespace Ui {
class StSceneItemWidget;
}

class StStatusBar : public QStatusBar
{
	Q_OBJECT
public:
	StStatusBar(QWidget *parent =0);
	~StStatusBar();
protected:
	void mouseReleaseEvent(QMouseEvent *);
signals:
	void barRelease();
private:

};

class StSceneItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StSceneItemWidget(QSize size, Object rectInfo);
    ~StSceneItemWidget();
    void setScaleValue(qreal value);
    void setSelected(bool bSelected);
	void setItemStyle(const QString style, const QString selStyle);
    void double_clicked();
    void changeItemRect(int x, int y, int w, int h, bool move);

    Object m_trackRectInfo;
    bool restricted;
signals:
    void signal_selected();
    void signal_deleted();
    void signal_changeTid(int);
    void signal_changeLabel();
	void signal_itemWidgetSelected(Object);

protected:
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
	void dragLeaveEvent(QDragLeaveEvent *);
    void leaveEvent(QEvent *);

private:
    Ui::StSceneItemWidget *ui;
    QStatusBar* m_pStatusBar;
    QPoint dragPosition;
    QPoint pressPos;
    bool    m_bMoveable;
    qreal   m_scaleValue;
    QSize   m_parentSize;
    bool    m_bRightBtnClicked;
	QString m_style;
	QString m_selectStyle;
};

#endif // STSCENEITEMWIDGET_H
