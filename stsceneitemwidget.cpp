#include "stsceneitemwidget.h"
#include "ui_stsceneitemwidget.h"
#include "inputrectinfodialog.h"
#include "trackInterface.h"
#include <QMouseEvent>
#include <vector>
#include <iostream>
#include <QReadWriteLock>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

extern Frames g_frames;
extern QReadWriteLock* g_pTrackInfosLock;
extern std::vector<TrackIdx> activated_tracks;
extern std::vector<FrameIdx> activation_starts;
extern bool generating;

StStatusBar::StStatusBar(QWidget*parent) :QStatusBar(parent)
{

}

StStatusBar::~StStatusBar()
{

}

void StStatusBar::mouseReleaseEvent(QMouseEvent *evnet)
{ 
    evnet->accept();
    emit barRelease();
}

StSceneItemWidget::StSceneItemWidget(QSize size, Object rectInfo) :
    m_parentSize(size), m_trackRectInfo(rectInfo), ui(new Ui::StSceneItemWidget)
{
    ui->setupUi(this);
    ui->label->setText(QString::number(m_trackRectInfo.track_id));

    restricted = false;
    m_scaleValue = 1;
    m_bMoveable = false;
    m_bRightBtnClicked = false;

    m_pStatusBar = new QStatusBar(this);
    m_pStatusBar->raise();
    m_pStatusBar->setStyleSheet("background - color: rgba(0, 0, 0, 0); border - bottom: 4px solid rgba(0, 0, 0, 50); border - right: 4px solid rgba(0, 0, 0, 50); ");

    m_style = "#frame{background-color: rgba(0,0,0,0); border: 1px solid rgba(0,0,0,0);}"
            "QLineEdit{color: black; background-color: rgba(255,255,255,120); border: 1px solid #666666;}"
            "QLabel{background-color: rgba(255,255,255,50); border: 1px solid #666666;}";

    m_selectStyle = "#frame{background-color: rgba(0,0,0,0); border: 3px solid red;}"
            "QLineEdit{color: black; background-color: rgba(255,255,255,120); border: 1px solid #666666;}"
            "QLabel{background-color: rgba(255,255,255,50); border: none;}";
}

StSceneItemWidget::~StSceneItemWidget()
{
    delete m_pStatusBar;
    delete ui;
}

void StSceneItemWidget::setScaleValue(qreal value)
{
    m_scaleValue = value;
}

void StSceneItemWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void StSceneItemWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        dragPosition = this->cursor().pos();
        pressPos = frameGeometry().topLeft();
        m_bMoveable = true;
        emit signal_selected();
        event->accept();
    }
    else if (event->button() == Qt::RightButton)
    {
        m_bRightBtnClicked = true;
    }
}

void StSceneItemWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(m_bMoveable)
    {
        this->setCursor(Qt::ClosedHandCursor);
        QPoint pos = this->cursor().pos();

        int x = (pos.x() - dragPosition.x()) / m_scaleValue + pressPos.x();
        int y = (pos.y() - dragPosition.y()) / m_scaleValue + pressPos.y();

        if (x < 0)
            x = 0;
        else if (x + this->width() > m_parentSize.width())
            x = m_parentSize.width() - this->width();
        if (y < 0)
            y = 0;
        else if(y + this->height() > m_parentSize.height())
            y = m_parentSize.height() - this->height();

        move(x, y);
        event->accept();
    }
}

void StSceneItemWidget::mouseReleaseEvent(QMouseEvent *event)
{
    this->setCursor(Qt::ArrowCursor);

    if (m_bRightBtnClicked)
    {
        m_bRightBtnClicked = false;
        emit signal_deleted();
        event->accept();
        return;
    }
    if (m_bMoveable)
    {
        m_bMoveable = false;

        changeItemRect(x(), y(), width(), height(), true);
        qDebug("TrackRect Dragged");

        emit signal_itemWidgetSelected(m_trackRectInfo);
    }
    event->accept();
}

void StSceneItemWidget::leaveEvent(QEvent *)
{
    this->setCursor(Qt::ArrowCursor);
}

void StSceneItemWidget::setSelected(bool bSelected)
{
    if (bSelected)
    {
        ui->frame->setStyleSheet(m_selectStyle);
        m_pStatusBar->setStyleSheet("background-color: rgba(0,0,0,0); border-bottom: 4px solid yellow; border-right: 4px solid yellow;");

        emit signal_itemWidgetSelected(m_trackRectInfo);
    }
    else
    {
        ui->frame->setStyleSheet(m_style);

        m_pStatusBar->setStyleSheet("background-color: rgba(0, 0, 0, 0); border-bottom: 4px solid rgba(0, 0, 0, 50); border - right: 4px solid rgba(0, 0, 0, 50); ");
    }
}

void StSceneItemWidget::setItemStyle(const QString style, const QString selStyle)
{
    m_style = style;
    m_selectStyle = selStyle;
    ui->frame->setStyleSheet(m_style);
}

void StSceneItemWidget::double_clicked()
{
    int pre_tid = m_trackRectInfo.track_id;
    int pre_label = m_trackRectInfo.label;
    bool pre_activated = std::find(activated_tracks.begin(), activated_tracks.end(), pre_tid) != activated_tracks.end();

    InputRectInfoDialog dlg(pre_tid, pre_label, pre_activated, qApp->activeWindow());
    int ret = dlg.exec();
    if (ret != QDialog::Accepted)
        return;

    int input_tid = dlg.getTrackId();
    int input_label = dlg.getLabel();
    bool is_activated = dlg.isActivated();

    if (input_tid != pre_tid)
    {
        if (!changeTrackId(m_trackRectInfo, input_tid, g_frames))
        {
            QMessageBox::warning(qApp->activeWindow(), ("Warning"), ("changeTrackId failed !"), ("OK"));
            return;
        }
        m_trackRectInfo.track_id = input_tid;
        ui->label->setText(QString::number(input_tid));
        emit signal_changeTid(input_tid);
    }
    if (input_label != pre_label){

        changeLabel(m_trackRectInfo, input_label, g_frames);
        m_trackRectInfo.label = input_label;
        emit signal_changeLabel();
    }
    if (is_activated&!pre_activated){
        activated_tracks.push_back(input_tid);
        activation_starts.push_back(m_trackRectInfo.frame_id);
    }
    if (!is_activated&pre_activated){
        std::vector<TrackIdx>::iterator it;
        it = std::find(activated_tracks.begin(), activated_tracks.end(), input_tid);
        int erase_index = it - activated_tracks.begin();

        activated_tracks.erase(it);
        activation_starts.erase(activation_starts.begin() + erase_index);
    }
}

void StSceneItemWidget::changeItemRect(int x, int y, int w, int h, bool interpolates)
{
    Rect<double> newRect = Rect<double>(double(x), double(y), double(w), double(h));

    trackInterface::Instance()->modifyRect(m_trackRectInfo, newRect, interpolates);
    qDebug("TrackRect Modified");

    m_trackRectInfo.rect = newRect;
}

void StSceneItemWidget::resizeEvent(QResizeEvent *){
    ui->frame->setGeometry(1, 1, this->width() - 2, this->height() - 2);
    m_pStatusBar->setGeometry(this->width() - 25, this->height() - 25, 25, 25);

    if(restricted)
        return;
    if(generating)
        return;
    changeItemRect(x(), y(), width(), height(), false);
}
