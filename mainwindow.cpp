#include "mainwindow.h"
#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"
#include "aboutdialog.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextStream>
#include <QTimer>
#include <QToolBar>

/* =========================== 构造 =========================== */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("新建文本文件 - 编辑器"));
    resize(960, 640);

    m_editor = new CodeEditor(this);
    m_editor->setContentsMargins(4, 4, 4, 4);
    m_editor->document()->setDocumentMargin(6);
    setCentralWidget(m_editor);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    // 查找 / 替换对话框
    m_findDialog = new FindDialog(this);
    connect(m_findDialog, &FindDialog::findRequested,
            this, &MainWindow::onFindRequested);

    m_replaceDialog = new ReplaceDialog(this);
    connect(m_replaceDialog, &ReplaceDialog::findRequested,
            this, &MainWindow::onFindRequested);
    connect(m_replaceDialog, &ReplaceDialog::replaceRequested,
            this, &MainWindow::onReplaceRequested);
    connect(m_replaceDialog, &ReplaceDialog::replaceAllRequested,
            this, &MainWindow::onReplaceAllRequested);

    connectEditorSignals();

    // 定时自动保存（每 60 秒，仅针对已有路径且被修改的文件）
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
    m_autoSaveTimer->start(60 * 1000);

    updateActions();
    updateStatusBar();
    updateWindowTitle();
}

/* =========================== Action =========================== */

