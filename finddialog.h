// finddialog.h
#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

class QLineEdit;
class QRadioButton;
class QCheckBox;

class FindDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FindDialog(QWidget *parent = nullptr);

    void setFindText(const QString &text);

signals:
    void findRequested(const QString &text, bool backward, bool caseSensitive);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onFindClicked();

private:
    QLineEdit    *m_findEdit  = nullptr;
    QRadioButton *m_upRadio   = nullptr;
    QRadioButton *m_downRadio = nullptr;
    QCheckBox    *m_caseCheck = nullptr;
};

#endif // FINDDIALOG_H
