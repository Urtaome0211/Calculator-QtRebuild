#pragma once
#include <QWidget>

// ============================================================
// 位翻转网格：64 个可点击方块，点击翻转对应位
// ============================================================
class BitGrid : public QWidget {
  Q_OBJECT
public:
  explicit BitGrid(QWidget* parent = nullptr);
  void setValue(quint64 v);
  void setBits(int bits);          // 8/16/32/64
  quint64 value() const { return value_; }
  QSize sizeHint() const override;
signals:
  void bitToggled(quint64 newValue);
protected:
  void paintEvent(QPaintEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void leaveEvent(QEvent* event) override;
private:
  int hitTest(const QPoint& p) const;
  quint64 value_ = 0;
  int bits_ = 64;
  int hover_ = -1;
};