void MainWindow::createActions()
{
    /* ---------------- 文件 ---------------- */
    m_newAct = new QAction(tr("新建(&N)"), this);
    m_newAct->setShortcut(QKeySequence::New);
    m_newAct->setStatusTip(tr("新建一个文本文件"));
    connect(m_newAct, &QAction::triggered, this, &MainWindow::newFile);

    m_openAct = new QAction(tr("打开(&O)..."), this);
    m_openAct->setShortcut(QKeySequence::Open);
    m_openAct->setStatusTip(tr("打开一个文本文件"));
    connect(m_openAct, &QAction::triggered, this, &MainWindow::openFile);

    m_saveAct = new QAction(tr("保存(&S)"), this);
    m_saveAct->setShortcut(QKeySequence::Save);
    m_saveAct->setStatusTip(tr("保存当前文件"));
    connect(m_saveAct, &QAction::triggered, this, [this] { saveFile(); });

    m_saveAsAct = new QAction(tr("另存为(&A)..."), this);
    m_saveAsAct->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+S")));
    m_saveAsAct->setStatusTip(tr("将当前内容另存为新文件"));
    connect(m_saveAsAct, &QAction::triggered, this, [this] { saveFileAs(); });

    m_exitAct = new QAction(tr("退出(&X)"), this);
    m_exitAct->setShortcut(QKeySequence(QStringLiteral("Ctrl+Q")));
    connect(m_exitAct, &QAction::triggered, this, &QWidget::close);

    /* ---------------- 编辑 ---------------- */
    m_undoAct = new QAction(tr("撤销(&U)"), this);
    m_undoAct->setShortcut(QKeySequence::Undo);
    connect(m_undoAct, &QAction::triggered, m_editor, &QPlainTextEdit::undo);

    m_redoAct = new QAction(tr("恢复(&R)"), this);
    m_redoAct->setShortcuts({ QKeySequence(QStringLiteral("Ctrl+Shift+Z")),
                             QKeySequence(QStringLiteral("Ctrl+Y")) });
    connect(m_redoAct, &QAction::triggered, m_editor, &QPlainTextEdit::redo);

    m_cutAct = new QAction(tr("剪切(&T)"), this);
    m_cutAct->setShortcut(QKeySequence::Cut);
    connect(m_cutAct, &QAction::triggered, m_editor, &QPlainTextEdit::cut);

    m_copyAct = new QAction(tr("复制(&C)"), this);
    m_copyAct->setShortcut(QKeySequence::Copy);
    connect(m_copyAct, &QAction::triggered, m_editor, &QPlainTextEdit::copy);

    m_pasteAct = new QAction(tr("粘贴(&P)"), this);
    m_pasteAct->setShortcut(QKeySequence::Paste);
    connect(m_pasteAct, &QAction::triggered, m_editor, &QPlainTextEdit::paste);

    m_findAct = new QAction(tr("查找(&F)..."), this);
    m_findAct->setShortcut(QKeySequence::Find);
    connect(m_findAct, &QAction::triggered, this, &MainWindow::showFindDialog);

    m_replaceAct = new QAction(tr("替换(&R)..."), this);
    m_replaceAct->setShortcut(QKeySequence(QStringLiteral("Ctrl+H")));
    connect(m_replaceAct, &QAction::triggered, this, &MainWindow::showReplaceDialog);

    m_selectAllAct = new QAction(tr("全选(&A)"), this);
    m_selectAllAct->setShortcut(QKeySequence::SelectAll);
    connect(m_selectAllAct, &QAction::triggered, m_editor, &QPlainTextEdit::selectAll);

    /* ---------------- 格式 ---------------- */
    m_wrapAct = new QAction(tr("自动换行(&W)"), this);
    m_wrapAct->setCheckable(true);
    m_wrapAct->setChecked(false);
    connect(m_wrapAct, &QAction::toggled, this, [this](bool on) {
        m_editor->setLineWrapMode(on ? QPlainTextEdit::WidgetWidth
                                     : QPlainTextEdit::NoWrap);
    });

    m_lineNumberAct = new QAction(tr("显示行号(&L)"), this);
    m_lineNumberAct->setCheckable(true);
    m_lineNumberAct->setChecked(true);
    m_lineNumberAct->setShortcut(QKeySequence(QStringLiteral("Ctrl+L")));
    connect(m_lineNumberAct, &QAction::toggled,
            m_editor, &CodeEditor::setLineNumberVisible);

    m_fontAct = new QAction(tr("字体(&F)..."), this);
    connect(m_fontAct, &QAction::triggered, this, &MainWindow::chooseFont);

    m_fontColorAct = new QAction(tr("字体颜色..."), this);
    connect(m_fontColorAct, &QAction::triggered, this, &MainWindow::chooseFontColor);

    m_fontBgAct = new QAction(tr("字体背景色..."), this);
    connect(m_fontBgAct, &QAction::triggered, this,
            &MainWindow::chooseFontBackgroundColor);

    m_editorBgAct = new QAction(tr("编辑器背景色..."), this);
    connect(m_editorBgAct, &QAction::triggered, this,
            &MainWindow::chooseEditorBackgroundColor);

    /* ---------------- 查看 ---------------- */
    m_toolBarAct = new QAction(tr("工具栏(&T)"), this);
    m_toolBarAct->setCheckable(true);
    m_toolBarAct->setChecked(true);

    m_statusBarAct = new QAction(tr("状态栏(&S)"), this);
    m_statusBarAct->setCheckable(true);
    m_statusBarAct->setChecked(true);

    /* ---------------- 帮助 ---------------- */
    m_aboutAct = new QAction(tr("关于(&A)..."), this);
    connect(m_aboutAct, &QAction::triggered, this, &MainWindow::showAbout);
}

/* =========================== 菜单 =========================== */

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("文件(&F)"));
    fileMenu->addAction(m_newAct);
    fileMenu->addAction(m_openAct);
    fileMenu->addAction(m_saveAct);
    fileMenu->addAction(m_saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAct);

    QMenu *editMenu = menuBar()->addMenu(tr("编辑(&E)"));
    editMenu->addAction(m_undoAct);
    editMenu->addAction(m_redoAct);
    editMenu->addSeparator();
    editMenu->addAction(m_cutAct);
    editMenu->addAction(m_copyAct);
    editMenu->addAction(m_pasteAct);
    editMenu->addSeparator();
    editMenu->addAction(m_findAct);
    editMenu->addAction(m_replaceAct);
    editMenu->addSeparator();
    editMenu->addAction(m_selectAllAct);

    QMenu *formatMenu = menuBar()->addMenu(tr("格式(&O)"));
    formatMenu->addAction(m_wrapAct);
    formatMenu->addAction(m_lineNumberAct);
    formatMenu->addSeparator();
    formatMenu->addAction(m_fontAct);
    formatMenu->addAction(m_fontColorAct);
    formatMenu->addAction(m_fontBgAct);
    formatMenu->addSeparator();
    formatMenu->addAction(m_editorBgAct);

    QMenu *viewMenu = menuBar()->addMenu(tr("查看(&V)"));
    viewMenu->addAction(m_toolBarAct);
    viewMenu->addAction(m_statusBarAct);

    QMenu *helpMenu = menuBar()->addMenu(tr("帮助(&H)"));
    helpMenu->addAction(m_aboutAct);
}

