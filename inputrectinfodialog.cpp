#include "inputrectinfodialog.h"
#include "ui_inputrectinfodialog.h"

InputRectInfoDialog::InputRectInfoDialog(int pre_tid, int pre_label, bool pre_activated, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InputRectInfoDialog)
{
    ui->setupUi(this);
    ui->lineEditTrackId->setText(QString::number(pre_tid));

    group=new QButtonGroup(this);
    group->addButton(ui->radio_pad, 1);
    group->addButton(ui->radio_face, 2);

    if(pre_label==1)
        ui->radio_pad->setChecked(true);
    else if(pre_label==2)
        ui->radio_face->setChecked(true);

    ui->check_generate->setChecked(pre_activated);

    connect(ui->buttonBox, SIGNAL(accepted()), this ,SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

InputRectInfoDialog::~InputRectInfoDialog()
{
    delete ui;
}

int InputRectInfoDialog::getTrackId()
{
    return ui->lineEditTrackId->text().toInt();
}

int InputRectInfoDialog::getLabel()
{
    return group->checkedId();
}

bool InputRectInfoDialog::isActivated(){
    return ui->check_generate->isChecked();
}
