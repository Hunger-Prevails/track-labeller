#include "trackidlistwidget.h"
#include "ui_trackidlistwidget.h"

#include "tracktypes.h"
#include <QReadWriteLock>
#include <QMessageBox>

using namespace std;

extern map<TrackIdx, vector<FrameIdx>> MOD_STORE;
extern Frames g_frames;
extern QReadWriteLock* g_pTrackInfosLock;

TrackIdListWidget::TrackIdListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TrackIdListWidget)
{
    ui->setupUi(this);

    ui->treeWidget->setColumnWidth(INDEX_TRACKID, 85);
    ui->treeWidget->setColumnWidth(INDEX_FRAME, 95);
}

TrackIdListWidget::~TrackIdListWidget()
{
    delete ui;
}

void TrackIdListWidget::updateListInfo(int curFrame)
{
	while (ui->treeWidget->topLevelItemCount() > 0)
	{
		QTreeWidgetItem* pItem = ui->treeWidget->takeTopLevelItem(0);
		if (pItem != NULL)
			delete pItem;
	}
	m_nExitTidList.clear();
    int tid = -1;

    g_pTrackInfosLock->lockForRead();

    for (int i=0; i < (int)g_frames.size(); ++i)
    {
        Tracks1Frame& trackFrameInfo = g_frames.at(i);
        std::map<TrackIdx, Object>::iterator it;
        for(it=trackFrameInfo.begin(); it!=trackFrameInfo.end(); ++it)
        {
           tid = it->second.track_id;
           if (!m_nExitTidList.contains(tid))
           {
               QTreeWidgetItem* pNewItem = new QTreeWidgetItem();
               pNewItem->setText(INDEX_TRACKID, QString::number(tid));
               pNewItem->setText(INDEX_FRAME, QString::number(it->second.frame_id));
               pNewItem->setText(INDEX_COUNT, QString::number(MOD_STORE[tid].size()));
               ui->treeWidget->addTopLevelItem(pNewItem);

               m_nExitTidList.append(tid);
           }
        }
    }
    g_pTrackInfosLock->unlock();

    for (int j=0; j<ui->treeWidget->topLevelItemCount(); ++j)
    {
        auto pItem = ui->treeWidget->topLevelItem(j);
        if (pItem != NULL && pItem->text(INDEX_FRAME).toInt() == curFrame)
        {
            ui->treeWidget->setCurrentItem(pItem);
            break;
        }
    }
}

void TrackIdListWidget::updateListInfo(int curFrame, int input_tid)
{
    while (ui->treeWidget->topLevelItemCount() > 0)
    {
        QTreeWidgetItem* pItem = ui->treeWidget->takeTopLevelItem(0);
        if (pItem != NULL)
            delete pItem;
    }
    m_nExitTidList.clear();
    int tid = -1;

    g_pTrackInfosLock->lockForRead();

    for (int i=0; i < (int)g_frames.size(); ++i)
    {
        Tracks1Frame& trackFrameInfo = g_frames.at(i);
        std::map<TrackIdx, Object>::iterator it;
        for(it=trackFrameInfo.begin(); it!=trackFrameInfo.end(); ++it)
        {
           tid = it->second.track_id;
           if (!m_nExitTidList.contains(tid))
           {
               QTreeWidgetItem* pNewItem = new QTreeWidgetItem();
               pNewItem->setText(INDEX_TRACKID, QString::number(tid));
               pNewItem->setText(INDEX_FRAME, QString::number(it->second.frame_id));
               pNewItem->setText(INDEX_COUNT, QString::number(MOD_STORE[tid].size()));
               ui->treeWidget->addTopLevelItem(pNewItem);

               m_nExitTidList.append(tid);
           }
        }
    }
    g_pTrackInfosLock->unlock();

    for (int j=0; j<ui->treeWidget->topLevelItemCount(); ++j)
    {
        auto pItem = ui->treeWidget->topLevelItem(j);
        bool cond1 = pItem->text(INDEX_FRAME).toInt() == curFrame;
        bool cond2 = pItem->text(INDEX_TRACKID).toInt() == input_tid;
        if (cond1&cond2)
        {
            ui->treeWidget->setCurrentItem(pItem);
            break;
        }
    }
}

void TrackIdListWidget::insertListInfo(int curFrame, int id)
{
	if (m_nExitTidList.contains(id))
		return;

    QTreeWidgetItem* pNewItem;
    g_pTrackInfosLock->lockForRead();

    for (int i = 0; i < (int)g_frames.size(); ++i)
	{
        Tracks1Frame& frameinfo = g_frames.at(i);
        std::map<TrackIdx, Object>::iterator iter = frameinfo.find(id);

        if (iter != frameinfo.end())
		{
            pNewItem = new QTreeWidgetItem();
            pNewItem->setText(INDEX_TRACKID, QString::number(id));
            pNewItem->setText(INDEX_FRAME, QString::number(curFrame));
            pNewItem->setText(INDEX_COUNT, QString::number(MOD_STORE[id].size()));

            bool inserted = false;

            for (int j = 0; j < ui->treeWidget->topLevelItemCount(); ++j){

                bool cond1 = curFrame == ui->treeWidget->topLevelItem(j)->text(INDEX_FRAME).toInt();
                bool cond2 = id < ui->treeWidget->topLevelItem(j)->text(INDEX_TRACKID).toInt();
                bool cond3 = curFrame < ui->treeWidget->topLevelItem(j)->text(INDEX_FRAME).toInt();

                if(cond1&cond2){
                    ui->treeWidget->insertTopLevelItem(j, pNewItem);
                    inserted = true;
                    break;
                }
                else if(cond3){
                    ui->treeWidget->insertTopLevelItem(j, pNewItem);
                    inserted = true;
                    break;
                }
            }
            if(!inserted)
                ui->treeWidget->addTopLevelItem(pNewItem);

            m_nExitTidList.append(id);
            break;
        }
	}
	g_pTrackInfosLock->unlock();
    ui->treeWidget->setCurrentItem(pNewItem);
}

void TrackIdListWidget::on_treeWidget_itemClicked(QTreeWidgetItem *item, int)
{
    if (item != NULL)
    {
        int frameid = item->text(INDEX_FRAME).toInt();
		int trackid = item->text(INDEX_TRACKID).toInt();

		emit signal_itemDoubleClicked(frameid,trackid);
    }
}

void TrackIdListWidget::slot_itemClicked(int idx)
{
	QTreeWidgetItem* pItem = NULL;

	for (int j = 0; j < ui->treeWidget->topLevelItemCount(); ++j)
	{
		pItem = ui->treeWidget->topLevelItem(j);
		if (pItem != NULL && pItem->text(INDEX_TRACKID) == QString::number(idx))
		{
			ui->treeWidget->setCurrentItem(pItem);
			break;
		}
	}
}

