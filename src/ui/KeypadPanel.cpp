#include "KeypadPanel.h"
#include "CalcButton.h"
#include "DisplayWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

// ============================================================
// 键盘计算面板基类（Qt 版）：记忆条 + 主键盘 + 表达式状态机
// ============================================================
KeypadPanel::KeypadPanel(AppContext& ctx, QWidget* parent)
    : ModePanel(ctx, parent) {
  // 注意：不要在这里调用虚函数（构造函数中虚派发不生效）。
  // 界面构建由 setupPanel() 完成，子类构造函数体末尾必须调用它。
}

void KeypadPanel::setupPanel() {
  auto* root = new QVBoxLayout(this);
  root->setContentsMargins(8, 4, 8, 8);
  root->setSpacing(4);

  // 记忆条
  const auto memCells = memoryRow();
  if (!memCells.isEmpty()) {
    memRow_ = new QWidget(this);
    auto* h = new QHBoxLayout(memRow_);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(4);
    for (const auto& row : memCells) {
      for (const KeyCell& cell : row.cells) {
        if (cell.cmd == 0) continue;
        auto* b = new CalcButton(cell.label, cell.cmd, cell.kind, true, memRow_);
        h->addWidget(b, 1);
        connect(b, &QPushButton::clicked, this,
                [this, cell] { onCommand(cell.cmd); });
      }
    }
    root->addWidget(memRow_);
  }

  // 主键盘
  const auto rows = buildLayout();
  int columns = 1;
  for (const auto& row : rows) {
    int c = 0;
    for (const KeyCell& cell : row.cells) c += std::max(1, cell.colSpan);
    columns = std::max(columns, c);
  }
  pad_ = new Keypad(columns, this);
  pad_->build(rows);
  root->addWidget(pad_, 1);
  for (CalcButton* b : pad_->allButtons()) {
    connect(b, &QPushButton::clicked, this, [this, b] { onCommand(b->cmd()); });
  }
}

void KeypadPanel::onShow() { updateDisplay(); }

void KeypadPanel::loadValue(const QString& v) {
  QString clean = v;
  clean.remove(QLatin1Char(','));
  clean.remove(QLatin1Char(' '));
  if (clean.isEmpty()) return;
  expr_ = clean;
  justEval_ = false;
  updateDisplay();
}

void KeypadPanel::insert(const QString& token) {
  std::wstring out;
  if (Expr::InsertToken(expr_.toStdWString(), token.toStdWString(), out)) {
    expr_ = QString::fromStdWString(out);
    updateDisplay();
  }
}

void KeypadPanel::updateDisplay() {
  ctx_.display->setExpr(QString::fromStdWString(NumFmt::GroupExpr(expr_.toStdWString())));
  double v;
  if (evalNow(v)) {
    ctx_.display->setResult(QString::fromStdWString(NumFmt::FormatDouble(v)), false);
  } else {
    ctx_.display->setResult(QString(), false);
  }
}

bool KeypadPanel::evalNow(double& v) {
  std::wstring err;
  return Expr::Evaluate(expr_.toStdWString(), v, err, angle());
}

void KeypadPanel::setResultText(double v) {
  lastResult_ = v;
  ctx_.display->setResult(QString::fromStdWString(NumFmt::FormatDouble(v)), false);
}

void KeypadPanel::pushHistory() {
  if (ctx_.addHistory) {
    ctx_.addHistory(NumFmt::GroupExpr(expr_.toStdWString()),
                    NumFmt::FormatDouble(lastResult_));
  }
}

void KeypadPanel::pressDigit(const QString& d) {
  if (justEval_) {
    expr_.clear();
    justEval_ = false;
  }
  insert(d);
}

void KeypadPanel::pressOp(const QString& sym) {
  if (justEval_) {
    expr_ = QString::fromStdWString(NumFmt::FormatRaw(lastResult_));
    justEval_ = false;
  }
  if (expr_.isEmpty()) expr_ = QStringLiteral("0");
  insert(sym);
}

void KeypadPanel::pressEquals() {
  double v;
  std::wstring err;
  if (Expr::Evaluate(expr_.toStdWString(), v, err, angle())) {
    setResultText(v);
    justEval_ = true;
    pushHistory();
  } else if (!err.empty()) {
    ctx_.display->setResult(QString::fromStdWString(err), true);
  }
}

void KeypadPanel::applyUnary(const QString& prefix, const QString& suffix) {
  double v;
  if (!evalNow(v)) return;
  const QString raw = QString::fromStdWString(NumFmt::FormatRaw(v));
  expr_ = prefix + raw + suffix;
  justEval_ = false;
  updateDisplay();
}

QVector<KeyRow> KeypadPanel::memoryRow() const {
  using K = Theme::Kind;
  return {{{
      {QStringLiteral("MC"), IDK_MC, K::Memory, 1, 1, true},
      {QStringLiteral("MR"), IDK_MR, K::Memory, 1, 1, true},
      {QStringLiteral("M+"), IDK_MP, K::Memory, 1, 1, true},
      {QStringLiteral("M−"), IDK_MM, K::Memory, 1, 1, true},
      {QStringLiteral("MS"), IDK_MS, K::Memory, 1, 1, true},
  }}};
}

