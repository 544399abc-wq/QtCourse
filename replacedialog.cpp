// replacedialog.cpp
#include "replacedialog.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>

ReplaceDialog::ReplaceDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("替换"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(false);

    m_findEdit    = new QLineEdit(this);
    m_replaceEdit = new QLineEdit(this);
    m_upRadio     = new QRadioButton(tr("向上(U)"), this);
    m_downRadio   = new QRadioButton(tr("向下(D)"), this);
    m_downRadio->setChecked(true);
    m_caseCheck   = new QCheckBox(tr("区分大小写"), this);

    QPushButton *findBtn       = new QPushButton(tr("查找下一个(F)"), this);
    QPushButton *replaceBtn    = new QPushButton(tr("替换(R)"), this);
    QPushButton *replaceAllBtn = new QPushButton(tr("全部替换(A)"), this);
    QPushButton *cancelBtn     = new QPushButton(tr("取消"), this);
    findBtn->setDefault(true);

    QHBoxLayout *dirLayout = new QHBoxLayout;
    dirLayout->addWidget(new QLabel(tr("方向"), this));
    dirLayout->addWidget(m_upRadio);
    dirLayout->addWidget(m_downRadio);
    dirLayout->addStretch();

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(new QLabel(tr("查找目标："), this), 0, 0);
    layout->addWidget(m_findEdit,    0, 1, 1, 2);
    layout->addWidget(findBtn,       0, 3);
    layout->addWidget(new QLabel(tr("替换为："), this), 1, 0);
    layout->addWidget(m_replaceEdit, 1, 1, 1, 2);
    layout->addWidget(replaceBtn,    1, 3);
    layout->addLayout(dirLayout,     2, 0, 1, 2);
    layout->addWidget(m_caseCheck,   2, 2);
    layout->addWidget(replaceAllBtn, 2, 3);
    layout->addWidget(cancelBtn,     3, 3);

    connect(findBtn,       &QPushButton::clicked, this, &ReplaceDialog::onFindClicked);
    connect(replaceBtn,    &QPushButton::clicked, this, &ReplaceDialog::onReplaceClicked);
    connect(replaceAllBtn, &QPushButton::clicked, this, &ReplaceDialog::onReplaceAllClicked);
    connect(cancelBtn,     &QPushButton::clicked, this, &QDialog::close);
    connect(m_findEdit, &QLineEdit::returnPressed, this, &ReplaceDialog::onFindClicked);

    resize(480, sizeHint().height());
}

void ReplaceDialog::setFindText(const QString &text)
{
    m_findEdit->setText(text);
    m_findEdit->selectAll();
}

void ReplaceDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    m_findEdit->setFocus();
    m_findEdit->selectAll();
}

void ReplaceDialog::onFindClicked()
{
    emit findRequested(m_findEdit->text(),
                       m_upRadio->isChecked(),
                       m_caseCheck->isChecked());
}

void ReplaceDialog::onReplaceClicked()
{
    emit replaceRequested(m_findEdit->text(),
                          m_replaceEdit->text(),
                          m_upRadio->isChecked(),
                          m_caseCheck->isChecked());
}

void ReplaceDialog::onReplaceAllClicked()
{
    emit replaceAllRequested(m_findEdit->text(),
                             m_replaceEdit->text(),
                             m_caseCheck->isChecked());
}
