#pragma once
#include <QPushButton>
#include "Theme.h"

// ============================================================
// 计算器按键：通过 kind 属性匹配 QSS，Toggle 类型可选中
// ============================================================
class CalcButton : public QPushButton {
  Q_OBJECT
public:
  CalcButton(const QString& label, int cmd, Theme::Kind kind,
             bool small = false, QWidget* parent = nullptr);
  int cmd() const { return cmd_; }
  void setSmall(bool small);
private:
  int cmd_;
};
