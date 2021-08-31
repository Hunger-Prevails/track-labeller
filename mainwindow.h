#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QShortcut>
#include <QStackedWidget>
#include <QTimer>
#include "stslider.h"
#include "tracktypes.h"

#define COUNT_OPTIONS 18
#define AUTO_SAVE_DEFAULT 60000
#define INTERVAL_DEFAULT 4
#define WIDTH_LIMIT_DEFAULT 0
#define HEIGHT_LIMIT_DEFAULT 0
#define EDGE_STEP_DEFAULT 3
#define MOVE_STEP_DEFAULT 3

class VideoManagement;
class TrackIdListWidget;
class TargetPropertyWidget;
class carPropertyWidget;
class bikePropertyWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void slot_loadVideo(bool);
    void slot_loadLabel(bool);
    void slot_SaveLabel(bool);
    void slot_actionOptions(bool);
    void slot_actionVideoProperty(bool);
    void slot_actionAbout(bool);

    void on_hide_clicked(bool);
    void slot_timeout();
    void slot_slider_valueChanged(int);

	void slot_itemWidgetSelected(Object);
	void slot_Insertlistinfo(int);
    void slot_Updatelistinfo(int);
	void slot_Updatelistinfo();
	void slot_trackidselected(int,int);

    void on_interval_set(void);
    void on_width_limit_set(void);
    void on_height_limit_set(void);
    void on_edge_step_set(void);
    void on_move_step_set(void);

protected:
    bool is_key(int key, int key_code);
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);
    void closeEvent(QCloseEvent *e);

private:
    void load_video();
    void load_label();
    void save_label();
    void count_track(int value);

    void initOptions();
    void setShortcutInfo();
    void setDefaultOptions();
    int readOptionsXml();
    int writeOptionsXml();
	
    Ui::MainWindow *ui;

    StSlider* m_pSlider;
    QTimer *timer;

    TrackIdListWidget* m_pTrackIdListWidget;
    VideoManagement* video_manager;

    QString m_strLabelFilePath;
	QString m_strVipNameFilePath;

    bool video_loaded;
};

#endif // MAINWINDOW_H
