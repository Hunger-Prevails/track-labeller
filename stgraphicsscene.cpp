#include "stgraphicsscene.h"
#include "stsceneitemwidget.h"
#include "optionsdialog.h"
#include "trackInterface.h"
#include <QGraphicsSceneMouseEvent>

#include <QReadWriteLock>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QCheckBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>

extern int move_step;
extern int edge_step;
extern int lower_width_limit;
extern int lower_height_limit;
extern Frames g_frames;
extern QReadWriteLock* g_pTrackInfosLock;

bool generating;

MyMessageBox::MyMessageBox() : QMessageBox()
{
    setSizeGripEnabled(true);
}

bool MyMessageBox::event(QEvent *e)
{
    bool result = QMessageBox::event(e);

    setMinimumHeight(0);
    setMaximumHeight(QWIDGETSIZE_MAX);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // make the detailed text expanding
    QWidget *textEdit = findChild<QWidget *>("qt_msgbox_label");

    if (textEdit)
    {
        textEdit->setMinimumHeight(0);
        textEdit->setMaximumHeight(QWIDGETSIZE_MAX);
        textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    return result;
}

StGraphicsScene::StGraphicsScene(QObject *parent) :
    QGraphicsScene(parent)
{
    generating = false;
    operatable = false;
    m_scaleValue = 1;
    m_nCurFrame = -1;
    selected_trackid = -1;
    m_pSelectedItemWidget = NULL;

    m_styleMap["ped"] = std::make_tuple(
                "#frame{background-color: rgba(0,0,0,0); border: 1px solid rgba(0,0,0,0);}"
                "QLineEdit{color: black; background-color: rgba(255,255,255,120); border: 1px solid #666666;}"
                "QLabel{background-color: rgba(240,150,0,50); border: 1px solid #666666;}",
                "#frame{background-color: rgba(0,0,0,0); border: 2px solid red;}"
                "QLineEdit{color: black; background-color: rgba(255,255,255,120); border: 1px solid #666666;}"
                "QLabel{background-color: rgba(255,255,255,50); border: none;}");

    m_styleMap["car"] = std::make_tuple(
                "#frame{background-color: rgba(0,0,0,0); border: 1px solid rgba(0,0,0,0);}"
                "QLineEdit{color: black; background-color: rgba(150,240,0,120); border: 1px solid #666666;}"
                "QLabel{background-color: rgba(150,240,0,50); border: 1px solid #666666;}",
                "#frame{background-color: rgba(0,0,0,0); border: 2px solid red;}"
                "QLineEdit{color: black; background-color: rgba(255,255,255,120); border: 1px solid #666666;}"
                "QLabel{background-color: rgba(255,255,255,50); border: none;}");

    m_styleMap["bike"] = std::make_tuple(
                "#frame{background-color: rgba(0,0,0,0); border: 1px solid rgba(0,0,0,0);}"
                "QLineEdit{color: black; background-color: rgba(0,150,240,120); border: 1px solid #666666;}"
                "QLabel{background-color: rgba(0,150,240,50); border: 1px solid #666666;}",
                "#frame{background-color: rgba(0,0,0,0); border: 2px solid red;}"
                "QLineEdit{color: black; background-color: rgba(255,255,255,120); border: 1px solid #666666;}"
                "QLabel{background-color: rgba(255,255,255,50); border: none;}");

    m_styleMapTo[1] = "ped";
    m_styleMapTo[2] = "bike";
    m_styleMapTo[3] = "car";

    QMetaObject::Connection ret = connect(this, SIGNAL(signal_Insertlistinfo(int)), this->parent(), SIGNAL(signal_Insertlistinfo(int)));
    Q_ASSERT(ret);
    ret = connect(this, SIGNAL(signal_Updatelistinfo()), this->parent(), SIGNAL(signal_Updatelistinfo()));
    Q_ASSERT(ret);
    ret = connect(this->parent(), SIGNAL(signal_trackidselected(int,int)), this, SLOT(slot_trackidSelected(int, int)));
    Q_ASSERT(ret);
}

bool StGraphicsScene::operation_enabled(){
    return operatable;
}

void StGraphicsScene::enable_operation(){
    operatable = true;
}

void StGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
        return;
    if(!operatable)
        return;

    foreach (StSceneItemWidget* pWidget, m_pItemWidgetList)
    {
        if (pWidget->frameGeometry().contains(event->buttonDownScenePos(Qt::LeftButton).toPoint()))
        {
            pWidget->double_clicked();
            event->accept();
            return;
        }
    }

    QPointF curPos = event->buttonDownScenePos(Qt::LeftButton);
    Object newRect(1, 1.0, m_nCurFrame, getNewTrackId(), Rect<double>(curPos.x(), curPos.y(), 150, 150));

    trackInterface::Instance()->addObject(newRect);

    QSize size = QSize(this->width(), this->height());

    generating = true;

    StSceneItemWidget* pWidget = new StSceneItemWidget(size, newRect);

    pWidget->setItemStyle(std::get<0>(m_styleMap[m_styleMapTo[newRect.label]]), std::get<1>(m_styleMap[m_styleMapTo[newRect.label]]));

    QMetaObject::Connection ret = connect(pWidget, SIGNAL(signal_selected()), this, SLOT(slot_setSelected()));
    Q_ASSERT(ret);
    ret = connect(pWidget, SIGNAL(signal_deleted()), this, SLOT(slot_deleted()));
    Q_ASSERT(ret);
    ret = connect(pWidget, SIGNAL(signal_changeTid(int)), this->parent(), SIGNAL(signal_Updatelistinfo(int)));
    Q_ASSERT(ret);
    ret = connect(pWidget, SIGNAL(signal_itemWidgetSelected(Object)), this->parent(), SIGNAL(signal_itemWidgetSelected(Object)));
    Q_ASSERT(ret);
    ret = connect(pWidget, SIGNAL(signal_changeLabel()), this, SLOT(slot_label_changed()));
    Q_ASSERT(ret);

    pWidget->setScaleValue(m_scaleValue);
    this->addWidget(pWidget);
    pWidget->move(curPos.toPoint());
    pWidget->setMinimumSize(lower_width_limit, lower_height_limit);

    generating = false;

    m_pItemWidgetList.append(pWidget);

    emit signal_Insertlistinfo(newRect.track_id);
}

