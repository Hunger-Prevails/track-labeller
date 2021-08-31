#ifndef INPUTRECTINFODIALOG_H
#define INPUTRECTINFODIALOG_H

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
class InputRectInfoDialog;
}

class InputRectInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InputRectInfoDialog(int pre_tid, int pre_label, bool pre_activated, QWidget *parent);
    ~InputRectInfoDialog();
    int getTrackId();
    int getLabel();
    bool isActivated();

private:
    Ui::InputRectInfoDialog *ui;
    QButtonGroup *group;
};

#endif // INPUTRECTINFODIALOG_H
