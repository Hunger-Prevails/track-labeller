#include "mainwindow.h"
#include "videomanagement.h"
#include "ui_mainwindow.h"
#include "optionsdialog.h"
#include "trackidlistwidget.h"
#include "ui_aboutdialog.h"
#include "questiondialog.h"
#include "trackInterface.h"
#include "trackio.h"
#include "trackmanage.h"

#include <QKeyEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QtXml/QDomElement>
#include <QTextStream>
#include <QToolTip>
#include <QDebug>
#include <QTime>
#include <QReadWriteLock>
#include <QScrollArea>

int frame_interval = INTERVAL_DEFAULT;
int lower_width_limit = WIDTH_LIMIT_DEFAULT;
int lower_height_limit = HEIGHT_LIMIT_DEFAULT;
int edge_step = EDGE_STEP_DEFAULT;
int move_step = MOVE_STEP_DEFAULT;

Frames g_frames;
QReadWriteLock* g_pTrackInfosLock;
sys_option_t g_sysOptionInfo;
bool control_pressed;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    control_pressed = false;
    video_loaded = false;

    g_pTrackInfosLock = new QReadWriteLock();
    ui->setupUi(this);

    m_pSlider = new StSlider(Qt::Horizontal, this);
    m_pSlider->setStyleSheet("QSlider::sub-page:horizontal {background-color: #5a8798;"
                             "border: 1px solid #4A708B; height: 8px; border-radius: 2px;}"
                             "QSlider::add-page:horizontal {background-color: #2A2A2A;"
                             "border-bottom: 1px solid #313131; height: 6px; border-radius: 1px;}"
                             "QSlider::handle:horizontal{background-color: #51D1FF;"
                             "width: 15px;height:23px;border-radius: 2px;}");

    connect(m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_slider_valueChanged(int)));
    m_pSlider->setEnabled(false);
    ui->horizontalLayoutSlider->insertWidget(0, m_pSlider);

    video_manager = new VideoManagement(ui->widgetVideo, ui->labelCurFrame, ui->labelCurTime, this);

    QMetaObject::Connection ret;
    ret = connect(ui->widgetVideo, SIGNAL(signal_itemWidgetSelected(Object)), this, SLOT(slot_itemWidgetSelected(Object)));
    Q_ASSERT(ret);
    ret = connect(ui->widgetVideo, SIGNAL(signal_Insertlistinfo(int)), this, SLOT(slot_Insertlistinfo(int)));
    Q_ASSERT(ret);
    ret = connect(ui->widgetVideo, SIGNAL(signal_Updatelistinfo(int)), this, SLOT(slot_Updatelistinfo(int)));
    Q_ASSERT(ret);
    ret = connect(ui->widgetVideo, SIGNAL(signal_Updatelistinfo()), this, SLOT(slot_Updatelistinfo()));
    Q_ASSERT(ret);

    connect(ui->actionLoad_Video, SIGNAL(triggered(bool)), this, SLOT(slot_loadVideo(bool)));
    connect(ui->actionLoad_Label, SIGNAL(triggered(bool)), this, SLOT(slot_loadLabel(bool)));
    connect(ui->actionSave_Label, SIGNAL(triggered(bool)), this, SLOT(slot_SaveLabel(bool)));

    connect(ui->actionOptions, SIGNAL(triggered(bool)), this, SLOT(slot_actionOptions(bool)));
    connect(ui->actionVideo_Property, SIGNAL(triggered(bool)), this, SLOT(slot_actionVideoProperty(bool)));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(slot_actionAbout(bool)));

    ui->actionLoad_Label->setEnabled(false);
    ui->actionSave_Label->setEnabled(false);
    ui->actionVideo_Property->setEnabled(false);

    m_pTrackIdListWidget = new TrackIdListWidget(this);
    ui->tabWidget->addTab(m_pTrackIdListWidget, "Track_id List");

    ret = connect(m_pTrackIdListWidget, SIGNAL(signal_itemDoubleClicked(int,int)), this, SLOT(slot_trackidselected(int,int)));
    Q_ASSERT(ret);
    ret = connect(m_pTrackIdListWidget, SIGNAL(signal_itemDoubleClicked(int,int)), ui->widgetVideo, SIGNAL(signal_trackidselected(int, int)));
    Q_ASSERT(ret);

    ui->edit_edge->setText(QString::number(edge_step));
    ui->edit_move->setText(QString::number(move_step));
    ui->edit_width->setText(QString::number(lower_width_limit));
    ui->edit_height->setText(QString::number(lower_height_limit));
    ui->edit_interval->setText(QString::number(frame_interval));

    connect(ui->button_hide, SIGNAL(clicked(bool)), this, SLOT(on_hide_clicked(bool)));
    connect(ui->edit_interval, SIGNAL(editingFinished()), this, SLOT(on_interval_set()));
    connect(ui->edit_width, SIGNAL(editingFinished()), this, SLOT(on_width_limit_set()));
    connect(ui->edit_height, SIGNAL(editingFinished()), this, SLOT(on_height_limit_set()));
    connect(ui->edit_move, SIGNAL(editingFinished()), this, SLOT(on_move_step_set()));
    connect(ui->edit_edge, SIGNAL(editingFinished()), this, SLOT(on_edge_step_set()));

    trackInterface::Instance();
    initOptions();
    ui->statusBar->showMessage("Please Load Files.");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    ui->widgetVideo->adjustPictureSize();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    int ret = QMessageBox::question(this, tr("Notice"), tr("Exit?"), tr("Yes"), tr("No"));
    if (0 != ret)
    {
        e->ignore();
        return;
    }
    e->accept();
}

