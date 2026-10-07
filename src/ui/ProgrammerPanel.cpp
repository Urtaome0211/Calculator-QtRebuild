// ============================================================
// ProgrammerPanel.cpp —— 程序员模式：进制/字长切换 + 位网格 + 位运算键盘
// ============================================================
#include "ProgrammerPanel.h"
#include "DisplayWidget.h"
#include "../core/IntegerEngine.h"
#include "../core/NumberFormat.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace {
// 任意进制整数的格式化串去掉分组空格/逗号，用作表达式数字
QString CleanInteger(quint64 v, unsigned base) {
  QString s = QString::fromStdWString(NumFmt::FormatInteger(v, base));
  s.remove(QLatin1Char(' '));
  s.remove(QLatin1Char(','));
  return s;
}
}  // namespace

ProgrammerPanel::ProgrammerPanel(AppContext& ctx, QWidget* parent)
    : ModePanel(ctx, parent) {
  createWidgets();
}

// ------------------------------------------------------------
// 构建界面：顶行（进制/字长）→ 位网格 → 6×7 主键盘
// ------------------------------------------------------------
void ProgrammerPanel::createWidgets() {
  using K = Theme::Kind;
  auto* root = new QVBoxLayout(this);
  root->setContentsMargins(8, 4, 8, 8);
  root->setSpacing(4);

  // 1) 顶行：8 个 Toggle 小按钮（HEX/DEC/OCT/BIN + QWORD/DWORD/WORD/BYTE）
  topRow_ = new QWidget(this);
  auto* topLayout = new QHBoxLayout(topRow_);
  topLayout->setContentsMargins(0, 0, 0, 0);
  topLayout->setSpacing(4);
  const struct { QString label; int cmd; } topDefs[] = {
      {QStringLiteral("HEX"), IDK_PR_BASE_HEX},
      {QStringLiteral("DEC"), IDK_PR_BASE_DEC},
      {QStringLiteral("OCT"), IDK_PR_BASE_OCT},
      {QStringLiteral("BIN"), IDK_PR_BASE_BIN},
      {QStringLiteral("QWORD"), IDK_PR_WS_QWORD},
      {QStringLiteral("DWORD"), IDK_PR_WS_DWORD},
      {QStringLiteral("WORD"), IDK_PR_WS_WORD},
      {QStringLiteral("BYTE"), IDK_PR_WS_BYTE},
  };
  for (const auto& def : topDefs) {
    auto* b = new CalcButton(def.label, def.cmd, K::Toggle, true, topRow_);
    topLayout->addWidget(b, 1);
    topButtons_.insert(def.cmd, b);
    connect(b, &QPushButton::clicked, this,
            [this, cmd = def.cmd] { onCommand(cmd); });
  }
  root->addWidget(topRow_);

  // 2) 位网格
  bitGrid_ = new BitGrid(this);
  connect(bitGrid_, &BitGrid::bitToggled, this, &ProgrammerPanel::onBitToggled);
  root->addWidget(bitGrid_);

  // 3) 主键盘 6×7：行 1-5 第 6 列留空，直接少放一个 cell
  const QVector<KeyRow> rows = {
      {{{QStringLiteral("("), IDK_PR_LPAR, K::Function},
        {QStringLiteral(")"), IDK_PR_RPAR, K::Function},
        {QStringLiteral("%"), IDK_PR_MOD, K::Function},
        {QStringLiteral("⌫"), IDK_BACK, K::Function},
        {QStringLiteral("CE"), IDK_CE, K::Function},
        {QStringLiteral("C"), IDK_CLEAR, K::Function}}},
      {{{QStringLiteral("A"), IDK_PR_A, K::Number},
        {QStringLiteral("<<"), IDK_PR_LSH, K::Function},
        {QStringLiteral(">>"), IDK_PR_RSH, K::Function},
        {QStringLiteral("mod"), IDK_PR_MOD, K::Function},
        {QStringLiteral("÷"), IDK_DIV, K::Function}}},
      {{{QStringLiteral("B"), IDK_PR_B, K::Number},
        {QStringLiteral("7"), IDK_7, K::Number},
        {QStringLiteral("8"), IDK_8, K::Number},
        {QStringLiteral("9"), IDK_9, K::Number},
        {QStringLiteral("×"), IDK_MUL, K::Function}}},
      {{{QStringLiteral("C"), IDK_PR_C, K::Number},
        {QStringLiteral("4"), IDK_4, K::Number},
        {QStringLiteral("5"), IDK_5, K::Number},
        {QStringLiteral("6"), IDK_6, K::Number},
        {QStringLiteral("−"), IDK_SUB, K::Function}}},
      {{{QStringLiteral("D"), IDK_PR_D, K::Number},
        {QStringLiteral("1"), IDK_1, K::Number},
        {QStringLiteral("2"), IDK_2, K::Number},
        {QStringLiteral("3"), IDK_3, K::Number},
        {QStringLiteral("+"), IDK_ADD, K::Function}}},
      {{{QStringLiteral("E"), IDK_PR_E, K::Number},
        {QStringLiteral("~"), IDK_PR_NOT, K::Function},
        {QStringLiteral("±"), IDK_SIGN, K::Function},
        {QStringLiteral("0"), IDK_0, K::Number},
        {QStringLiteral("="), IDK_EQU, K::Accent}}},
      {{{QStringLiteral("F"), IDK_PR_F, K::Number},
        {QStringLiteral("AND"), IDK_PR_AND, K::Function},
        {QStringLiteral("OR"), IDK_PR_OR, K::Function},
        {QStringLiteral("XOR"), IDK_PR_XOR, K::Function},
        {QStringLiteral("NOT"), IDK_PR_NOT, K::Function},
        {QStringLiteral("NAND"), IDK_PR_NAND, K::Function}}},
  };
  pad_ = new Keypad(6, this);
  pad_->build(rows, 4, 4);
  root->addWidget(pad_, 1);

  // 连接所有按钮：IDK_PR_NOT 被 “~” 与 “NOT” 两个按钮共用，
  // Keypad 内部 QHash 按 cmd 去重，故遍历网格布局逐项连接，确保两者都生效。
  for (int i = 0; i < pad_->grid()->count(); ++i) {
    if (auto* b = qobject_cast<CalcButton*>(pad_->grid()->itemAt(i)->widget())) {
      connect(b, &QPushButton::clicked, this, [this, b] { onCommand(b->cmd()); });
    }
  }

  setBase(16);
  setBits(64);
}

