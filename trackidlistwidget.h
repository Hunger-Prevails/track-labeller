#ifndef TRACKIDLISTWIDGET_H
#define TRACKIDLISTWIDGET_H

#include <QWidget>

enum{
	INDEX_FRAME,
    INDEX_TRACKID,
    INDEX_COUNT
};

class QTreeWidgetItem;

namespace Ui {
    class TrackIdListWidget;
}

class TrackIdListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TrackIdListWidget(QWidget *parent = 0);
    ~TrackIdListWidget();
    void insertListInfo(int,int);
	void updateListInfo(int);
    void updateListInfo(int,int);

public slots:
	void slot_itemClicked(int idx);

signals:
    void signal_itemDoubleClicked(int,int);

private slots:
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

private:
    Ui::TrackIdListWidget *ui;

    QList<int> m_nExitTidList; //marks the first appearance of each label
};

#endif // TRACKIDLISTWIDGET_H
