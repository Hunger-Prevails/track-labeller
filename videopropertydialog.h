#ifndef VIDEOPROPERTYDIALOG_H
#define VIDEOPROPERTYDIALOG_H

#include <QDialog>

namespace Ui {
class VideoPropertyDialog;
}

class VideoPropertyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoPropertyDialog(const QString& filePath, int frames, int fps, const QString& strTimes, QSize resolution, QWidget *parent = 0);
    ~VideoPropertyDialog();

private:
    Ui::VideoPropertyDialog *ui;
};

#endif // VIDEOPROPERTYDIALOG_H
