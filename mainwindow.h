#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextCharFormat>

class CodeEditor;
class FindDialog;
class ReplaceDialog;

class QAction;
class QLabel;
class QTimer;
class QToolBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT  // 必须有这个宏，否则会报 vtable 错误
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();

    // 查找 / 替换
    void showFindDialog();
    void showReplaceDialog();
    void onFindRequested(const QString &text, bool backward, bool caseSensitive);
    void onReplaceRequested(const QString &text, const QString &replacement,
                            bool backward, bool caseSensitive);
    void onReplaceAllRequested(const QString &text, const QString &replacement,
                               bool caseSensitive);

    // 格式
    void chooseFont();
    void chooseFontColor();
    void chooseFontBackgroundColor();
    void chooseEditorBackgroundColor();

    // 其他
    void showAbout();
    void updateActions();
    void updateStatusBar();
    void updateWindowTitle();
    void autoSave();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void connectEditorSignals();

    bool maybeSave();
    bool loadFile(const QString &fileName);
    bool writeFile(const QString &fileName);
    bool findText(const QString &text, bool backward, bool caseSensitive, bool wrap);
    int  replaceAll(const QString &text, const QString &replacement, bool caseSensitive);
    void applyCharFormat(const QTextCharFormat &format);

private:
    CodeEditor  *m_editor = nullptr;
    FindDialog  *m_findDialog = nullptr;
    ReplaceDialog *m_replaceDialog = nullptr;
    QTimer      *m_autoSaveTimer = nullptr;
    QString      m_currentFile;

    QToolBar *m_toolBar = nullptr;

    QAction *m_newAct         = nullptr;
    QAction *m_openAct        = nullptr;
    QAction *m_saveAct        = nullptr;
    QAction *m_saveAsAct      = nullptr;
    QAction *m_exitAct        = nullptr;
    QAction *m_undoAct        = nullptr;
    QAction *m_redoAct        = nullptr;
    QAction *m_cutAct         = nullptr;
    QAction *m_copyAct        = nullptr;
    QAction *m_pasteAct       = nullptr;
    QAction *m_findAct        = nullptr;
    QAction *m_replaceAct     = nullptr;
    QAction *m_selectAllAct   = nullptr;
    QAction *m_wrapAct        = nullptr;
    QAction *m_lineNumberAct  = nullptr;
    QAction *m_fontAct        = nullptr;
    QAction *m_fontColorAct   = nullptr;
    QAction *m_fontBgAct      = nullptr;
    QAction *m_editorBgAct    = nullptr;
    QAction *m_toolBarAct     = nullptr;
    QAction *m_statusBarAct   = nullptr;
    QAction *m_aboutAct       = nullptr;

    QLabel *m_lengthLabel = nullptr;
    QLabel *m_linesLabel  = nullptr;
    QLabel *m_posLabel    = nullptr;
    QLabel *m_authorLabel = nullptr;
};

#endif // MAINWINDOW_H
