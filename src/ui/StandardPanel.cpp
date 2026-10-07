#include "StandardPanel.h"

// ============================================================
// 标准模式：4×6 数字键盘
// ============================================================
StandardPanel::StandardPanel(AppContext& ctx, QWidget* parent)
    : KeypadPanel(ctx, parent) {
  setupPanel();   // 两阶段初始化（构造体末尾调用，虚派发此时生效）
}

QVector<KeyRow> StandardPanel::buildLayout() const {
  using K = Theme::Kind;
  return {
      {{{QStringLiteral("%"), IDK_PERCENT, K::Function},
        {QStringLiteral("CE"), IDK_CE, K::Function},
        {QStringLiteral("C"), IDK_CLEAR, K::Function},
        {QStringLiteral("⌫"), IDK_BACK, K::Function}}},
      {{{QStringLiteral("1/x"), IDK_RECIP, K::Function},
        {QStringLiteral("x²"), IDK_SQ, K::Function},
        {QStringLiteral("√x"), IDK_SQRT, K::Function},
        {QStringLiteral("÷"), IDK_DIV, K::Function}}},
      {{{QStringLiteral("7"), IDK_7, K::Number},
        {QStringLiteral("8"), IDK_8, K::Number},
        {QStringLiteral("9"), IDK_9, K::Number},
        {QStringLiteral("×"), IDK_MUL, K::Function}}},
      {{{QStringLiteral("4"), IDK_4, K::Number},
        {QStringLiteral("5"), IDK_5, K::Number},
        {QStringLiteral("6"), IDK_6, K::Number},
        {QStringLiteral("−"), IDK_SUB, K::Function}}},
      {{{QStringLiteral("1"), IDK_1, K::Number},
        {QStringLiteral("2"), IDK_2, K::Number},
        {QStringLiteral("3"), IDK_3, K::Number},
        {QStringLiteral("+"), IDK_ADD, K::Function}}},
      {{{QStringLiteral("±"), IDK_SIGN, K::Number},
        {QStringLiteral("0"), IDK_0, K::Number},
        {QStringLiteral("."), IDK_DOT, K::Number},
        {QStringLiteral("="), IDK_EQU, K::Accent}}},
  };
}
