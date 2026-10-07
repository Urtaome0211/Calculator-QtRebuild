#include "ScientificPanel.h"
// ============================================================
// 科学模式面板：8 列 × 6 行主键盘 + 三角侧栏 + 2nd 切换 + 角度制切换
// 说明：记忆条按钮（含 DEG/RAD/GRAD）由基类直接建在 QHBoxLayout 的
// CalcButton 行中，未包装为 Keypad，因此通过 findChildren 统一刷新勾选态。
// ============================================================

ScientificPanel::ScientificPanel(AppContext& ctx, QWidget* parent)
    : KeypadPanel(ctx, parent) {
  setupPanel();   // 两阶段初始化（构造体末尾调用，虚派发此时生效）
}

// ---------- 记忆条：MC/MR/M+/M-/MS + 角度制切换（共 8 格） ----------
QVector<KeyRow> ScientificPanel::memoryRow() const {
  using K = Theme::Kind;
  return {{{
      {QStringLiteral("MC"),   IDK_MC,      K::Memory, 1, 1, true},
      {QStringLiteral("MR"),   IDK_MR,      K::Memory, 1, 1, true},
      {QStringLiteral("M+"),   IDK_MP,      K::Memory, 1, 1, true},
      {QStringLiteral("M-"),   IDK_MM,      K::Memory, 1, 1, true},
      {QStringLiteral("MS"),   IDK_MS,      K::Memory, 1, 1, true},
      {QStringLiteral("DEG"),  IDK_SC_DEG,  K::Toggle, 1, 1, true},
      {QStringLiteral("RAD"),  IDK_SC_RAD,  K::Toggle, 1, 1, true},
      {QStringLiteral("GRAD"), IDK_SC_GRAD, K::Toggle, 1, 1, true},
  }}};
}

// ---------- 主键盘：8 列 × 6 行 ----------
// 行 0：2nd π e C ⌫ ( ) + sin(跨 2 行)
// 行 1：x² 1/x |x| exp mod % ÷（第 8 列被 sin 占用）
// 行 2：√x xʸ x√y 7 8 9 × + cos(跨 2 行)
// 行 3：10ˣ log logy 4 5 6 −（第 8 列被 cos 占用）
// 行 4：ln 2ˣ n! 1 2 3 + + tan(跨 2 行)
// 行 5：rand eˣ ± 0 . =（第 8 列被 tan 占用）
// KeyCell 聚合顺序：{label, id, kind, colSpan, rowSpan, small}
QVector<KeyRow> ScientificPanel::buildLayout() const {
  using K = Theme::Kind;
  const K Fn = K::Function;

  QVector<KeyRow> rows;
  rows.push_back(KeyRow{{
    {QStringLiteral("2nd"), IDK_SC_2ND,  K::Toggle},
    {QStringLiteral("π"),   IDK_SC_PI,   Fn},
    {QStringLiteral("e"),   IDK_SC_E,    Fn},
    {QStringLiteral("C"),   IDK_CLEAR,   Fn},
    {QStringLiteral("⌫"),   IDK_BACK,    Fn},
    {QStringLiteral("("),   IDK_SC_LPAR, Fn},
    {QStringLiteral(")"),   IDK_SC_RPAR, Fn},
    {QStringLiteral("sin"), IDK_SC_SIN,  Fn, 1, 2},
  }});
  rows.push_back(KeyRow{{
    {QStringLiteral("x²"),  IDK_SQ,      Fn},
    {QStringLiteral("1/x"), IDK_RECIP,   Fn},
    {QStringLiteral("|x|"), IDK_SC_ABS,  Fn},
    {QStringLiteral("exp"), IDK_SC_EPOW, Fn},
    {QStringLiteral("mod"), IDK_SC_MOD,  Fn},
    {QStringLiteral("%"),   IDK_PERCENT, Fn},
    {QStringLiteral("÷"),   IDK_DIV,     Fn},
  }});
  rows.push_back(KeyRow{{
    {QStringLiteral("√x"),  IDK_SQRT,    Fn},
    {QStringLiteral("xʸ"),  IDK_SC_POW,  Fn},
    {QStringLiteral("x√y"), IDK_SC_ROOTY, Fn},
    {QStringLiteral("7"),   IDK_7,       K::Number},
    {QStringLiteral("8"),   IDK_8,       K::Number},
    {QStringLiteral("9"),   IDK_9,       K::Number},
    {QStringLiteral("×"),   IDK_MUL,     Fn},
    {QStringLiteral("cos"), IDK_SC_COS,  Fn, 1, 2},
  }});
  rows.push_back(KeyRow{{
    {QStringLiteral("10ˣ"), IDK_SC_POW10, Fn},
    {QStringLiteral("log"), IDK_SC_LOG,  Fn},
    {QStringLiteral("logy"),IDK_SC_LOGB, Fn},
    {QStringLiteral("4"),   IDK_4,       K::Number},
    {QStringLiteral("5"),   IDK_5,       K::Number},
    {QStringLiteral("6"),   IDK_6,       K::Number},
    {QStringLiteral("−"),   IDK_SUB,     Fn},
  }});
  rows.push_back(KeyRow{{
    {QStringLiteral("ln"),  IDK_SC_LN,   Fn},
    {QStringLiteral("2ˣ"),  IDK_SC_2POW, Fn},
    {QStringLiteral("n!"),  IDK_SC_FACT, Fn},
    {QStringLiteral("1"),   IDK_1,       K::Number},
    {QStringLiteral("2"),   IDK_2,       K::Number},
    {QStringLiteral("3"),   IDK_3,       K::Number},
    {QStringLiteral("+"),   IDK_ADD,     Fn},
    {QStringLiteral("tan"), IDK_SC_TAN,  Fn, 1, 2},
  }});
  rows.push_back(KeyRow{{
    {QStringLiteral("rand"), IDK_SC_RAND, Fn},
    {QStringLiteral("eˣ"),   IDK_CE,      Fn},   // 复用 IDK_CE 作为 eˣ 按钮 ID
    {QStringLiteral("±"),    IDK_SIGN,    Fn},
    {QStringLiteral("0"),    IDK_0,       K::Number},
    {QStringLiteral("."),    IDK_DOT,     K::Number},
    {QStringLiteral("="),    IDK_EQU,     K::Accent},
  }});

  return rows;
}