bool MainWindow::is_key(int key, int key_code){
    return g_sysOptionInfo.keySequenceList.at(key_code) == QKeySequence(key);
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Control){
        control_pressed = true;
        return;
    }
    if(control_pressed){
        if(is_key(ev->key(), INDEX_KEY_LOAD_VIDEO))
            load_video();
        else if(is_key(ev->key(), INDEX_KEY_LOAD_LABEL))
            load_label();
        else if(is_key(ev->key(), INDEX_KEY_SAVE_LABEL))
            save_label();
        else if(is_key(ev->key(), INDEX_KEY_COUNT_TRACK))
            count_track(m_pSlider->value());
    }
    if (!ui->widgetVideo->operation_enabled())
        return;

    if(is_key(ev->key(), INDEX_KEY_NEXT_FRAME)){

        int step = m_pSlider->singleStep();
        int value = m_pSlider->value();
        value += step * frame_interval;

        bool cond1 = value < video_manager->m_nFrames;
        bool cond2 = value >= 0;

        if(cond1 & cond2){
            addActivated(value, g_frames, true);
            m_pSlider->setValue(value);
        }
        else {
            qDebug()<<"Out of Margin.";
        }
    }
    if(is_key(ev->key(), INDEX_KEY_PREV_FRAME)){

        int step = m_pSlider->singleStep();
        int value = m_pSlider->value();
        value -= step * frame_interval;

        bool cond1 = value < video_manager->m_nFrames;
        bool cond2 = value >= 0;

        if(cond1 & cond2){
            addActivated(value, g_frames, false);
            m_pSlider->setValue(value);
        }
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Control){
        control_pressed = false;
        return;
    }

    if (!ui->widgetVideo->operation_enabled())
        return;

    if(is_key(ev->key(), INDEX_KEY_MOVE_UP))
        ui->widgetVideo->setRectgeometry(INDEX_KEY_MOVE_UP);

    else if(is_key(ev->key(), INDEX_KEY_MOVE_DOWN))
        ui->widgetVideo->setRectgeometry(INDEX_KEY_MOVE_DOWN);

    else if(is_key(ev->key(), INDEX_KEY_MOVE_LEFT))
        ui->widgetVideo->setRectgeometry(INDEX_KEY_MOVE_LEFT);

    else if(is_key(ev->key(), INDEX_KEY_MOVE_RIGHT))
        ui->widgetVideo->setRectgeometry(INDEX_KEY_MOVE_RIGHT);

    else if(is_key(ev->key(), INDEX_KEY_EDGE_UP))
        ui->widgetVideo->setRectgeometry(INDEX_KEY_EDGE_UP);

    else if(is_key(ev->key(), INDEX_KEY_EDGE_DOWN))
        ui->widgetVideo->setRectgeometry(INDEX_KEY_EDGE_DOWN);

    else if(is_key(ev->key(), INDEX_KEY_EDGE_LEFT))
        ui->widgetVideo->setRectgeometry(INDEX_KEY_EDGE_LEFT);

    else if(is_key(ev->key(), INDEX_KEY_EDGE_RIGHT))
        ui->widgetVideo->setRectgeometry(INDEX_KEY_EDGE_RIGHT);
}

