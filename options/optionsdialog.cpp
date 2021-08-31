#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "generaloptionswidget.h"
#include "shortcutoptionswidget.h"
#include <QMessageBox>

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    m_pGeneralOptionsWidget = new GeneralOptionsWidget(this);
    m_pShortCutOptionsWidget = new ShortCutOptionsWidget(this);
    ui->stackedWidget->addWidget(m_pGeneralOptionsWidget);
    ui->stackedWidget->addWidget(m_pShortCutOptionsWidget);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(slot_accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(slot_cancel()));

    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::slot_accept()
{
    m_pGeneralOptionsWidget->saveOptions();
    m_pShortCutOptionsWidget->saveOptions();

    this->accept();
}

void OptionsDialog::slot_cancel()
{
    bool bChanged1 = m_pGeneralOptionsWidget->isOptionsChanged();
    bool bChanged2 = m_pShortCutOptionsWidget->isOptionsChanged();
    if (bChanged1|bChanged2)
    {
        int ret = QMessageBox::question(this, tr("提示"), tr("配置信息已修改，是否保存后退出？"), tr("保存"), tr("取消"));
        if (0 == ret)
        {
            m_pGeneralOptionsWidget->saveOptions();
            m_pShortCutOptionsWidget->saveOptions();
            this->accept();
            return;
        }
    }
    this->reject();
}