void StGraphicsScene::setScaleValue(qreal value)
{
    m_scaleValue = value;
    foreach (StSceneItemWidget* pWidget, m_pItemWidgetList)
    {
        if (pWidget != NULL)
            pWidget->setScaleValue(value);
    }
}

void StGraphicsScene::slot_setSelected()
{
    StSceneItemWidget* pWidget = qobject_cast<StSceneItemWidget*>(sender());
    if (NULL == pWidget)
        return;

    foreach (StSceneItemWidget* pTmp, m_pItemWidgetList)
    {
        if (pTmp != pWidget)
            pTmp->setSelected(false);
    }
    pWidget->setSelected(true);
    m_pSelectedItemWidget = pWidget;
}

void StGraphicsScene::slot_trackidSelected(int frameid,int trackId)
{
    foreach(StSceneItemWidget* pTmp, m_pItemWidgetList)
    {
        bool cond1 = pTmp->m_trackRectInfo.track_id == trackId;
        bool cond2 = pTmp->m_trackRectInfo.frame_id == frameid;
        if (cond1 & cond2)
        {
            pTmp->setSelected(true);
            m_pSelectedItemWidget = pTmp;
        }
        else
        {
            pTmp->setSelected(false);
        }
    }
}
void StGraphicsScene::slot_label_changed()
{
    StSceneItemWidget* pWidget = qobject_cast<StSceneItemWidget*>(sender());
    if (NULL == pWidget)
        return;

    pWidget->setItemStyle(std::get<0>(m_styleMap[m_styleMapTo[pWidget->m_trackRectInfo.label]]),
                        std::get<1>(m_styleMap[m_styleMapTo[pWidget->m_trackRectInfo.label]]));
}