// ------------------------------------------------------------
// 命令处理
// ------------------------------------------------------------
bool ProgrammerPanel::onCommand(int cmd) {
  switch (cmd) {
    case IDK_PR_BASE_HEX: setBase(16); return true;
    case IDK_PR_BASE_DEC: setBase(10); return true;
    case IDK_PR_BASE_OCT: setBase(8); return true;
    case IDK_PR_BASE_BIN: setBase(2); return true;
    case IDK_PR_WS_QWORD: setBits(64); return true;
    case IDK_PR_WS_DWORD: setBits(32); return true;
    case IDK_PR_WS_WORD: setBits(16); return true;
    case IDK_PR_WS_BYTE: setBits(8); return true;
    case IDK_0: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("0")); return true;
    case IDK_1: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("1")); return true;
    case IDK_2: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("2")); return true;
    case IDK_3: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("3")); return true;
    case IDK_4: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("4")); return true;
    case IDK_5: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("5")); return true;
    case IDK_6: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("6")); return true;
    case IDK_7: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("7")); return true;
    case IDK_8: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("8")); return true;
    case IDK_9: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("9")); return true;
    case IDK_PR_A: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("A")); return true;
    case IDK_PR_B: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("B")); return true;
    case IDK_PR_C: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("C")); return true;
    case IDK_PR_D: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("D")); return true;
    case IDK_PR_E: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("E")); return true;
    case IDK_PR_F: if (justEval_) { expr_.clear(); justEval_ = false; } insert(QStringLiteral("F")); return true;
    case IDK_ADD:
      if (justEval_) { expr_ = CleanInteger(value_, base_); justEval_ = false; }
      insert(QStringLiteral("+")); return true;
    case IDK_SUB:
      if (justEval_) { expr_ = CleanInteger(value_, base_); justEval_ = false; }
      insert(QStringLiteral("−")); return true;
    case IDK_MUL:
      if (justEval_) { expr_ = CleanInteger(value_, base_); justEval_ = false; }
      insert(QStringLiteral("×")); return true;
    case IDK_DIV:
      if (justEval_) { expr_ = CleanInteger(value_, base_); justEval_ = false; }
      insert(QStringLiteral("÷")); return true;
    case IDK_PR_MOD:
      if (justEval_) { expr_ = CleanInteger(value_, base_); justEval_ = false; }
      insert(QStringLiteral("%")); return true;
    case IDK_PR_LSH:
      if (justEval_) { expr_ = CleanInteger(value_, base_); justEval_ = false; }
      insert(QStringLiteral("<<")); return true;
    case IDK_PR_RSH:
      if (justEval_) { expr_ = CleanInteger(value_, base_); justEval_ = false; }
      insert(QStringLiteral(">>")); return true;
    case IDK_PR_AND:
      if (justEval_) { expr_ = CleanInteger(value_, base_); justEval_ = false; }
      insert(QStringLiteral("&")); return true;
    case IDK_PR_OR:
      if (justEval_) { expr_ = CleanInteger(value_, base_); justEval_ = false; }
      insert(QStringLiteral("|")); return true;
    case IDK_PR_XOR:
      if (justEval_) { expr_ = CleanInteger(value_, base_); justEval_ = false; }
      insert(QStringLiteral("^")); return true;
    case IDK_PR_NOT:
      if (justEval_) { expr_ = CleanInteger(value_, base_); justEval_ = false; }
      insert(QStringLiteral("~")); return true;
    case IDK_PR_NAND:
      if (justEval_) { expr_ = CleanInteger(value_, base_); justEval_ = false; }
      insert(QStringLiteral("~&")); return true;
    case IDK_PR_LPAR:
      if (justEval_) { expr_.clear(); justEval_ = false; }
      insert(QStringLiteral("(")); return true;
    case IDK_PR_RPAR: insert(QStringLiteral(")")); return true;
    case IDK_SIGN: {
      // ± ：二进制补码取反（-x = ~x + 1，按字长掩码）
      quint64 v = 0;
      std::wstring err;
      if (!IntEng::Evaluate(expr_.isEmpty() ? std::wstring(L"0") : expr_.toStdWString(),
                            base_, bits_, v, err)) {
        return true;
      }
      const quint64 neg = (0ull - v) & IntEng::Mask(bits_);
      expr_ = CleanInteger(neg, base_);
      justEval_ = false;
      updateDisplay();
      return true;
    }
    case IDK_EQU: eval(); return true;
    case IDK_CLEAR:
      expr_.clear();
      justEval_ = false;
      ctx_.display->setExpr(QString());
      ctx_.display->setResult(QString(), false);
      return true;
    case IDK_CE:
      expr_.clear();
      updateDisplay();
      return true;
    case IDK_BACK: {
      std::wstring out;
      if (IntEng::BackspaceExpr(expr_.toStdWString(), out)) {
        expr_ = QString::fromStdWString(out);
        justEval_ = false;
        updateDisplay();
      }
      return true;
    }
  }
  return false;
}

