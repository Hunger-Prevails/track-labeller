#include "shortcutoptionswidget.h"
#include "ui_shortcutoptionswidget.h"
#include <QMessageBox>

#include "optionsdialog.h"
extern sys_option_t g_sysOptionInfo;

ShortCutOptionsWidget::ShortCutOptionsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShortCutOptionsWidget)
{
    ui->setupUi(this);

    m_pKeySequenceEditList << ui->keyLoadVideo;
    m_pKeySequenceEditList << ui->keyLoadLabel;
    m_pKeySequenceEditList << ui->keySaveLabel;
    m_pKeySequenceEditList << ui->keyCountTrack;
    m_pKeySequenceEditList << ui->keySequenceEditNextFrame;
    m_pKeySequenceEditList << ui->keySequenceEditPeriorFrame;
    m_pKeySequenceEditList << ui->keySequenceEditZoomIn;
    m_pKeySequenceEditList << ui->keySequenceEditZoomOut;
    m_pKeySequenceEditList << ui->keySequenceEditLoadVip;
    m_pKeySequenceEditList << ui->keySequenceEditSaveVipName;
    m_pKeySequenceEditList << ui->keySequenceEditUp;
    m_pKeySequenceEditList << ui->keySequenceEditDown;
    m_pKeySequenceEditList << ui->keySequenceEditLeft;
    m_pKeySequenceEditList << ui->keySequenceEditRight;

    QKeySequenceEdit* pEdit = NULL;
    for (int i=0; i<m_pKeySequenceEditList.count(); ++i)
    {
        pEdit = m_pKeySequenceEditList.at(i);
        if (pEdit != NULL)
        {
            connect(pEdit, SIGNAL(editingFinished()), this, SLOT(slot_editFinished()));

            if (i < g_sysOptionInfo.keySequenceList.count())
                pEdit->setKeySequence(g_sysOptionInfo.keySequenceList.at(i));
        }
    }
}

ShortCutOptionsWidget::~ShortCutOptionsWidget()
{
    delete ui;
}

void ShortCutOptionsWidget::slot_editFinished()
{
    QKeySequenceEdit* pEdit = qobject_cast<QKeySequenceEdit*>(sender());
    foreach (QKeySequenceEdit* pTmp, m_pKeySequenceEditList)
    {
        if (pTmp != pEdit && pTmp->keySequence() == pEdit->keySequence())
        {
            QMessageBox::warning(this, tr("警告"), tr("检测到快捷键冲突，请重新设定！"), tr("确定"), tr("取消"));
            pEdit->clear();
            return;
        }
    }
}

void ShortCutOptionsWidget::saveOptions()
{
    QKeySequenceEdit* pEdit = NULL;
    for (int i=0; i<m_pKeySequenceEditList.count(); ++i)
    {
        pEdit = m_pKeySequenceEditList.at(i);
        if (pEdit != NULL && i < g_sysOptionInfo.keySequenceList.count())
            g_sysOptionInfo.keySequenceList.replace(i, pEdit->keySequence());
    }
}

bool ShortCutOptionsWidget::isOptionsChanged()
{
    bool bChanged = false;
    for (int i=0; i<m_pKeySequenceEditList.count(); ++i)
    {
        QKeySequenceEdit *pEdit = m_pKeySequenceEditList.at(i);
        if (pEdit != NULL && i < g_sysOptionInfo.keySequenceList.count())
        {
            if (g_sysOptionInfo.keySequenceList.at(i) != pEdit->keySequence())
            {
                bChanged = true;
                break;
            }
        }
    }
    return bChanged;
}
