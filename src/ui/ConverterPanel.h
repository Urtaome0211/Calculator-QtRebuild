#pragma once
#include <QComboBox>
#include <QLineEdit>
#include "ModePanel.h"

// ============================================================
// 单位转换器：类别 + 双单位卡片，双向即时换算
// ============================================================
class ConverterPanel : public ModePanel {
  Q_OBJECT
public:
  ConverterPanel(AppContext& ctx, QWidget* parent = nullptr);
  void onShow() override;
  void setCategory(const QString& id);
  int altLineCount() const override { return 1; }
private slots:
  void convert();          // 左 -> 右
  void convertReverse();   // 右 -> 左
  void reloadUnits();
  void swapUnits();
private:
  QComboBox* comboCat_ = nullptr;
  QComboBox* comboFrom_ = nullptr;
  QComboBox* comboTo_ = nullptr;
  QLineEdit* editFrom_ = nullptr;
  QLineEdit* editTo_ = nullptr;
  QString catId_ = QStringLiteral("length");
  bool updating_ = false;
};