// ------------------------------------------------------------
// 智能插入 token：交给 IntEng::InsertToken 校验并更新表达式
// ------------------------------------------------------------
void ProgrammerPanel::insert(const QString& token) {
  std::wstring out;
  if (IntEng::InsertToken(expr_.toStdWString(), token.toStdWString(), base_, out)) {
    expr_ = QString::fromStdWString(out);
    justEval_ = false;
    updateDisplay();
  }
}

// ------------------------------------------------------------
// 刷新显示：表达式 / 结果 / 位网格 / 四种进制明细
// ------------------------------------------------------------
void ProgrammerPanel::updateDisplay() {
  ctx_.display->setExpr(expr_);
  quint64 v = 0;
  std::wstring err;
  if (IntEng::Evaluate(expr_.toStdWString(), base_, bits_, v, err)) {
    value_ = v;
    hasValue_ = true;
    bitGrid_->setValue(v);
    ctx_.display->setResult(
        QString::fromStdWString(NumFmt::FormatInteger(v, base_)), false);
  } else {
    if (hasValue_) bitGrid_->setValue(value_);  // 失败时位网格保持上次值
    ctx_.display->setResult(QString(), false);
  }
  // 四种进制明细
  QStringList lines;
  lines << QStringLiteral("DEC  ") +
               QString::fromStdWString(NumFmt::FormatSigned(value_, bits_));
  lines << QStringLiteral("HEX  ") +
               QString::fromStdWString(NumFmt::FormatInteger(value_, 16));
  lines << QStringLiteral("OCT  ") +
               QString::fromStdWString(NumFmt::FormatInteger(value_, 8));
  lines << QStringLiteral("BIN  ") +
               QString::fromStdWString(NumFmt::FormatInteger(value_, 2));
  ctx_.display->setAltLines(lines);
}

