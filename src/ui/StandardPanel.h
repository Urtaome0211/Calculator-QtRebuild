#pragma once
#include "KeypadPanel.h"
#include "../Commands.h"

// ============================================================
// 标准模式：4×6 数字键盘 + 记忆条
// ============================================================
class StandardPanel : public KeypadPanel {
  Q_OBJECT
public:
  StandardPanel(AppContext& ctx, QWidget* parent = nullptr);
protected:
  QVector<KeyRow> buildLayout() const override;
};
