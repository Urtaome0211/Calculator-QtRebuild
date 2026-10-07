#pragma once
#include <QComboBox>
#include <QLineEdit>
#include <QWidget>
#include "ModePanel.h"

// ============================================================
// 日期计算：日期差值 / 基准日期加减（天/周/月/年）
// ============================================================
class DatePanel : public ModePanel {
  Q_OBJECT
public:
  DatePanel(AppContext& ctx, QWidget* parent = nullptr);
  void onShow() override;
  int altLineCount() const override { return 2; }
private slots:
  void doCalc();
  void resetInputs();
  void applyMode();
private:
  bool modeDiff_ = true;
  QWidget* diffPage_ = nullptr;
  QWidget* addPage_ = nullptr;
  QLineEdit* editFrom_ = nullptr;
  QLineEdit* editTo_ = nullptr;
  QLineEdit* editBase_ = nullptr;
  QLineEdit* editAmount_ = nullptr;
  QComboBox* comboUnit_ = nullptr;
  QComboBox* comboSign_ = nullptr;
};
