#ifndef QUESTIONDIALOG_H
#define QUESTIONDIALOG_H

#include <QDialog>

namespace Ui {
class QuestionDialog;
}

class QuestionDialog : public QDialog
{
    Q_OBJECT

public:
	explicit QuestionDialog(const QString& strTitle, const QString& strText, const QString& strTips, QWidget *parent = 0);
	bool isNolongerTips();
	~QuestionDialog();

private:
	Ui::QuestionDialog *ui;
};

#endif // QUESTIONDIALOG_H