/* =========================== 工具栏 =========================== */

void MainWindow::createToolBars()
{
    m_toolBar = addToolBar(tr("主工具栏"));
    m_toolBar->setObjectName(QStringLiteral("mainToolBar"));
    m_toolBar->setMovable(false);
    m_toolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    m_toolBar->addAction(m_newAct);
    m_toolBar->addAction(m_openAct);
    m_toolBar->addAction(m_saveAct);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_undoAct);
    m_toolBar->addAction(m_redoAct);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_cutAct);
    m_toolBar->addAction(m_copyAct);
    m_toolBar->addAction(m_pasteAct);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_wrapAct);
    m_toolBar->addAction(m_lineNumberAct);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_findAct);
    m_toolBar->addAction(m_replaceAct);

    connect(m_toolBarAct, &QAction::toggled, m_toolBar, &QToolBar::setVisible);
}

/* =========================== 状态栏 =========================== */

void MainWindow::createStatusBar()
{
    m_lengthLabel = new QLabel(this);
    m_linesLabel  = new QLabel(this);
    m_posLabel    = new QLabel(this);
    m_authorLabel = new QLabel(tr("袁光宇"), this);

    statusBar()->addPermanentWidget(m_lengthLabel);
    statusBar()->addPermanentWidget(m_linesLabel);
    statusBar()->addPermanentWidget(m_posLabel);
    statusBar()->addPermanentWidget(m_authorLabel);
    statusBar()->showMessage(tr("就绪"), 2000);

    connect(m_statusBarAct, &QAction::toggled, this, [this](bool on) {
        statusBar()->setVisible(on);
    });
}

/* =========================== 信号连接 =========================== */

void MainWindow::connectEditorSignals()
{
    connect(m_editor, &QPlainTextEdit::textChanged, this, [this] {
        updateStatusBar();
        updateActions();
    });
    connect(m_editor, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateStatusBar);
    connect(m_editor, &QPlainTextEdit::copyAvailable,
            this, &MainWindow::updateActions);
    connect(m_editor->document(), &QTextDocument::undoAvailable,
            this, &MainWindow::updateActions);
    connect(m_editor->document(), &QTextDocument::redoAvailable,
            this, &MainWindow::updateActions);
    connect(m_editor->document(), &QTextDocument::modificationChanged,
            this, [this](bool) {
                updateWindowTitle();
                updateActions();
            });
    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::updateActions);
}

/* =========================== 文件操作 =========================== */

void MainWindow::newFile()
{
    if (!maybeSave())
        return;

    m_editor->clear();
    m_currentFile.clear();
    m_editor->document()->setModified(false);
    updateWindowTitle();
    statusBar()->showMessage(tr("已新建文件"), 2000);
}

void MainWindow::openFile()
{
    if (!maybeSave())
        return;

    const QString fileName = QFileDialog::getOpenFileName(
        this, tr("打开文件"), QString(),
        tr("文本文件 (*.txt);;所有文件 (*)"));
    if (fileName.isEmpty())
        return;

    loadFile(fileName);
}

bool MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("打开文件"),
                             tr("无法打开文件 \"%1\"：\n%2")
                                 .arg(fileName, file.errorString()));
        return false;
    }

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#endif
    const QString content = in.readAll();
    file.close();

    m_editor->setPlainText(content);
    m_currentFile = fileName;
    m_editor->document()->setModified(false);
    updateWindowTitle();
    statusBar()->showMessage(tr("已打开 %1").arg(fileName), 3000);
    return true;
}

bool MainWindow::saveFile()
{
    if (m_currentFile.isEmpty())
        return saveFileAs();
    return writeFile(m_currentFile);
}

