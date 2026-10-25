#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFont>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QKeyEvent>

// Key labels that are not plain ASCII.
// Built from QChar code points (NOT \u string literals): MSVC with code page 936
// cannot represent U+2212 / U+232B in a narrow literal (warning C4566)
// and would turn them into "?".
static const QString K_MINUS = QString(QChar(0x2212));   // minus
static const QString K_TIMES = QString(QChar(0x00D7));   // multiply
static const QString K_DIV   = QString(QChar(0x00F7));   // divide
static const QString K_BACK  = QString(QChar(0x232B));   // backspace

static const int MAX_DIGITS = 16;   // max digits in one operand

class Calculator : public QWidget {
public:
    explicit Calculator(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle(QStringLiteral("\u8BA1\u7B97\u5668"));   // "Calculator"
        resize(420, 540);
        setFocusPolicy(Qt::StrongFocus);   // the window itself receives key events
        setStyleSheet(
            "QWidget{background:#f3f3f3;}"
            "QLabel#lblExpression{color:#666;padding:6px 16px 0 16px;font-size:16px;}"
            "QLabel#lblDisplay{color:black;padding:0 16px 10px 16px;}"
            "QPushButton{border:1px solid #e6e6e6;background:#fafafa;font-size:18px;color:#222;}"
            "QPushButton:hover{background:#e8e8e8;}"
            "QPushButton:pressed{background:#d8d8d8;}"
            "QPushButton[role='num']{background:#ffffff;font-weight:bold;}"
            "QPushButton[role='op']{background:#f0f0f0;}"
            "QPushButton[role='eq']{background:#8fc1ea;}"
            "QPushButton[role='eq']:hover{background:#79b2e0;}");

        // Expression line (small) + number line (large)
        lblExpression = new QLabel;
        lblExpression->setObjectName("lblExpression");
        lblExpression->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lblExpression->setMinimumHeight(34);

        lblDisplay = new QLabel("0");
        lblDisplay->setObjectName("lblDisplay");
        lblDisplay->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QFont font = lblDisplay->font();
        font.setPointSize(36);
        lblDisplay->setFont(font);
        lblDisplay->setMinimumHeight(90);

        // 5 x 4 button grid ("=" spans two rows, "0" spans two columns)
        struct KeyDef { QString text; const char *role; int row, col, rowSpan, colSpan; };
        const KeyDef keys[] = {
                               {"C","fn",0,0,1,1},      {K_DIV,"op",0,1,1,1},   {K_TIMES,"op",0,2,1,1},  {K_BACK,"fn",0,3,1,1},
                               {"7","num",1,0,1,1},     {"8","num",1,1,1,1},    {"9","num",1,2,1,1},     {K_MINUS,"op",1,3,1,1},
                               {"4","num",2,0,1,1},     {"5","num",2,1,1,1},    {"6","num",2,2,1,1},     {"+","op",2,3,1,1},
                               {"1","num",3,0,1,1},     {"2","num",3,1,1,1},    {"3","num",3,2,1,1},     {"=","eq",3,3,2,1},
                               {"0","num",4,0,1,2},     {".","fn",4,2,1,1},
                               };
        auto *grid = new QGridLayout;
        grid->setSpacing(0);
        grid->setContentsMargins(0, 0, 0, 0);
        for (int r = 0; r < 5; ++r) grid->setRowStretch(r, 1);
        for (int c = 0; c < 4; ++c) grid->setColumnStretch(c, 1);
        for (const KeyDef &k : keys) {
            auto *btn = new QPushButton(k.text);
            btn->setProperty("role", k.role);
            btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            btn->setFocusPolicy(Qt::NoFocus);   // keep focus on the window so the keyboard keeps working after a click
            grid->addWidget(btn, k.row, k.col, k.rowSpan, k.colSpan);
            const QString key = k.text;
            connect(btn, &QPushButton::clicked, this, [this, key]() { onKey(key); });
        }

        auto *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);
        mainLayout->addWidget(lblExpression);
        mainLayout->addWidget(lblDisplay);
        mainLayout->addLayout(grid, 1);
    }

protected:
    // Keyboard input is translated to the same key labels the buttons use.
    void keyPressEvent(QKeyEvent *event) override {
        switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:     onKey("=");    return;
        case Qt::Key_Backspace: onKey(K_BACK); return;
        case Qt::Key_Delete:
        case Qt::Key_Escape:    onKey("C");    return;
        default: break;
        }
        const QString t = event->text();
        if (t.size() == 1 && t[0].isDigit())                      onKey(t);
        else if (t == "." || t == "+" || t == "=")                onKey(t);
        else if (t == "-")                                        onKey(K_MINUS);
        else if (t == "*" || t == "x" || t == "X")                onKey(K_TIMES);
        else if (t == "/")                                        onKey(K_DIV);
        else QWidget::keyPressEvent(event);
    }