// ------------------------------------------------------------
// 求值并写入历史
// ------------------------------------------------------------
void ProgrammerPanel::eval() {
  quint64 v = 0;
  std::wstring err;
  if (IntEng::Evaluate(expr_.toStdWString(), base_, bits_, v, err)) {
    value_ = v;
    hasValue_ = true;
    bitGrid_->setValue(v);
    justEval_ = true;
    ctx_.display->setResult(
        QString::fromStdWString(NumFmt::FormatInteger(v, base_)), false);
    if (ctx_.addHistory) {
      ctx_.addHistory(expr_.toStdWString(), NumFmt::FormatInteger(v, base_));
    }
  } else if (!err.empty()) {
    ctx_.display->setResult(QString::fromStdWString(err), true);
  }
  updateDisplay();
}

// ------------------------------------------------------------
// 进制 / 字长切换
// ------------------------------------------------------------
void ProgrammerPanel::setBase(unsigned base) {
  base_ = base;
  applyBaseEnabled();
  auto check = [this](int cmd, bool on) {
    if (CalcButton* b = topButtons_.value(cmd)) b->setChecked(on);
  };
  check(IDK_PR_BASE_HEX, base == 16);
  check(IDK_PR_BASE_DEC, base == 10);
  check(IDK_PR_BASE_OCT, base == 8);
  check(IDK_PR_BASE_BIN, base == 2);
  updateDisplay();
}

void ProgrammerPanel::setBits(unsigned bits) {
  bits_ = bits;
  bitGrid_->setBits(bits);
  auto check = [this](int cmd, bool on) {
    if (CalcButton* b = topButtons_.value(cmd)) b->setChecked(on);
  };
  check(IDK_PR_WS_QWORD, bits == 64);
  check(IDK_PR_WS_DWORD, bits == 32);
  check(IDK_PR_WS_WORD, bits == 16);
  check(IDK_PR_WS_BYTE, bits == 8);
  updateDisplay();
}

// ------------------------------------------------------------
// 按进制禁用不合法的数字键
// ------------------------------------------------------------
void ProgrammerPanel::applyBaseEnabled() {
  pad_->setEnabled(IDK_PR_A, base_ == 16);
  pad_->setEnabled(IDK_PR_B, base_ == 16);
  pad_->setEnabled(IDK_PR_C, base_ == 16);
  pad_->setEnabled(IDK_PR_D, base_ == 16);
  pad_->setEnabled(IDK_PR_E, base_ == 16);
  pad_->setEnabled(IDK_PR_F, base_ == 16);
  pad_->setEnabled(IDK_8, base_ > 8);
  pad_->setEnabled(IDK_9, base_ > 8);
  pad_->setEnabled(IDK_2, base_ > 2);
  pad_->setEnabled(IDK_3, base_ > 2);
  pad_->setEnabled(IDK_4, base_ > 2);
  pad_->setEnabled(IDK_5, base_ > 2);
  pad_->setEnabled(IDK_6, base_ > 2);
  pad_->setEnabled(IDK_7, base_ > 2);
}