bool MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("另存为"),
        m_currentFile.isEmpty() ? QString() : m_currentFile,
        tr("文本文件 (*.txt);;所有文件 (*)"));
    if (fileName.isEmpty())
        return false;

    // 未写扩展名时自动补 .txt
    if (!QFileInfo(fileName).suffix().isEmpty() == false)
        fileName += QStringLiteral(".txt");

    return writeFile(fileName);
}

bool MainWindow::writeFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("保存文件"),
                             tr("无法保存文件 \"%1\"：\n%2")
                                 .arg(fileName, file.errorString()));
        return false;
    }

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#endif
    out << m_editor->toPlainText();
    file.close();

    m_currentFile = fileName;
    m_editor->document()->setModified(false);
    updateWindowTitle();
    statusBar()->showMessage(tr("已保存 %1").arg(fileName), 3000);
    return true;
}

bool MainWindow::maybeSave()
{
    if (!m_editor->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret =
        QMessageBox::warning(this, tr("简单文本编辑器"),
                             tr("文档已被修改，是否保存？"),
                             QMessageBox::Save | QMessageBox::Discard
                                 | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return saveFile();
    if (ret == QMessageBox::Cancel)
        return false;
    return true;    // Discard
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

void MainWindow::autoSave()
{
    if (!m_currentFile.isEmpty() && m_editor->document()->isModified()) {
        if (writeFile(m_currentFile))
            statusBar()->showMessage(tr("已自动保存"), 1500);
    }
}

/* =========================== 查找 / 替换 =========================== */

void MainWindow::showFindDialog()
{
    if (m_replaceDialog->isVisible())
        m_replaceDialog->hide();

    const QString selected = m_editor->textCursor().selectedText();
    if (!selected.isEmpty() && !selected.contains(QChar::ParagraphSeparator))
        m_findDialog->setFindText(selected);

    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::showReplaceDialog()
{
    if (m_findDialog->isVisible())
        m_findDialog->hide();

    const QString selected = m_editor->textCursor().selectedText();
    if (!selected.isEmpty() && !selected.contains(QChar::ParagraphSeparator))
        m_replaceDialog->setFindText(selected);

    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
}

bool MainWindow::findText(const QString &text, bool backward, bool caseSensitive, bool wrap)
{
    if (text.isEmpty())
        return false;

    QTextDocument::FindFlags flags;
    if (backward)
        flags |= QTextDocument::FindBackward;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    if (m_editor->find(text, flags))
        return true;

    if (!wrap)
        return false;

    // 回绕：把光标移到文档首/尾后重新查找一次
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
    m_editor->setTextCursor(cursor);

    return m_editor->find(text, flags);
}

void MainWindow::onFindRequested(const QString &text, bool backward, bool caseSensitive)
{
    if (text.isEmpty()) {
        statusBar()->showMessage(tr("请输入要查找的内容"), 3000);
        return;
    }

    if (findText(text, backward, caseSensitive, true))
        statusBar()->showMessage(tr("找到 \"%1\"").arg(text), 2000);
    else
        statusBar()->showMessage(tr("未找到 \"%1\"").arg(text), 3000);

    updateStatusBar();
}

void MainWindow::onReplaceRequested(const QString &text, const QString &replacement,
                                    bool backward, bool caseSensitive)
{
    if (text.isEmpty()) {
        statusBar()->showMessage(tr("请输入要替换的内容"), 3000);
        return;
    }

    // 当前选中内容与查找内容一致 -> 直接替换
    QTextCursor cursor = m_editor->textCursor();
    bool currentIsMatch = false;
    if (cursor.hasSelection()) {
        const QString sel = cursor.selectedText();
        currentIsMatch = caseSensitive
                             ? (sel == text)
                             : (sel.compare(text, Qt::CaseInsensitive) == 0);
    }

    if (currentIsMatch) {
        cursor.insertText(replacement);
        m_editor->setTextCursor(cursor);
    }

    if (!findText(text, backward, caseSensitive, true))
        statusBar()->showMessage(tr("未找到 \"%1\"").arg(text), 3000);

    updateStatusBar();
}

int MainWindow::replaceAll(const QString &text, const QString &replacement, bool caseSensitive)
{
    if (text.isEmpty())
        return 0;

    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    QTextDocument *doc = m_editor->document();

    QTextCursor editCursor(doc);
    editCursor.beginEditBlock();     // 整批替换合并为一步撤销

    int count = 0;
    QTextCursor found = doc->find(text, 0, flags);
    while (!found.isNull()) {
        found.insertText(replacement);
        ++count;
        found = doc->find(text, found, flags);
    }

    editCursor.endEditBlock();
    return count;
}

void MainWindow::onReplaceAllRequested(const QString &text, const QString &replacement,
                                       bool caseSensitive)
{
    if (text.isEmpty()) {
        QMessageBox::information(m_replaceDialog, tr("替换"),
                                 tr("请输入要查找的内容。"));
        return;
    }

    const int count = replaceAll(text, replacement, caseSensitive);
    if (count > 0)
        QMessageBox::information(m_replaceDialog, tr("替换"),
                                 tr("共替换 %1 处。").arg(count));
    else
        QMessageBox::information(m_replaceDialog, tr("替换"),
                                 tr("未找到要替换的内容。"));

    updateStatusBar();
}

/* =========================== 格式操作 =========================== */

void MainWindow::chooseFont()
{
    bool ok = false;
    const QFont font = QFontDialog::getFont(&ok, m_editor->font(), this, tr("选择字体"));
    if (ok)
        m_editor->setFont(font);
}

void MainWindow::applyCharFormat(const QTextCharFormat &format)
{
    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::Document);
    cursor.mergeCharFormat(format);
    m_editor->mergeCurrentCharFormat(format);
}

void MainWindow::chooseFontColor()
{
    // 修复：改用 textCursor().charFormat() 获取颜色
    QColor defaultColor = m_editor->textCursor().charFormat().foreground().color();
    if (!defaultColor.isValid()) defaultColor = Qt::black;

    const QColor color = QColorDialog::getColor(defaultColor, this, tr("字体颜色"));
    if (!color.isValid())
        return;

    QTextCharFormat fmt;
    fmt.setForeground(color);
    applyCharFormat(fmt);
}

void MainWindow::chooseFontBackgroundColor()
{
    const QColor color = QColorDialog::getColor(Qt::yellow, this, tr("字体背景色"));
    if (!color.isValid())
        return;

    QTextCharFormat fmt;
    fmt.setBackground(color);
    applyCharFormat(fmt);
}

void MainWindow::chooseEditorBackgroundColor()
{
    const QColor color = QColorDialog::getColor(Qt::white, this, tr("编辑器背景色"));
    if (!color.isValid())
        return;

    m_editor->setStyleSheet(
        QStringLiteral("QPlainTextEdit { background-color: %1; color: black; }").arg(color.name()));
}

/* =========================== 关于 =========================== */

void MainWindow::showAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}

/* =========================== 状态更新 =========================== */

void MainWindow::updateActions()
{
    QTextDocument *doc = m_editor->document();

    m_undoAct->setEnabled(doc->isUndoAvailable());
    m_redoAct->setEnabled(doc->isRedoAvailable());

    const bool hasSelection = m_editor->textCursor().hasSelection();
    m_cutAct->setEnabled(hasSelection);
    m_copyAct->setEnabled(hasSelection);
    m_pasteAct->setEnabled(m_editor->canPaste());

    m_saveAct->setEnabled(doc->isModified() || !m_currentFile.isEmpty());
}

void MainWindow::updateStatusBar()
{
    const QString content = m_editor->toPlainText();
    m_lengthLabel->setText(tr("length: %1").arg(content.length()));
    m_linesLabel->setText(tr("lines: %1").arg(m_editor->blockCount()));

    const QTextCursor cursor = m_editor->textCursor();
    m_posLabel->setText(tr("Ln: %1   Col: %2")
                            .arg(cursor.blockNumber() + 1)
                            .arg(cursor.positionInBlock() + 1));
}

void MainWindow::updateWindowTitle()
{
    QString name = m_currentFile.isEmpty()
    ? tr("新建文本文件")
    : QFileInfo(m_currentFile).fileName();

    if (m_editor->document()->isModified())
        name.prepend(QLatin1Char('*'));

    setWindowTitle(name + tr(" - 编辑器"));
}