private:
    // ---- widgets ----
    QLabel *lblExpression;   // expression typed so far, e.g. "3 + 5 x"
    QLabel *lblDisplay;      // number being typed, or the result

    // ---- calculator state ----
    QString cur = "0";       // text of the number being typed / the last result
    QVector<double>  nums;   // operands already entered (before an operator)
    QVector<QString> ops;    // operators entered; ops[i] sits between nums[i] and nums[i+1]
    bool    waiting = false; // true: next digit/dot starts a NEW operand
        // (right after an operator or after "=")
    QString expr;            // text shown on the expression line
    QString errorText;       // non-empty: show this message instead of cur

    // ---- helpers ----
    static QString fmt(double v) { return QString::number(v, 'g', 15); }
    double val() const { return cur.toDouble(); }
    static bool isOperator(const QString &k) {
        return k == "+" || k == K_MINUS || k == K_TIMES || k == K_DIV;
    }
    static QString errDivZero() { return QStringLiteral("\u9664\u6570\u4E0D\u80FD\u4E3A\u96F6"); }  // divide by zero

    void refresh() {
        lblDisplay->setText(errorText.isEmpty() ? cur : errorText);
        lblExpression->setText(expr);
    }

    void reset() { cur = "0"; nums.clear(); ops.clear(); waiting = false; expr.clear(); }
    void error(const QString &msg) { reset(); errorText = msg; }

    // Rebuild the expression line from nums/ops, e.g. "3 + 5 x"
    void updateExpr() {
        QStringList parts;
        for (int i = 0; i < ops.size(); ++i) parts << fmt(nums[i]) << ops[i];
        expr = parts.join(" ");
    }

    // If a new operand is expected, start it from "0" (a finished "=" also clears the old expression).
    void beginOperand() {
        if (!waiting) return;
        if (ops.isEmpty()) expr.clear();
        cur = "0";
        waiting = false;
    }

    // Evaluate operands[0] ops[0] operands[1] ... with precedence:
    // first pass handles multiply/divide, second pass handles add/subtract.
    // Returns false on division by zero.
    bool evaluate(const QVector<double> &operands, double &result) const {
        QVector<double> terms;      // values left after multiply/divide
        QVector<QString> addOps;    // the + / - operators between those terms
        terms << operands.first();
        for (int i = 0; i < ops.size(); ++i) {
            const double rhs = operands[i + 1];
            if (ops[i] == K_TIMES) {
                terms.last() *= rhs;
            } else if (ops[i] == K_DIV) {
                if (rhs == 0) return false;
                terms.last() /= rhs;
            } else {
                addOps << ops[i];
                terms << rhs;
            }
        }
        result = terms.first();
        for (int i = 0; i < addOps.size(); ++i)
            result = (addOps[i] == "+") ? result + terms[i + 1] : result - terms[i + 1];
        return true;
    }

    // ---- key handlers ----
    void inputDigit(const QString &d) {
        beginOperand();
        const int digits = cur.length() - (cur.contains('.') ? 1 : 0);
        if (cur == "0")                 cur = d;      // no leading zeros
        else if (digits < MAX_DIGITS)   cur += d;
    }

    void inputDot() {
        beginOperand();
        if (!cur.contains('.')) cur += ".";           // ignore repeated decimal points
    }

    void inputOperator(const QString &op) {
        if (waiting && !ops.isEmpty()) {
            ops.last() = op;                          // two operators in a row: replace the old one
        } else {
            nums << val();                            // also used after "=": continue with the result
            ops << op;
        }
        waiting = true;
        updateExpr();
    }

    void inputEquals() {
        if (ops.isEmpty()) return;                    // nothing to calculate
        QVector<double> operands = nums;
        operands << val();                            // last operand (if an operator was just typed, reuse cur)
        double result = 0;
        if (!evaluate(operands, result)) { error(errDivZero()); return; }
        const QString text = expr + " " + cur + " =";
        cur = fmt(result);
        nums.clear();
        ops.clear();
        expr = text;
        waiting = true;                               // typing a digit now starts a new calculation
    }

    void backspace() {
        if (waiting) {
            if (ops.isEmpty()) return;                // do not edit a result
            ops.removeLast();                         // just typed an operator: take it back
            cur = fmt(nums.takeLast());
            waiting = false;
            updateExpr();
            return;
        }
        cur.chop(1);
        if (cur.isEmpty() || cur == "-") cur = "0";
    }

    // Single entry point for mouse AND keyboard
    void onKey(const QString &k) {
        errorText.clear();                            // any key dismisses an error message
        if (k.size() == 1 && k[0].isDigit())   inputDigit(k);
        else if (k == ".")                     inputDot();
        else if (isOperator(k))                inputOperator(k);
        else if (k == "=")                     inputEquals();
        else if (k == "C")                     reset();
        else if (k == K_BACK)                  backspace();
        refresh();
    }
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Calculator w;
    w.show();
    return a.exec();
}