void MainWindow::load_video()
{
    if(ui->widgetVideo->operation_enabled()){
        ui->statusBar->showMessage("Reload Declined.");
        return;
    }
    QString str = g_sysOptionInfo.strDefaultFileDir;
    QString strVideoFile = QFileDialog::getOpenFileName(this, "Select Files", str, "Video File (*.avi);;All Files (*.*)");

    if (!strVideoFile.isEmpty())
    {
        if (!video_manager->setFileName(strVideoFile))
        {
            QMessageBox::warning(this, tr("Warning"), tr("Invalid Video File."), tr("OK"));
            return;
        }
        int msecs = video_manager->m_nFrames * 1000 / video_manager->m_nFps;
        QTime totalTime = QTime::fromMSecsSinceStartOfDay(msecs);
        QString strTime = totalTime.toString("mm:ss");
        if (totalTime.hour() > 0)
            strTime = totalTime.toString("hh:mm:ss");

        m_pSlider->setMinimum(0);
        m_pSlider->setMaximum(video_manager->m_nFrames);
        m_pSlider->setValue(0);

        ui->labelTotalFrame->setText(QString("%1").arg(video_manager->m_nFrames));
        ui->labelTotalTime->setText(strTime);
        ui->labelCurFrame->setText("0");
        ui->labelCurTime->setText("00:00");

        m_pSlider->setEnabled(true);
        ui->actionLoad_Video->setEnabled(false);
        ui->actionLoad_Label->setEnabled(true);
        ui->actionVideo_Property->setEnabled(true);

        video_manager->setCurFrame(m_pSlider->value());
        video_loaded = true;

        ui->statusBar->showMessage("Video Loaded.", 1000);
        qDebug("Video File Loaded");
    }
}

void MainWindow::load_label()
{
    if(ui->widgetVideo->operation_enabled()){
        ui->statusBar->showMessage("Reload Declined.");
        return;
    }
    if(!video_loaded){
        ui->statusBar->showMessage("Video Load Required.");
        return;
    }
    QString str = g_sysOptionInfo.strDefaultFileDir;
    QString strLabelFile = QFileDialog::getOpenFileName(this, "Select Files", str, "Label File (*.txt);;All Files (*.*)");

    if (!strLabelFile.isEmpty())
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        g_frames.clear();
        for(int i=0; i < video_manager->m_nFrames; i++)
            g_frames.push_back(Tracks1Frame());

        QByteArray byteArray = strLabelFile.toLocal8Bit();
        const char* strName = byteArray.data();

        if (!loadLabels(std::string(strName), g_frames))
        {
            QMessageBox::warning(this, ("Warning"), ("Labelfile Load Error!"), ("OK"));
            return;
        }
        m_strLabelFilePath = strLabelFile;
        m_pTrackIdListWidget->updateListInfo(m_pSlider->value());

        ui->actionSave_Label->setEnabled(true);
        ui->actionLoad_Label->setEnabled(false);

        video_manager->setCurFrame(m_pSlider->value());

        QApplication::restoreOverrideCursor();
        timer = new QTimer();
        connect(timer, SIGNAL(timeout()), this, SLOT(slot_timeout()));
        timer->start(g_sysOptionInfo.auto_save_interval);

        ui->widgetVideo->enable_operation();

        ui->statusBar->showMessage("Labels Loaded.", 1000);
        qDebug("Labels Loaded.");
    }
}