// ---------- 命令处理：科学按键 + 2nd 分支，其余交基类 ----------
bool ScientificPanel::onCommand(int cmd) {
  switch (cmd) {
    case IDK_SC_2ND:                       // 切换第二功能
      second_ = !second_;
      applySecond();
      pad_->setChecked(IDK_SC_2ND, second_);
      return true;
    case IDK_SC_PI:   insert(QStringLiteral("π"));  return true;
    case IDK_SC_E:    insert(QStringLiteral("e"));  return true;
    case IDK_SC_FACT: insert(QStringLiteral("!"));  return true;
    case IDK_SC_POW:  insert(QStringLiteral("^"));  return true;
    case IDK_SC_LPAR: insert(QStringLiteral("("));  return true;
    case IDK_SC_RPAR: insert(QStringLiteral(")"));  return true;
    case IDK_SC_SIN:   pressFunc(second_ ? QStringLiteral("asin") : QStringLiteral("sin")); return true;
    case IDK_SC_COS:   pressFunc(second_ ? QStringLiteral("acos") : QStringLiteral("cos")); return true;
    case IDK_SC_TAN:   pressFunc(second_ ? QStringLiteral("atan") : QStringLiteral("tan")); return true;
    case IDK_SC_ABS:   pressFunc(QStringLiteral("abs"));   return true;
    case IDK_SC_MOD:   pressFunc(QStringLiteral("mod"));   return true;
    case IDK_SC_EPOW:  pressFunc(QStringLiteral("exp"));   return true;
    case IDK_SC_POW10: pressFunc(second_ ? QStringLiteral("pow2") : QStringLiteral("pow10")); return true;
    case IDK_SC_2POW:  pressFunc(QStringLiteral("pow2"));  return true;
    case IDK_SC_LOG:   pressFunc(second_ ? QStringLiteral("logy") : QStringLiteral("log")); return true;
    case IDK_SC_LN:    pressFunc(second_ ? QStringLiteral("exp") : QStringLiteral("ln")); return true;
    case IDK_SC_ROOTY: pressFunc(QStringLiteral("xroot")); return true;
    case IDK_SC_LOGB:  pressFunc(QStringLiteral("logy"));  return true;
    case IDK_SC_RAND:  pressFunc(QStringLiteral("rand"));  return true;
    case IDK_CE:       pressFunc(QStringLiteral("exp"));   return true;  // eˣ 按钮
    case IDK_SC_DEG:
      angle_ = Expr::AngleMode::Deg;
      updateDisplay();
      refreshAngleChecks();
      return true;
    case IDK_SC_RAD:
      angle_ = Expr::AngleMode::Rad;
      updateDisplay();
      refreshAngleChecks();
      return true;
    case IDK_SC_GRAD:
      angle_ = Expr::AngleMode::Grad;
      updateDisplay();
      refreshAngleChecks();
      return true;
    default:
      return KeypadPanel::onCommand(cmd);  // 数字/四则/记忆等共享命令
  }
}

