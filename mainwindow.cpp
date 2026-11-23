#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QApplication>
#include <QHeaderView> // 必须引入，用于设置表头拉伸

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("医院诊疗系统");
    resize(800, 500);

    // 1. 初始化数据库
    initDatabase();

    // 2. 设置页面堆栈
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    loginPage = createLoginPage();
    welcomePage = createWelcomePage();
    patientPage = createPatientPage();

    stackedWidget->addWidget(loginPage);    // 0
    stackedWidget->addWidget(welcomePage);  // 1
    stackedWidget->addWidget(patientPage);  // 2

    stackedWidget->setCurrentIndex(0); // 默认显示登录页
}

MainWindow::~MainWindow() {}

// ================= 数据库初始化 =================
void MainWindow::initDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");

    // 如果你要用回你本地的绝对路径，请把下面这行改成你的长路径
    QString dbPath = "D:/clinic.db";
    db.setDatabaseName(dbPath);
    qDebug() << "数据库路径:" << dbPath;

    if (!db.open()) {
        QMessageBox::critical(this, "数据库错误", "无法连接到数据库: " + db.lastError().text());
        return;
    }

    QSqlQuery query;
    // 创建 User 表
    query.exec("CREATE TABLE IF NOT EXISTS User (ID TEXT PRIMARY KEY, FULLNAME TEXT, USERNAME TEXT UNIQUE, PASSWORD TEXT)");
    // 创建 Patient 表
    query.exec("CREATE TABLE IF NOT EXISTS Patient (ID TEXT PRIMARY KEY, ID_CARD TEXT, NAME TEXT, SEX INTEGER, DOB TEXT, HEIGHT REAL, WEIGHT REAL, MOBILEPHONE TEXT, AGE INTEGER, CREATEDTIMESTAMP TEXT)");
    // 创建 History 表
    query.exec("CREATE TABLE IF NOT EXISTS History (ID INTEGER PRIMARY KEY AUTOINCREMENT, USER_ID TEXT, EVENT TEXT, TIMESTAMP TEXT)");

    // 插入默认管理员 (shiqi / shiqi)
    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT OR IGNORE INTO User (ID, FULLNAME, USERNAME, PASSWORD) VALUES (?, ?, ?, ?)");
    insertQuery.addBindValue("1");
    insertQuery.addBindValue("管理员");
    insertQuery.addBindValue("shiqi");
    insertQuery.addBindValue("shiqi");
    insertQuery.exec();
}

// ================= 记录操作日志 =================
void MainWindow::logEvent(const QString &event) {
    QSqlQuery query;
    query.prepare("INSERT INTO History (USER_ID, EVENT, TIMESTAMP) VALUES (?, ?, ?)");
    query.addBindValue("shiqi");
    query.addBindValue(event);
    query.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    query.exec();
}

