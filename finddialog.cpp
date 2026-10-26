// finddialog.cpp
#include "finddialog.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("查找"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(false);

    m_findEdit  = new QLineEdit(this);
    m_upRadio   = new QRadioButton(tr("向上(U)"), this);
    m_downRadio = new QRadioButton(tr("向下(D)"), this);
    m_downRadio->setChecked(true);
    m_caseCheck = new QCheckBox(tr("区分大小写"), this);

    QPushButton *findBtn   = new QPushButton(tr("查找下一个(F)"), this);
    QPushButton *cancelBtn = new QPushButton(tr("取消"), this);
    findBtn->setDefault(true);

    QHBoxLayout *dirLayout = new QHBoxLayout;
    dirLayout->addWidget(new QLabel(tr("方向"), this));
    dirLayout->addWidget(m_upRadio);
    dirLayout->addWidget(m_downRadio);
    dirLayout->addStretch();

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(new QLabel(tr("查找目标："), this), 0, 0);
    layout->addWidget(m_findEdit, 0, 1, 1, 2);
    layout->addWidget(findBtn,    0, 3);
    layout->addLayout(dirLayout,  1, 0, 1, 2);
    layout->addWidget(m_caseCheck,1, 2);
    layout->addWidget(cancelBtn,  1, 3);

    connect(findBtn,   &QPushButton::clicked, this, &FindDialog::onFindClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::close);
    connect(m_findEdit, &QLineEdit::returnPressed, this, &FindDialog::onFindClicked);

    resize(430, sizeHint().height());
}

void FindDialog::setFindText(const QString &text)
{
    m_findEdit->setText(text);
    m_findEdit->selectAll();
}

void FindDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    m_findEdit->setFocus();
    m_findEdit->selectAll();
}

void FindDialog::onFindClicked()
{
    emit findRequested(m_findEdit->text(),
                       m_upRadio->isChecked(),
                       m_caseCheck->isChecked());
}
