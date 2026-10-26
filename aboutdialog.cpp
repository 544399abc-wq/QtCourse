// aboutdialog.cpp
#include "aboutdialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("关于"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumWidth(430);

    QLabel *titleLabel = new QLabel(tr("小牛文本编辑"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *infoLabel = new QLabel(this);
    infoLabel->setTextFormat(Qt::RichText);
    infoLabel->setText(tr("开发者：袁光宇<br>"
                          "学号：2024414300123<br>"
                          "email：*****@qq.com"));
    infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *versionLabel = new QLabel(tr("V1.0 build 202611"), this);

    QPushButton *okButton = new QPushButton(tr("OK"), this);
    okButton->setFixedWidth(110);
    connect(okButton, &QPushButton::clicked, this, &AboutDialog::accept);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(versionLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(okButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(infoLabel);
    mainLayout->addStretch();
    mainLayout->addLayout(bottomLayout);
}