// ================= 页面1：登录 =================
QWidget* MainWindow::createLoginPage() {
    QWidget *page = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(page);

    QLabel *titleLabel = new QLabel("欢迎使用诊疗测试系统");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin-bottom: 20px;");

    QFormLayout *formLayout = new QFormLayout;
    usernameEdit = new QLineEdit("shiqi");
    passwordEdit = new QLineEdit("shiqi");
    passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("用户名:", usernameEdit);
    formLayout->addRow("密码:", passwordEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *btnRegister = new QPushButton("注册");
    QPushButton *btnLogin = new QPushButton("登录");
    btnLayout->addWidget(btnRegister);
    btnLayout->addWidget(btnLogin);

    QWidget *centerWidget = new QWidget;
    QVBoxLayout *centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->addWidget(titleLabel);
    centerLayout->addLayout(formLayout);
    centerLayout->addLayout(btnLayout);
    mainLayout->addWidget(centerWidget, Qt::AlignCenter);

    QLabel *devInfo = new QLabel("开发者: 袁光宇  - 学号:2024414300123");
    devInfo->setAlignment(Qt::AlignCenter);
    devInfo->setStyleSheet("color: gray;");
    mainLayout->addWidget(devInfo, 0, Qt::AlignBottom);

    connect(btnRegister, &QPushButton::clicked, [=](){ QMessageBox::information(this, "提示", "暂未开放"); });
    connect(btnLogin, &QPushButton::clicked, this, &MainWindow::onLoginClicked);

    return page;
}

// ================= 页面2：欢迎页 =================
QWidget* MainWindow::createWelcomePage() {
    QWidget *page = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(page);

    QLabel *titleLabel = new QLabel("欢 迎");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; margin-bottom: 40px;");

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(30);

    // 只保留患者管理按钮
    QPushButton *btnPatient = new QPushButton("患者管理");
    btnPatient->setFixedSize(120, 80); // 设置按钮大小

    // 为了保持居中，在按钮左右两边添加弹簧
    btnLayout->addStretch();
    btnLayout->addWidget(btnPatient);
    btnLayout->addStretch();

    mainLayout->addStretch();
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(btnLayout);
    mainLayout->addStretch();

    connect(btnPatient, &QPushButton::clicked, this, &MainWindow::onPatientManageClicked);

    return page;
}

// ================= 页面3：患者管理 =================
QWidget* MainWindow::createPatientPage() {
    QWidget *page = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(page);

    QHBoxLayout *topBarLayout = new QHBoxLayout;
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("输入姓名查询...");
    QPushButton *btnSearch = new QPushButton("查找");
    QPushButton *btnAdd = new QPushButton("添加");
    QPushButton *btnDelete = new QPushButton("删除");
    QPushButton *btnModify = new QPushButton("修改");

    topBarLayout->addWidget(searchEdit);
    topBarLayout->addWidget(btnSearch);
    topBarLayout->addWidget(btnAdd);
    topBarLayout->addWidget(btnDelete);
    topBarLayout->addWidget(btnModify);

    tableView = new QTableView;
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 给表格设置尺寸策略，防止被按钮挤没
    tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 使用自定义模型绑定
    patientModel = new PatientModel(this, db);
    patientModel->setTable("Patient");
    patientModel->setHeaderData(0, Qt::Horizontal, "ID");
    patientModel->setHeaderData(1, Qt::Horizontal, "身份证");
    patientModel->setHeaderData(2, Qt::Horizontal, "姓名");
    patientModel->setHeaderData(3, Qt::Horizontal, "性别");
    patientModel->setHeaderData(4, Qt::Horizontal, "出生日期");
    patientModel->setHeaderData(5, Qt::Horizontal, "身高");
    patientModel->setHeaderData(6, Qt::Horizontal, "体重");
    patientModel->setHeaderData(7, Qt::Horizontal, "手机号");
    patientModel->select();
    tableView->setModel(patientModel);

    // 隐藏不需要显示的列
    tableView->setColumnHidden(8, true);
    tableView->setColumnHidden(9, true);

    // 自动拉伸填满窗口
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    mainLayout->addLayout(topBarLayout);
    mainLayout->addWidget(tableView);

    QPushButton *btnBack = new QPushButton("< 返回主页");
    mainLayout->addWidget(btnBack, 0, Qt::AlignLeft);

    // --- 按钮事件绑定 ---
    connect(btnBack, &QPushButton::clicked, [=](){ stackedWidget->setCurrentIndex(1); });
    connect(btnAdd, &QPushButton::clicked, [=](){ showEditDialog(-1); });

    // 修改
    connect(btnModify, &QPushButton::clicked, [=](){
        int row = tableView->currentIndex().row();
        if (row < 0) {
            QMessageBox::warning(this, "提示", "请先选择要修改的行！");
            return;
        }
        showEditDialog(row);
    });

    // 查找
    connect(btnSearch, &QPushButton::clicked, [=](){
        QString keyword = searchEdit->text().trimmed();
        if (keyword.isEmpty()) {
            patientModel->setFilter("");
        } else {
            patientModel->setFilter(QString("NAME LIKE '%%1%'").arg(keyword));
        }
        patientModel->select();
    });

    // 删除
    connect(btnDelete, &QPushButton::clicked, [=](){
        int row = tableView->currentIndex().row();
        if (row < 0) {
            QMessageBox::warning(this, "提示", "请先选择要删除的行！");
            return;
        }

        QString name = patientModel->data(patientModel->index(row, 2)).toString();
        if (QMessageBox::question(this, "确认删除", QString("确定要删除患者【%1】吗？").arg(name),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
            return;
        }

        if (patientModel->removeRow(row)) {
            if (patientModel->submitAll()) {
                patientModel->select();
                logEvent("删除了患者: " + name);
                QMessageBox::information(this, "成功", "删除成功！");
            } else {
                patientModel->revertAll();
                QMessageBox::critical(this, "错误", "删除失败！");
            }
        }
    });

    return page;
}

// ================= 弹窗：添加/修改患者 =================
void MainWindow::showEditDialog(int row) {
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(row == -1 ? "添加患者" : "修改患者");
    dialog->setFixedWidth(400);

    QFormLayout *formLayout = new QFormLayout;
    QLineEdit *idEdit = new QLineEdit;
    QLineEdit *nameEdit = new QLineEdit;
    QLineEdit *idCardEdit = new QLineEdit;
    QComboBox *sexCombo = new QComboBox;
    sexCombo->addItems({"男", "女"});
    QDateEdit *dobEdit = new QDateEdit(QDate(2000, 1, 1));
    dobEdit->setCalendarPopup(true);
    dobEdit->setDisplayFormat("yyyy-MM-dd");
    QDoubleSpinBox *heightSpin = new QDoubleSpinBox;
    heightSpin->setRange(0, 300);
    QDoubleSpinBox *weightSpin = new QDoubleSpinBox;
    weightSpin->setRange(0, 500);
    QLineEdit *phoneEdit = new QLineEdit;

    formLayout->addRow("ID:", idEdit);
    formLayout->addRow("姓名:", nameEdit);
    formLayout->addRow("身份证:", idCardEdit);
    formLayout->addRow("性别:", sexCombo);
    formLayout->addRow("出生日期:", dobEdit);
    formLayout->addRow("身高:", heightSpin);
    formLayout->addRow("体重:", weightSpin);
    formLayout->addRow("手机号:", phoneEdit);

    // 如果是修改，回显数据
    if (row >= 0) {
        idEdit->setText(patientModel->data(patientModel->index(row, 0)).toString());
        idEdit->setReadOnly(true);
        idCardEdit->setText(patientModel->data(patientModel->index(row, 1)).toString());
        nameEdit->setText(patientModel->data(patientModel->index(row, 2)).toString());
        sexCombo->setCurrentIndex(patientModel->data(patientModel->index(row, 3)).toInt());
        dobEdit->setDate(QDate::fromString(patientModel->data(patientModel->index(row, 4)).toString(), "yyyy-MM-dd"));
        heightSpin->setValue(patientModel->data(patientModel->index(row, 5)).toDouble());
        weightSpin->setValue(patientModel->data(patientModel->index(row, 6)).toDouble());
        phoneEdit->setText(patientModel->data(patientModel->index(row, 7)).toString());
    } else {
        idEdit->setText(QString::number(QDateTime::currentMSecsSinceEpoch()));
        idEdit->setReadOnly(true);
    }

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *btnSave = new QPushButton("保存");
    QPushButton *btnCancel = new QPushButton("取消");
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnCancel);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(btnLayout);

    connect(btnCancel, &QPushButton::clicked, dialog, &QDialog::reject);

    connect(btnSave, &QPushButton::clicked, [=](){
        if (nameEdit->text().trimmed().isEmpty() || idCardEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(dialog, "校验失败", "姓名和身份证号为必填项！");
            return;
        }

        QSqlQuery query;
        if (row == -1) {
            query.prepare("INSERT INTO Patient (ID, ID_CARD, NAME, SEX, DOB, HEIGHT, WEIGHT, MOBILEPHONE, CREATEDTIMESTAMP) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
            query.addBindValue(idEdit->text());
            query.addBindValue(idCardEdit->text());
            query.addBindValue(nameEdit->text());
            query.addBindValue(sexCombo->currentIndex());
            query.addBindValue(dobEdit->date().toString("yyyy-MM-dd"));
            query.addBindValue(heightSpin->value());
            query.addBindValue(weightSpin->value());
            query.addBindValue(phoneEdit->text());
            query.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        } else {
            query.prepare("UPDATE Patient SET ID_CARD=?, NAME=?, SEX=?, DOB=?, HEIGHT=?, WEIGHT=?, MOBILEPHONE=? WHERE ID=?");
            query.addBindValue(idCardEdit->text());
            query.addBindValue(nameEdit->text());
            query.addBindValue(sexCombo->currentIndex());
            query.addBindValue(dobEdit->date().toString("yyyy-MM-dd"));
            query.addBindValue(heightSpin->value());
            query.addBindValue(weightSpin->value());
            query.addBindValue(phoneEdit->text());
            query.addBindValue(idEdit->text());
        }

        if (query.exec()) {
            patientModel->select();
            logEvent(row == -1 ? "添加了患者: " + nameEdit->text() : "修改了患者: " + nameEdit->text());
            dialog->accept();
        } else {
            QMessageBox::critical(dialog, "错误", "操作失败: " + query.lastError().text());
        }
    });

    dialog->exec();
}

// ================= 槽函数 =================
void MainWindow::onLoginClicked() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名和密码不能为空！");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT FULLNAME FROM User WHERE USERNAME = ? AND PASSWORD = ?");
    query.addBindValue(username);
    query.addBindValue(password);

    if (query.exec() && query.next()) {
        logEvent("用户登录成功");
        stackedWidget->setCurrentIndex(1);
    } else {
        QMessageBox::critical(this, "登录失败", "用户名或密码错误！");
        passwordEdit->clear();
        passwordEdit->setFocus();
    }
}

void MainWindow::onPatientManageClicked() {
    patientModel->select();
    stackedWidget->setCurrentIndex(2);
}