// 插入函数 token（引擎自动补隐式乘法与右括号）
void ScientificPanel::pressFunc(const QString& name) {
  insert(name + QStringLiteral("("));
}

// ---------- 2nd 模式按钮文字切换（7 组） ----------
void ScientificPanel::applySecond() {
  auto label = [this](int id, const QString& on, const QString& off) {
    if (CalcButton* b = pad_->get(id)) b->setText(second_ ? on : off);
  };
  label(IDK_SC_SIN,   QStringLiteral("sin⁻¹"), QStringLiteral("sin"));
  label(IDK_SC_COS,   QStringLiteral("cos⁻¹"), QStringLiteral("cos"));
  label(IDK_SC_TAN,   QStringLiteral("tan⁻¹"), QStringLiteral("tan"));
  label(IDK_SC_POW,   QStringLiteral("x√y"),   QStringLiteral("xʸ"));
  label(IDK_SC_POW10, QStringLiteral("2ˣ"),    QStringLiteral("10ˣ"));
  label(IDK_SC_LOG,   QStringLiteral("logy"),  QStringLiteral("log"));
  label(IDK_SC_LN,    QStringLiteral("eˣ"),    QStringLiteral("ln"));
  pad_->setChecked(IDK_SC_2ND, second_);
}

// ---------- 刷新记忆条上 DEG/RAD/GRAD 的选中态 ----------
// 记忆条按钮是基类直接创建的 CalcButton（无 Keypad 包装且无句柄），
// 只能通过 findChildren 查找后按 cmd 匹配勾选。
void ScientificPanel::refreshAngleChecks() {
  const auto buttons = findChildren<CalcButton*>();
  for (CalcButton* b : buttons) {
    switch (b->cmd()) {
      case IDK_SC_DEG:  b->setChecked(angle_ == Expr::AngleMode::Deg);  break;
      case IDK_SC_RAD:  b->setChecked(angle_ == Expr::AngleMode::Rad);  break;
      case IDK_SC_GRAD: b->setChecked(angle_ == Expr::AngleMode::Grad); break;
      default: break;
    }
  }
}

// ---------- 切到科学模式：刷新显示 + 恢复 2nd/角度制按钮状态 ----------
void ScientificPanel::onShow() {
  KeypadPanel::onShow();          // 基类刷新表达式与结果
  applySecond();
  refreshAngleChecks();
}

// ---------- 快捷字符输入：p/P → π，e → e，^ → ^，! → ! ----------
bool ScientificPanel::handleChar(const QString& c) {
  if (c.size() == 1) {
    switch (c[0].unicode()) {
      case 'p': case 'P': insert(QStringLiteral("π")); return true;
      case 'e':            insert(QStringLiteral("e")); return true;  // 科学模式无程序员 A-F 输入
      case '^':            insert(QStringLiteral("^")); return true;
      case '!':            insert(QStringLiteral("!")); return true;
      default: break;
    }
  }
  return KeypadPanel::handleChar(c);  // 数字/+-*/./(/)/,/%
}
