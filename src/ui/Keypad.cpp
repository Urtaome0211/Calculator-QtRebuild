#include "Keypad.h"

// ============================================================
// 声明式键盘实现
// ============================================================
Keypad::Keypad(int columns, QWidget* parent)
    : QWidget(parent), columns_(columns) {
  grid_ = new QGridLayout(this);
  grid_->setContentsMargins(0, 0, 0, 0);
}

void Keypad::build(const QVector<KeyRow>& rows, int hGap, int vGap) {
  grid_->setSpacing(std::max(hGap, vGap));
  buttons_.clear();
  int r = 0;
  for (const KeyRow& row : rows) {
    int c = 0;
    for (const KeyCell& cell : row.cells) {
      const int span = std::max(1, cell.colSpan);
      if (cell.cmd != 0) {
        auto* btn = new CalcButton(cell.label, cell.cmd, cell.kind, cell.small, this);
        grid_->addWidget(btn, r, c, std::max(1, cell.rowSpan), span);
        buttons_.insert(cell.cmd, btn);
      }
      c += span;
    }
    r++;
  }
  for (int col = 0; col < columns_; col++) grid_->setColumnStretch(col, 1);
  for (int row = 0; row < rows.size(); row++) grid_->setRowStretch(row, 1);
}

CalcButton* Keypad::get(int cmd) const { return buttons_.value(cmd, nullptr); }

void Keypad::setChecked(int cmd, bool on) {
  if (CalcButton* b = get(cmd)) b->setChecked(on);
}

void Keypad::setEnabled(int cmd, bool on) {
  if (CalcButton* b = get(cmd)) b->setEnabled(on);
}

QVector<CalcButton*> Keypad::allButtons() const {
  QVector<CalcButton*> v;
  for (CalcButton* b : buttons_) v << b;
  return v;
}