void MainWindow::save_label()
{
    if(!ui->widgetVideo->operation_enabled()){
        ui->statusBar->showMessage("Label Load Required.");
        return;
    }
    int option = 0;
    if(g_sysOptionInfo.show_save_tips)
        option = QMessageBox::question(this, tr("Prompt"), tr("Choose your Save Options"), tr("Save and Overwrite"), tr("Save as"), tr("Cancel"));

    if (0 == option)
    {
        QByteArray byteArray = m_strLabelFilePath.toLocal8Bit();
        const char* strName = byteArray.data();

        if (!saveLabels(std::string(strName), g_frames))
        {
            QMessageBox::warning(this, ("Warning"), ("SaveLabels Failed !"), ("OK"));
            return;
        }
        ui->statusBar->showMessage("Labels Overwritten.", 3000);
        qDebug("Labels Overwritten.");
    }

    else if (1 == option)
    {
        QString strFileName = QFileDialog::getSaveFileName(this, "Select Directory", m_strLabelFilePath, "File (*.txt);;All Files (*.*)");
        if (!strFileName.isEmpty())
        {
            QByteArray byteArray = strFileName.toLocal8Bit();
            const char* strName = byteArray.data();

            if (!saveLabels(std::string(strName), g_frames))
            {
                QMessageBox::warning(this, ("Warning"), ("Save Label file failed !"), ("OK"));
                return;
            }
            m_strLabelFilePath = strFileName;
            ui->statusBar->showMessage(QString("Labels Saved."));
        }
        qDebug("Labels Saved");
    }
    m_pTrackIdListWidget->updateListInfo(m_pSlider->value());
}

void MainWindow::count_track(int value){
    if(!ui->widgetVideo->operation_enabled()){
        ui->statusBar->showMessage("Label Load Required.");
        return;
    }
    int sum_tracks = 0;
    for(int i = 0; i <= value; i++)
        sum_tracks += g_frames[i].size();
    ui->statusBar->showMessage(QString("Count Tracks: ") + QString::number(sum_tracks));
}

void MainWindow::slot_loadVideo(bool){
    load_video();
}

void MainWindow::slot_loadLabel(bool){
    load_label();
}

void MainWindow::slot_SaveLabel(bool){
    save_label();
}

void MainWindow::slot_slider_valueChanged(int value)
{
    if (ui->widgetVideo->operation_enabled())
    {
        video_manager->setCurFrame(value);
    }
}

void MainWindow::slot_actionOptions(bool)
{
    OptionsDialog dlg(this);
    int ret = dlg.exec();
    if (ret == QDialog::Accepted)
    {
        if (writeOptionsXml() < 0)
        {
            QMessageBox::warning(this, ("Warning"), ("saveLabels failed !"), ("OK"));
            return;
        }
        ui->statusBar->showMessage(QString("Options Saved."), 3000);
        qDebug()<<"Options Saved.";
    }
}

void MainWindow::slot_actionVideoProperty(bool)
{
    video_manager->showVideoProperty();
}

void MainWindow::slot_actionAbout(bool)
{
    QDialog *pAboutDlg = new QDialog(this);
    Ui::AboutDialog pUi;
    pUi.setupUi(pAboutDlg);
    connect(pUi.pushButton, SIGNAL(clicked()), pAboutDlg, SLOT(deleteLater()));
    pAboutDlg->exec();
}