bool KeypadPanel::onCommand(int cmd) {
  switch (cmd) {
    case IDK_0: pressDigit(QStringLiteral("0")); return true;
    case IDK_1: pressDigit(QStringLiteral("1")); return true;
    case IDK_2: pressDigit(QStringLiteral("2")); return true;
    case IDK_3: pressDigit(QStringLiteral("3")); return true;
    case IDK_4: pressDigit(QStringLiteral("4")); return true;
    case IDK_5: pressDigit(QStringLiteral("5")); return true;
    case IDK_6: pressDigit(QStringLiteral("6")); return true;
    case IDK_7: pressDigit(QStringLiteral("7")); return true;
    case IDK_8: pressDigit(QStringLiteral("8")); return true;
    case IDK_9: pressDigit(QStringLiteral("9")); return true;
    case IDK_DOT: pressDigit(QStringLiteral(".")); return true;
    case IDK_ADD: pressOp(QStringLiteral("+")); return true;
    case IDK_SUB: pressOp(QStringLiteral("−")); return true;
    case IDK_MUL: pressOp(QStringLiteral("×")); return true;
    case IDK_DIV: pressOp(QStringLiteral("÷")); return true;
    case IDK_EQU: pressEquals(); return true;
    case IDK_CLEAR:
      expr_.clear();
      justEval_ = false;
      ctx_.display->setExpr(QString());
      ctx_.display->setResult(QString(), false);
      return true;
    case IDK_CE: {
      std::wstring out;
      if (Expr::ClearEntry(expr_.toStdWString(), out)) {
        expr_ = QString::fromStdWString(out);
        updateDisplay();
      }
      return true;
    }
    case IDK_BACK: {
      std::wstring out;
      if (Expr::BackspaceExpr(expr_.toStdWString(), out)) {
        expr_ = QString::fromStdWString(out);
        justEval_ = false;
        updateDisplay();
      }
      return true;
    }
    case IDK_PERCENT: {
      std::wstring out;
      if (Expr::ApplyPercent(expr_.toStdWString(), out, angle())) {
        expr_ = QString::fromStdWString(out);
        updateDisplay();
      }
      return true;
    }
    case IDK_SIGN: {
      std::wstring out;
      if (Expr::NegateEntry(expr_.toStdWString(), out)) {
        expr_ = QString::fromStdWString(out);
        justEval_ = false;
        updateDisplay();
      }
      return true;
    }
    case IDK_RECIP: applyUnary(QStringLiteral("1/("), QStringLiteral(")")); return true;
    case IDK_SQ: applyUnary(QStringLiteral("sqr("), QStringLiteral(")")); return true;
    case IDK_SQRT: applyUnary(QStringLiteral("sqrt("), QStringLiteral(")")); return true;
    case IDK_MC:
      if (ctx_.memory) {
        *ctx_.memory = 0;
        if (ctx_.refreshMemory) ctx_.refreshMemory();
      }
      return true;
    case IDK_MR:
      if (ctx_.memory) {
        if (justEval_) {
          expr_.clear();
          justEval_ = false;
        }
        insert(QString::fromStdWString(NumFmt::FormatRaw(*ctx_.memory)));
      }
      return true;
    case IDK_MP:
    case IDK_MM:
    case IDK_MS: {
      if (!ctx_.memory) return true;
      double v;
      if (!evalNow(v)) return true;
      if (cmd == IDK_MP) *ctx_.memory += v;
      else if (cmd == IDK_MM) *ctx_.memory -= v;
      else *ctx_.memory = v;
      if (ctx_.refreshMemory) ctx_.refreshMemory();
      return true;
    }
  }
  return false;
}

bool KeypadPanel::handleChar(const QString& c) {
  if (c.size() != 1) return false;
  const QChar ch = c[0];
  if (ch.isDigit()) { pressDigit(c); return true; }
  switch (ch.unicode()) {
    case '.':
      pressDigit(QStringLiteral("."));
      return true;
    case '+':
      pressOp(QStringLiteral("+"));
      return true;
    case '-':
      pressOp(QStringLiteral("−"));
      return true;
    case '*':
      pressOp(QStringLiteral("×"));
      return true;
    case '/':
      pressOp(QStringLiteral("÷"));
      return true;
    case '%': {
      std::wstring out;
      if (Expr::ApplyPercent(expr_.toStdWString(), out, angle())) {
        expr_ = QString::fromStdWString(out);
        updateDisplay();
      }
      return true;
    }
    case '(':
      insert(QStringLiteral("("));
      return true;
    case ')':
      insert(QStringLiteral(")"));
      return true;
    case ',':
      insert(QStringLiteral(","));
      return true;
  }
  return false;
}

bool KeypadPanel::handleKey(int key) {
  switch (key) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
      pressEquals();
      return true;
    case Qt::Key_Backspace: {
      std::wstring out;
      if (Expr::BackspaceExpr(expr_.toStdWString(), out)) {
        expr_ = QString::fromStdWString(out);
        justEval_ = false;
        updateDisplay();
      }
      return true;
    }
    case Qt::Key_Escape:
      expr_.clear();
      justEval_ = false;
      ctx_.display->setExpr(QString());
      ctx_.display->setResult(QString(), false);
      return true;
    case Qt::Key_Delete: {
      std::wstring out;
      if (Expr::ClearEntry(expr_.toStdWString(), out)) {
        expr_ = QString::fromStdWString(out);
        updateDisplay();
      }
      return true;
    }
    case Qt::Key_F9: {
      std::wstring out;
      if (Expr::NegateEntry(expr_.toStdWString(), out)) {
        expr_ = QString::fromStdWString(out);
        justEval_ = false;
        updateDisplay();
      }
      return true;
    }
  }
  return false;
}
