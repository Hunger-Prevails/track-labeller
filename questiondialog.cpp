#include "questiondialog.h"
#include "ui_questiondialog.h"

QuestionDialog::QuestionDialog(const QString& strTitle, const QString& strText, const QString& strTips,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuestionDialog)
{
	ui->setupUi(this);
	this->setWindowTitle(strTitle);
	ui->label->setText(strText);
	ui->checkBox->setText(strTips);
	connect(ui->pushButtonOk, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect(ui->pushButtonCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
}

QuestionDialog::~QuestionDialog()
{
    delete ui;
}

bool QuestionDialog::isNolongerTips()
{
    return ui->checkBox->isChecked();
}