void MainWindow::initOptions()
{
    if (readOptionsXml() < 0){
        qDebug()<<"Failed to Read Options, Using Default Options.";
        setDefaultOptions();
    }
}

void MainWindow::setDefaultOptions()
{
    g_sysOptionInfo.keySequenceList.clear();
    g_sysOptionInfo.show_save_tips = true;
    g_sysOptionInfo.enable_auto_save = true;
    g_sysOptionInfo.auto_save_interval = AUTO_SAVE_DEFAULT;
    g_sysOptionInfo.strDefaultFileDir = QDir(".").absolutePath();

    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_O));
    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_L));
    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_X));
    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_C));
    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_E));
    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_Q));
    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_W));
    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_S));
    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_A));
    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_D));
    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_Up));
    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_Down));
    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_Left));
    g_sysOptionInfo.keySequenceList.append(QKeySequence(Qt::Key_Right));

    writeOptionsXml();
}

int MainWindow::readOptionsXml()
{
    int ret = -1;
    QDomDocument doc;
    QString strPath = QApplication::applicationDirPath().append("/configure/configParam.xml");
    QString strFileName = QDir::toNativeSeparators(strPath);

    QFile systemFile(strFileName);
    if (!systemFile.open(QIODevice::ReadOnly))
    {
        return ret;
    }
    if (!doc.setContent(&systemFile))
    {
        systemFile.close();
        return ret;
    }
    QDomElement root = doc.documentElement();
    QDomElement child = root.firstChildElement();
    QDomNode root_c = root.firstChild();
    QDomNode child_c = child.firstChild();
    QDomElement root_e;
    QDomElement child_e;

    int nSuccessCount = 0;

    g_sysOptionInfo.keySequenceList.clear();

    while (!root_c.isNull())
    {
        root_e = root_c.toElement();
        if (!root_e.isNull())
        {
            if (root_e.tagName() == "configParam")
            {
                while (!child_c.isNull())
                {
                    child_e = child_c.toElement();
                    if (!child_e.isNull())
                    {
                        QString strText = child_e.text();
                        if (child_e.tagName() == "strDefaultFileDir")
                        {
                            g_sysOptionInfo.strDefaultFileDir = strText;
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "bShowSaveTips")
                        {
                            g_sysOptionInfo.show_save_tips = (1 == strText.toInt());
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "bEnableAutoSave"){
                            g_sysOptionInfo.enable_auto_save = (1 == strText.toInt());
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "bAutoSaveInterval"){
                            g_sysOptionInfo.auto_save_interval = strText.toInt();
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_LoadVideo")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_LoadLabel")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_SaveLabel")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_CountTrack")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_NextFrame")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_PrevFrame")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_EdgeUp")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_EdgeDown")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_EdgeLeft")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_EdgeRight")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_MoveUp")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_MoveDown")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_MoveLeft")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                        else if (child_e.tagName() == "keySequence_MoveRight")
                        {
                            g_sysOptionInfo.keySequenceList.append(QKeySequence(strText));
                            nSuccessCount++;
                        }
                    }
                    child_c = child_c.nextSibling();
                }
            }
        }
        child = child.nextSiblingElement();
        child_c = child.firstChild();
        root_c = root_c.nextSibling();
    }
    systemFile.close();

    if (nSuccessCount == COUNT_OPTIONS){
        ret = 0;
        qDebug()<<"Options Read";
    }
    return ret;
}

