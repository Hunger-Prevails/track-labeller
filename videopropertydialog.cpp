#include "videopropertydialog.h"
#include "ui_videopropertydialog.h"
#include <QFileInfo>
#include <QDateTime>

VideoPropertyDialog::VideoPropertyDialog(const QString& filePath, int frames, int fps,  const QString& strTimes, QSize resolution, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VideoPropertyDialog)
{
    ui->setupUi(this);

    QFileInfo fileInfo(filePath);

    ui->labelFileName->setText(fileInfo.fileName());
    ui->labelFileSize->setText(QString("%1B").arg(fileInfo.size()));
    ui->labelFileTime->setText(fileInfo.created().toString("yyyy-MM-dd hh:mm:ss"));
    ui->labelVideoFrames->setText(QString::number(frames));
    ui->labelVideoFps->setText(QString::number(fps));
    ui->labelVideoTimes->setText(strTimes);

    QString str = QString("%1 * %2").arg(resolution.width()).arg(resolution.height());
    ui->labelVideoResolution->setText(str);

    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(accept()));
}

VideoPropertyDialog::~VideoPropertyDialog()
{
    delete ui;
}
