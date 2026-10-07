#pragma once
#include "KeypadPanel.h"

// ============================================================
// 科学模式：8 列键盘 + 三角侧栏 + 2nd 切换 + 角度制
// ============================================================
class ScientificPanel : public KeypadPanel {
  Q_OBJECT
public:
  ScientificPanel(AppContext& ctx, QWidget* parent = nullptr);
  void onShow() override;
  bool handleChar(const QString& c) override;
protected:
  QVector<KeyRow> memoryRow() const override;
  QVector<KeyRow> buildLayout() const override;
  bool onCommand(int cmd) override;
  Expr::AngleMode angle() const override { return angle_; }
private:
  void applySecond();
  void refreshAngleChecks();
  void pressFunc(const QString& name);
  Expr::AngleMode angle_ = Expr::AngleMode::Deg;
};
