#include "CalcButton.h"
#include <QStyle>

// ============================================================
// 计算器按键实现
// ============================================================
CalcButton::CalcButton(const QString& label, int cmd, Theme::Kind kind,
                       bool small, QWidget* parent)
    : QPushButton(label, parent), cmd_(cmd) {
  QString k;
  switch (kind) {
    case Theme::Kind::Number: k = QStringLiteral("num"); break;
    case Theme::Kind::Function: k = QStringLiteral("fn"); break;
    case Theme::Kind::Accent: k = QStringLiteral("accent"); break;
    case Theme::Kind::Memory: k = QStringLiteral("mem"); break;
    case Theme::Kind::Toggle: k = QStringLiteral("toggle"); break;
  }
  setProperty("kind", k);
  if (kind == Theme::Kind::Toggle) setCheckable(true);
  if (small) setProperty("small", QStringLiteral("true"));
  setFocusPolicy(Qt::NoFocus);
  setCursor(Qt::PointingHandCursor);
}

void CalcButton::setSmall(bool small) {
  setProperty("small", small ? QStringLiteral("true") : QString());
  style()->unpolish(this);
  style()->polish(this);
}