// ------------------------------------------------------------
// 位翻转回调：同步表达式与显示
// ------------------------------------------------------------
void ProgrammerPanel::onBitToggled(quint64 newValue) {
  value_ = newValue;
  hasValue_ = true;
  expr_ = CleanInteger(value_, base_);
  justEval_ = false;
  updateDisplay();
}

// ------------------------------------------------------------
// 外部载入数值：去分隔符并校验当前进制合法性
// ------------------------------------------------------------
void ProgrammerPanel::loadValue(const QString& v) {
  QString clean = v;
  clean.remove(QLatin1Char(','));
  clean.remove(QLatin1Char(' '));
  if (clean.isEmpty()) return;
  for (const QChar& ch : clean) {
    int d = -1;
    if (ch >= QLatin1Char('0') && ch <= QLatin1Char('9')) {
      d = ch.unicode() - '0';
    } else {
      const QChar lower = ch.toLower();
      if (lower >= QLatin1Char('a') && lower <= QLatin1Char('f')) {
        d = lower.unicode() - 'a' + 10;
      }
    }
    if (d < 0 || d >= int(base_)) return;
  }
  expr_ = clean;
  justEval_ = false;
  updateDisplay();
}

// ------------------------------------------------------------
// 键盘输入
// ------------------------------------------------------------
bool ProgrammerPanel::handleChar(const QString& c) {
  if (c.size() != 1) return false;
  const QChar ch = c[0];
  if (ch.isDigit()) {
    const int d = ch.digitValue();
    if (d < 0 || d >= int(base_)) return false;  // 超出进制范围
    if (justEval_) { expr_.clear(); justEval_ = false; }
    insert(QString(ch));
    return true;
  }
  if ((ch >= QLatin1Char('a') && ch <= QLatin1Char('f')) ||
      (ch >= QLatin1Char('A') && ch <= QLatin1Char('F'))) {
    if (base_ != 16) return false;
    if (justEval_) { expr_.clear(); justEval_ = false; }
    insert(QString(ch.toUpper()));
    return true;
  }
  switch (ch.unicode()) {
    case '+': insert(QStringLiteral("+")); return true;
    case '-': insert(QStringLiteral("−")); return true;
    case '*': insert(QStringLiteral("×")); return true;
    case '/': insert(QStringLiteral("÷")); return true;
    case '%': insert(QStringLiteral("%")); return true;
    case '(': insert(QStringLiteral("(")); return true;
    case ')': insert(QStringLiteral(")")); return true;
    case '&': insert(QStringLiteral("&")); return true;
    case '|': insert(QStringLiteral("|")); return true;
    case '^': insert(QStringLiteral("^")); return true;
    case '~': insert(QStringLiteral("~")); return true;
    case '<': insert(QStringLiteral("<")); return true;  // InsertToken 合并成 <<
    case '>': insert(QStringLiteral(">")); return true;  // InsertToken 合并成 >>
  }
  return false;
}

bool ProgrammerPanel::handleKey(int key) {
  switch (key) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
      eval();
      return true;
    case Qt::Key_Escape:
      expr_.clear();
      justEval_ = false;
      ctx_.display->setExpr(QString());
      ctx_.display->setResult(QString(), false);
      return true;
    case Qt::Key_Backspace: {
      std::wstring out;
      if (IntEng::BackspaceExpr(expr_.toStdWString(), out)) {
        expr_ = QString::fromStdWString(out);
        justEval_ = false;
        updateDisplay();
      }
      return true;
    }
  }
  return false;
}

// ------------------------------------------------------------
// 面板显示时刷新
// ------------------------------------------------------------
void ProgrammerPanel::onShow() { updateDisplay(); }
