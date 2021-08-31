#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
class GeneralOptionsWidget;
class ShortCutOptionsWidget;

enum{
    INDEX_KEY_LOAD_VIDEO,
    INDEX_KEY_LOAD_LABEL,
    INDEX_KEY_SAVE_LABEL,
    INDEX_KEY_COUNT_TRACK,
    INDEX_KEY_NEXT_FRAME,
    INDEX_KEY_PREV_FRAME,
    INDEX_KEY_EDGE_UP,
    INDEX_KEY_EDGE_DOWN,
    INDEX_KEY_EDGE_LEFT,
    INDEX_KEY_EDGE_RIGHT,
    INDEX_KEY_MOVE_UP,
    INDEX_KEY_MOVE_DOWN,
    INDEX_KEY_MOVE_LEFT,
    INDEX_KEY_MOVE_RIGHT,
};

typedef struct sys_option_t
{
    bool show_save_tips;
    bool enable_auto_save;
    int auto_save_interval;
    QString strDefaultFileDir;
    QList<QKeySequence> keySequenceList;
}sys_option_t;

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = 0);
    ~OptionsDialog();

private slots:
    void slot_accept();
    void slot_cancel();

private:
    Ui::OptionsDialog *ui;
    GeneralOptionsWidget* m_pGeneralOptionsWidget;
    ShortCutOptionsWidget* m_pShortCutOptionsWidget;
};

#endif // OPTIONSDIALOG_H
