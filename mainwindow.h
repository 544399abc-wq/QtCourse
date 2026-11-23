#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QLabel>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QDateTime>

// ================= 自定义模型：解决性别 0/1 显示为 男/女 的问题 =================
class PatientModel : public QSqlTableModel {
public:
    explicit PatientModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase())
        : QSqlTableModel(parent, db) {}

    // 重写 data 函数
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (role == Qt::DisplayRole && index.column() == 3) { // 第3列是性别
            int sexVal = QSqlTableModel::data(index, role).toInt();
            return (sexVal == 0) ? "男" : "女";
        }
        return QSqlTableModel::data(index, role);
    }
};

// ================= 主窗口类 =================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget *stackedWidget;
    QWidget *loginPage, *welcomePage, *patientPage;

    void initDatabase();
    void logEvent(const QString &event);

    QWidget* createLoginPage();
    QWidget* createWelcomePage();
    QWidget* createPatientPage();
    void showEditDialog(int row = -1);

    QLineEdit *usernameEdit, *passwordEdit;
    QLineEdit *searchEdit;
    QTableView *tableView;

    QSqlDatabase db;
    PatientModel *patientModel; // 使用自定义模型

private slots:
    void onLoginClicked();
    void onPatientManageClicked();
};

#endif // MAINWINDOW_H
