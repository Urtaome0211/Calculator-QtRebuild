#pragma once
#include "ModePanel.h"
#include "Keypad.h"
#include "BitGrid.h"
#include "../Commands.h"

// ============================================================
// 程序员模式：进制/字长切换 + 位网格 + 位运算键盘
// ============================================================
class ProgrammerPanel : public ModePanel {
  Q_OBJECT
public:
  ProgrammerPanel(AppContext& ctx, QWidget* parent = nullptr);
  void onShow() override;
  void loadValue(const QString& v) override;
  bool handleKey(int key) override;
  bool handleChar(const QString& c) override;
  int altLineCount() const override { return 4; }
private slots:
  void onBitToggled(quint64 newValue);
private:
  bool onCommand(int cmd);
  void insert(const QString& token);
  void updateDisplay();
  void eval();
  void setBase(unsigned base);
  void setBits(unsigned bits);
  void applyBaseEnabled();
  void createWidgets();
  QString expr_;
  unsigned base_ = 16, bits_ = 64;
  quint64 value_ = 0;
  bool hasValue_ = false, justEval_ = false;
  QWidget* topRow_ = nullptr;
  BitGrid* bitGrid_ = nullptr;
  Keypad* pad_ = nullptr;
  QHash<int, CalcButton*> topButtons_;
};