int MainWindow::writeOptionsXml()
{
    QString strPath = QApplication::applicationDirPath().append("/configure");

    QDir dir(QDir::toNativeSeparators(strPath));
    if (!dir.exists())
    {
        dir.mkdir(QDir::toNativeSeparators(strPath));
    }

    QString strFileName = QDir::toNativeSeparators(strPath.append("/configParam.xml"));

    QFile systemFile(strFileName);
    if (!systemFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qCritical("Open configure file failed.");
        return -1;
    }

    QDomDocument doc;
    QDomText text;
    QString strTemp;
    QDomElement configParam, strDefaultFileDir;
    QDomElement bShowSaveTips, bEnableAutoSave, bAutoSaveInterval;
    QDomElement keySequence_LoadVideo, keySequence_LoadLabel;
    QDomElement keySequence_SaveLabel, keySequence_CountTrack;
    QDomElement keySequence_NextFrame, keySequence_PrevFrame;
    QDomElement keySequence_EdgeUp, keySequence_EdgeDown, keySequence_EdgeLeft, keySequence_EdgeRight;
    QDomElement keySequence_MoveUp, keySequence_MoveDown, keySequence_MoveLeft, keySequence_MoveRight;
    QDomElement senetime = doc.createElement("senetime");
    doc.appendChild(senetime);

    configParam = doc.createElement("configParam");
    senetime.appendChild(configParam);

    strDefaultFileDir = doc.createElement("strDefaultFileDir");
    text = doc.createTextNode(g_sysOptionInfo.strDefaultFileDir);
    strDefaultFileDir.appendChild(text);
    configParam.appendChild(strDefaultFileDir);

    bShowSaveTips = doc.createElement("bShowSaveTips");
    strTemp = QString::number(int(g_sysOptionInfo.show_save_tips));
    text = doc.createTextNode(strTemp);
    bShowSaveTips.appendChild(text);
    configParam.appendChild(bShowSaveTips);

    bEnableAutoSave = doc.createElement("bEnableAutoSave");
    strTemp = QString::number(int(g_sysOptionInfo.enable_auto_save));
    text = doc.createTextNode(strTemp);
    bEnableAutoSave.appendChild(text);
    configParam.appendChild(bEnableAutoSave);

    bAutoSaveInterval = doc.createElement("bAutoSaveInterval");
    strTemp = QString::number(g_sysOptionInfo.auto_save_interval);
    text = doc.createTextNode(strTemp);
    bAutoSaveInterval.appendChild(text);
    configParam.appendChild(bAutoSaveInterval);

    keySequence_LoadVideo = doc.createElement("keySequence_LoadVideo");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_LOAD_VIDEO).toString());
    keySequence_LoadVideo.appendChild(text);
    configParam.appendChild(keySequence_LoadVideo);

    keySequence_LoadLabel = doc.createElement("keySequence_LoadLabel");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_LOAD_LABEL).toString());
    keySequence_LoadLabel.appendChild(text);
    configParam.appendChild(keySequence_LoadLabel);

    keySequence_SaveLabel = doc.createElement("keySequence_SaveLabel");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_SAVE_LABEL).toString());
    keySequence_SaveLabel.appendChild(text);
    configParam.appendChild(keySequence_SaveLabel);

    keySequence_CountTrack = doc.createElement("keySequence_CountTrack");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_COUNT_TRACK).toString());
    keySequence_CountTrack.appendChild(text);
    configParam.appendChild(keySequence_CountTrack);

    keySequence_NextFrame = doc.createElement("keySequence_NextFrame");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_NEXT_FRAME).toString());
    keySequence_NextFrame.appendChild(text);
    configParam.appendChild(keySequence_NextFrame);

    keySequence_PrevFrame = doc.createElement("keySequence_PrevFrame");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_PREV_FRAME).toString());
    keySequence_PrevFrame.appendChild(text);
    configParam.appendChild(keySequence_PrevFrame);

    keySequence_EdgeUp = doc.createElement("keySequence_EdgeUp");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_EDGE_UP).toString());
    keySequence_EdgeUp.appendChild(text);
    configParam.appendChild(keySequence_EdgeUp);

    keySequence_EdgeDown = doc.createElement("keySequence_EdgeDown");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_EDGE_DOWN).toString());
    keySequence_EdgeDown.appendChild(text);
    configParam.appendChild(keySequence_EdgeDown);

    keySequence_EdgeLeft = doc.createElement("keySequence_EdgeLeft");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_EDGE_LEFT).toString());
    keySequence_EdgeLeft.appendChild(text);
    configParam.appendChild(keySequence_EdgeLeft);

    keySequence_EdgeRight = doc.createElement("keySequence_EdgeRight");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_EDGE_RIGHT).toString());
    keySequence_EdgeRight.appendChild(text);
    configParam.appendChild(keySequence_EdgeRight);

    keySequence_MoveUp = doc.createElement("keySequence_MoveUp");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_MOVE_UP).toString());
    keySequence_MoveUp.appendChild(text);
    configParam.appendChild(keySequence_MoveUp);

    keySequence_MoveDown = doc.createElement("keySequence_MoveDown");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_MOVE_DOWN).toString());
    keySequence_MoveDown.appendChild(text);
    configParam.appendChild(keySequence_MoveDown);

    keySequence_MoveLeft = doc.createElement("keySequence_MoveLeft");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_MOVE_LEFT).toString());
    keySequence_MoveLeft.appendChild(text);
    configParam.appendChild(keySequence_MoveLeft);

    keySequence_MoveRight = doc.createElement("keySequence_MoveRight");
    text = doc.createTextNode(g_sysOptionInfo.keySequenceList.at(INDEX_KEY_MOVE_RIGHT).toString());
    keySequence_MoveRight.appendChild(text);
    configParam.appendChild(keySequence_MoveRight);

    QTextStream out(&systemFile);
    doc.save(out, 4);
    systemFile.close();

    qDebug()<<"Tool options saved.";
    return 0;
}

