#pragma once
#include "ModePanel.h"
#include "Keypad.h"
#include "../Commands.h"
#include "../core/ExpressionEngine.h"
#include "../core/NumberFormat.h"

// ============================================================
// 键盘计算面板基类（标准/科学共用）：记忆条 + 主键盘 + 表达式状态机
// ============================================================
class KeypadPanel : public ModePanel {
  Q_OBJECT
public:
  KeypadPanel(AppContext& ctx, QWidget* parent = nullptr);
  void onShow() override;
  void loadValue(const QString& v) override;
  bool handleKey(int key) override;
  bool handleChar(const QString& c) override;
protected:
  virtual QVector<KeyRow> memoryRow() const;   // 默认 MC MR M+ M- MS
  virtual QVector<KeyRow> buildLayout() const = 0;
  virtual bool onCommand(int cmd);
  virtual Expr::AngleMode angle() const { return Expr::AngleMode::Deg; }
  // 两阶段初始化：必须在子类构造函数体末尾调用（构造函数中虚派发不生效，
  // 直接调用纯虚 buildLayout() 会触发 "Pure virtual function called" 崩溃）
  void setupPanel();
  // ---- 工具 ----
  void insert(const QString& token);
  void updateDisplay();
  bool evalNow(double& v);
  void pressDigit(const QString& d);
  void pressOp(const QString& sym);
  void pressEquals();
  void applyUnary(const QString& prefix, const QString& suffix);
  void pushHistory();
  void setResultText(double v);
  QString expr_;
  bool justEval_ = false;
  double lastResult_ = 0.0;
  bool second_ = false;
  Keypad* pad_ = nullptr;
  Keypad* memPad_ = nullptr;
private:
  QWidget* memRow_ = nullptr;
};
