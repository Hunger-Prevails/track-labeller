#ifndef GENERALOPTIONSWIDGET_H
#define GENERALOPTIONSWIDGET_H

#include <QWidget>

namespace Ui {
class GeneralOptionsWidget;
}

class GeneralOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralOptionsWidget(QWidget *parent = 0);
    ~GeneralOptionsWidget();
    void saveOptions();
    bool isOptionsChanged();

private slots:
    void on_pushButton_clicked();

private:
    Ui::GeneralOptionsWidget *ui;
};

#endif // GENERALOPTIONSWIDGET_H