void MainWindow::slot_Insertlistinfo(int id)
{
    m_pTrackIdListWidget->insertListInfo(m_pSlider->value(),id);
}

void MainWindow::slot_Updatelistinfo(int input_tid)
{
    m_pTrackIdListWidget->updateListInfo(m_pSlider->value(), input_tid);
}

void MainWindow::slot_Updatelistinfo()
{
    m_pTrackIdListWidget->updateListInfo(m_pSlider->value());
}

void MainWindow::slot_itemWidgetSelected(Object obj)
{
    m_pTrackIdListWidget->slot_itemClicked(obj.track_id);
}

void MainWindow::slot_trackidselected(int frameid,int trackid)
{
    if (m_pSlider)
    {
        m_pSlider->setValue(frameid);
    }
}

void MainWindow::on_interval_set(void){
    ui->edit_interval->clearFocus();
    frame_interval=ui->edit_interval->text().toInt();
}

void MainWindow::on_width_limit_set(void){
    ui->edit_width->clearFocus();
    int newval = ui->edit_width->text().toInt();
    if(newval != lower_width_limit){
        lower_width_limit = newval;
        video_manager->setCurFrame(m_pSlider->value());
    }
}

void MainWindow::on_height_limit_set(void){
    ui->edit_height->clearFocus();
    int newval = ui->edit_height->text().toInt();
    if(newval != lower_height_limit){
        lower_height_limit = newval;
        video_manager->setCurFrame(m_pSlider->value());
    }
}

void MainWindow::on_move_step_set(){
    ui->edit_move->clearFocus();
    move_step=ui->edit_move->text().toInt();
}

void MainWindow::on_edge_step_set(){
    ui->edit_edge->clearFocus();
    edge_step=ui->edit_edge->text().toInt();
}

void MainWindow::on_hide_clicked(bool){

    if (ui->tabWidget->isHidden())
    {
        ui->button_hide->setText(">");
        ui->tabWidget->show();
    }
    else
    {
        ui->button_hide->setText("<");
        ui->tabWidget->hide();
    }
    QTimer::singleShot(10, ui->widgetVideo, SLOT(adjustPictureSize()));
}

void MainWindow::slot_timeout(){
    save_label();
    timer->start(g_sysOptionInfo.auto_save_interval);
}
