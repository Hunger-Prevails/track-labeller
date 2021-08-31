#include "generaloptionswidget.h"
#include "ui_generaloptionswidget.h"
#include <QFileDialog>

#include "optionsdialog.h"
extern sys_option_t g_sysOptionInfo;

GeneralOptionsWidget::GeneralOptionsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GeneralOptionsWidget)
{
    ui->setupUi(this);

    ui->lineEdit->setText(g_sysOptionInfo.strDefaultFileDir);
    ui->lineEdit->setToolTip(g_sysOptionInfo.strDefaultFileDir);
    ui->check_save_tip->setChecked(g_sysOptionInfo.show_save_tips);
    ui->check_auto_save->setChecked(g_sysOptionInfo.enable_auto_save);
    ui->edit_save_interval->setText(QString::number(g_sysOptionInfo.auto_save_interval));
}

GeneralOptionsWidget::~GeneralOptionsWidget()
{
    delete ui;
}

void GeneralOptionsWidget::saveOptions()
{
    g_sysOptionInfo.strDefaultFileDir = ui->lineEdit->text();
    g_sysOptionInfo.show_save_tips = ui->check_save_tip->isChecked();
    g_sysOptionInfo.enable_auto_save = ui->check_auto_save->isChecked();
    g_sysOptionInfo.auto_save_interval = ui->edit_save_interval->text().toInt();
}

bool GeneralOptionsWidget::isOptionsChanged()
{
    bool cond1 = g_sysOptionInfo.strDefaultFileDir != ui->lineEdit->text();
    bool cond2 = g_sysOptionInfo.auto_save_interval != ui->edit_save_interval->text().toInt();
    bool cond3 = g_sysOptionInfo.enable_auto_save != ui->check_auto_save->isChecked();
    bool cond4 = g_sysOptionInfo.show_save_tips != ui->check_save_tip->isChecked();
    return cond1|cond2|cond3|cond4;
}

void GeneralOptionsWidget::on_pushButton_clicked()
{
    QString str = ui->lineEdit->text();
    if (str.isEmpty())
        str = ".";
    QString strFileName = QFileDialog::getExistingDirectory(this, "Select Directory", str, QFileDialog::ShowDirsOnly
                                                                    | QFileDialog::DontResolveSymlinks);
    if (!strFileName.isEmpty())
    {
        ui->lineEdit->setText(strFileName);
        ui->lineEdit->setToolTip(strFileName);
    }
}
