// replacedialog.h
#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

class QLineEdit;
class QRadioButton;
class QCheckBox;

class ReplaceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReplaceDialog(QWidget *parent = nullptr);

    void setFindText(const QString &text);

signals:
    void findRequested(const QString &text, bool backward, bool caseSensitive);
    void replaceRequested(const QString &text, const QString &replacement,
                          bool backward, bool caseSensitive);
    void replaceAllRequested(const QString &text, const QString &replacement,
                             bool caseSensitive);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onFindClicked();
    void onReplaceClicked();
    void onReplaceAllClicked();

private:
    QLineEdit    *m_findEdit       = nullptr;
    QLineEdit    *m_replaceEdit    = nullptr;
    QRadioButton *m_upRadio        = nullptr;
    QRadioButton *m_downRadio      = nullptr;
    QCheckBox    *m_caseCheck      = nullptr;
};

#endif // REPLACEDIALOG_H
