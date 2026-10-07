#pragma once
#include <QWidget>
#include <QGridLayout>
#include <QVector>
#include <QHash>
#include "CalcButton.h"

// ============================================================
// 声明式键盘：按 KeyCell/KeyRow 规格批量创建按钮
// ============================================================
struct KeyCell {
  QString label;
  int cmd = 0;                          // 0 = 空白占位
  Theme::Kind kind = Theme::Kind::Number;
  int colSpan = 1;
  int rowSpan = 1;
  bool small = false;
};
struct KeyRow { QVector<KeyCell> cells; };

class Keypad : public QWidget {
  Q_OBJECT
public:
  explicit Keypad(int columns, QWidget* parent = nullptr);
  void build(const QVector<KeyRow>& rows, int hGap = 4, int vGap = 4);
  CalcButton* get(int cmd) const;
  void setChecked(int cmd, bool on);
  void setEnabled(int cmd, bool on);
  QVector<CalcButton*> allButtons() const;
  QGridLayout* grid() const { return grid_; }
private:
  int columns_;
  QGridLayout* grid_ = nullptr;
  QHash<int, CalcButton*> buttons_;
};