void StGraphicsScene::slot_deleted()
{
    StSceneItemWidget* pWidget = qobject_cast<StSceneItemWidget*>(sender());
    if (NULL == pWidget)
        return;
    QMessageBox message;

    QLabel* pLabel = new QLabel("Please Choose your delete options");
    pLabel->setScaledContents(true);
    pLabel->setIndent(20);

    QCheckBox * checkboxCur = new QCheckBox("Current Frame", &message);
    checkboxCur->setChecked(true);

    QCheckBox * checkboxPre = new QCheckBox("Every Frame Before", &message);
    QCheckBox * checkboxFol = new QCheckBox("Every Frame After", &message);

    QPushButton * okb = message.addButton(QMessageBox::Ok);
    QPushButton * cancelb = message.addButton(QMessageBox::Cancel);

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->addWidget(pLabel,2);
    vbox->addWidget(checkboxCur,1);
    vbox->addWidget(checkboxPre,1);
    vbox->addWidget(checkboxFol,1);

    dynamic_cast<QGridLayout *>(message.layout())->addLayout(vbox, 0, 1);

    dynamic_cast<QGridLayout *>(message.layout())->addWidget(okb, 5, 3, 1, 1);

    dynamic_cast<QGridLayout *>(message.layout())->addWidget(cancelb, 5, 5, 1, 1);

    bool bcur = true;
    bool bpre = false;
    bool bfol = false;

    QObject::connect(checkboxCur, &QCheckBox::stateChanged, [&](int state){
        bcur = static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked ? true : false;
    });

    QObject::connect(checkboxPre, &QCheckBox::stateChanged, [&](int state){
        bpre = static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked ? true : false;
    });

    QObject::connect(checkboxFol, &QCheckBox::stateChanged, [&](int state){
        bfol = static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked ? true : false;
    });

    if (message.exec() != QMessageBox::Ok)
        return;

    foreach (StSceneItemWidget* pTmp, m_pItemWidgetList)
    {
        if (pTmp == pWidget)
        {
            Object rect = pTmp->m_trackRectInfo;
            if (bcur)
            {
                trackInterface::Instance()->removeObject(rect, 0);
                m_pItemWidgetList.removeOne(pTmp);
            }
            if (bpre)
                trackInterface::Instance()->removeObject(rect, 1);

            if (bfol)
                trackInterface::Instance()->removeObject(rect, 2);

            QThread::msleep(100);
            emit signal_Updatelistinfo();
            break;
        }
    }
    if (m_pSelectedItemWidget == pWidget)
        m_pSelectedItemWidget = NULL;

    if(bcur){
        pWidget->close();
        pWidget->deleteLater();
    }
}

void StGraphicsScene::clearItems()
{
    selected_trackid = -1;
    if(m_pSelectedItemWidget != NULL)
        selected_trackid = m_pSelectedItemWidget->m_trackRectInfo.track_id;

    while (m_pItemWidgetList.count() > 0)
    {
        StSceneItemWidget* pTmp = m_pItemWidgetList.takeFirst();
        if (pTmp != NULL)
        {
            pTmp->close();
            pTmp->deleteLater();
        }
    }
    m_pSelectedItemWidget = NULL;
}

