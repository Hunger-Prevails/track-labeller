#ifndef SHORTCUTOPTIONSWIDGET_H
#define SHORTCUTOPTIONSWIDGET_H

#include <QWidget>
#include <QKeySequenceEdit>

namespace Ui {
class ShortCutOptionsWidget;
}

class ShortCutOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ShortCutOptionsWidget(QWidget *parent = 0);
    ~ShortCutOptionsWidget();
    void saveOptions();
    bool isOptionsChanged();

private slots:
    void slot_editFinished();

private:
    Ui::ShortCutOptionsWidget *ui;

    QList<QKeySequenceEdit*> m_pKeySequenceEditList;
};

#endif // SHORTCUTOPTIONSWIDGET_H