void StGraphicsScene::updateTrackRects(TrackIdx frame_id)
{
    if(frame_id >= g_frames.size())
        return;
    Tracks1Frame curFrame = g_frames[frame_id];

    std::map<TrackIdx, Object>::iterator it;
    for(it = curFrame.begin(); it != curFrame.end(); ++it)
    {
        QSize size = QSize(this->width(), this->height());

        int l = (int)it->second.rect.left;
        int t = (int)it->second.rect.top;
        int w = (int)it->second.rect.width;
        int h = (int)it->second.rect.height;

        generating = true;

        StSceneItemWidget* pWidget = new StSceneItemWidget(size, it->second);

        int label = it->second.label;
        pWidget->setItemStyle(std::get<0>(m_styleMap[m_styleMapTo[label]]), std::get<1>(m_styleMap[m_styleMapTo[label]]));

        QMetaObject::Connection ret = connect(pWidget, SIGNAL(signal_selected()), this, SLOT(slot_setSelected()));
        Q_ASSERT(ret);
        ret = connect(pWidget, SIGNAL(signal_deleted()), this, SLOT(slot_deleted()));
        Q_ASSERT(ret);
        ret = connect(pWidget, SIGNAL(signal_changeTid(int)), this->parent(), SIGNAL(signal_Updatelistinfo(int)));
        Q_ASSERT(ret);
        ret = connect(pWidget, SIGNAL(signal_itemWidgetSelected(Object)), this->parent(), SIGNAL(signal_itemWidgetSelected(Object)));
        Q_ASSERT(ret);
        ret = connect(pWidget, SIGNAL(signal_changeLabel()), this, SLOT(slot_label_changed()));
        Q_ASSERT(ret);

        pWidget->setScaleValue(m_scaleValue);
        this->addWidget(pWidget);
        pWidget->setMinimumSize(lower_width_limit, lower_height_limit);
        pWidget->setGeometry(l, t, w, h);

        generating = false;

        m_pItemWidgetList.append(pWidget);
    }
    foreach (StSceneItemWidget* pTmp, m_pItemWidgetList)
    {
        if (pTmp->m_trackRectInfo.track_id == selected_trackid){
            m_pSelectedItemWidget = pTmp;
            pTmp->setSelected(true);
            break;
        }
    }
}

void StGraphicsScene::setRectGeometry(int index)
{
    if (NULL == m_pSelectedItemWidget)
        return;

    int x = m_pSelectedItemWidget->x();
    int y = m_pSelectedItemWidget->y();
    int w = m_pSelectedItemWidget->width();
    int h = m_pSelectedItemWidget->height();
    switch (index)
    {
    case INDEX_KEY_EDGE_UP:
    {
        h -= edge_step;
        if (h < 0)
            h = 0;
        m_pSelectedItemWidget->restricted = true;
        m_pSelectedItemWidget->resize(w, h);
        m_pSelectedItemWidget->restricted = false;
    }
        break;
    case INDEX_KEY_EDGE_DOWN:
    {
        h += edge_step;
        if (y + h > this->height())
            h = this->height() - y;
        m_pSelectedItemWidget->restricted = true;
        m_pSelectedItemWidget->resize(w, h);
        m_pSelectedItemWidget->restricted = false;
    }
        break;
    case INDEX_KEY_EDGE_LEFT:
    {
        w -= edge_step;
        if (w < 0)
            w = 0;
        m_pSelectedItemWidget->restricted = true;
        m_pSelectedItemWidget->resize(w, h);
        m_pSelectedItemWidget->restricted = false;
    }
        break;
    case INDEX_KEY_EDGE_RIGHT:
    {
        w += edge_step;
        if (x + w > this->width())
            w = this->width() - x;
        m_pSelectedItemWidget->restricted = true;
        m_pSelectedItemWidget->resize(w, h);
        m_pSelectedItemWidget->restricted = false;
    }
        break;
    case INDEX_KEY_MOVE_UP:
    {
        y -= move_step;
        if (y < 0)
            y = 0;
        m_pSelectedItemWidget->move(x, y);
    }
        break;
    case INDEX_KEY_MOVE_DOWN:
    {
        y += move_step;
        if (y + h > this->height())
            y = this->height() - h;
        m_pSelectedItemWidget->move(x, y);
    }
        break;
    case INDEX_KEY_MOVE_LEFT:
    {
        x -= move_step;
        if (x < 0)
            x = 0;
        m_pSelectedItemWidget->move(x, y);
    }
        break;
    case INDEX_KEY_MOVE_RIGHT:
    {
        x += move_step;
        if (x + w > this->width())
            x = this->width() - w;
        m_pSelectedItemWidget->move(x, y);
    }
        break;
    }
    m_pSelectedItemWidget->changeItemRect(x, y, w, h, true);
}
