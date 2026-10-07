#include "DisplayWidget.h"
#include "Theme.h"
#include <QPainter>
#include <QFontMetrics>

// ============================================================
// 显示区实现：QPainter 绘制表达式/结果/次要行 + M 徽标
// ============================================================
DisplayWidget::DisplayWidget(QWidget* parent) : QWidget(parent) {
  setMinimumHeight(heightHint());
}

int DisplayWidget::heightHint() const {
  return 8 + 24 + 56 + alt_.size() * 22 + 6;
}

void DisplayWidget::setExpr(const QString& s) {
  expr_ = s;
  update();
}
void DisplayWidget::setResult(const QString& s, bool error) {
  result_ = s;
  error_ = error;
  update();
}
void DisplayWidget::setAltLines(const QStringList& lines) {
  alt_ = lines;
  updateGeometry();   // 高度随次要行数变化，通知布局重新布局
  update();
}
void DisplayWidget::setMemory(bool on) {
  mem_ = on;
  update();
}

void DisplayWidget::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.fillRect(rect(), Theme::bg());

  int y = 8;
  // M 记忆徽标
  if (mem_) {
    const QRectF chip(0, 12, 38, 22);
    p.setPen(Qt::NoPen);
    p.setBrush(Theme::accent());
    p.drawRoundedRect(chip, 4, 4);
    p.setPen(Theme::accentText());
    QFont f = p.font();
    f.setPointSize(9);
    f.setBold(true);
    p.setFont(f);
    p.drawText(chip, Qt::AlignCenter, QStringLiteral("M"));
  }

  const int w = width();
  // 表达式行（右对齐，超出时缩小字号）
  if (!expr_.isEmpty()) {
    int size = 11;
    QFont f;
    f.setPointSize(size);
    while (size > 7 && QFontMetrics(f).horizontalAdvance(expr_) > w - 12) {
      f.setPointSize(--size);
    }
    p.setFont(f);
    p.setPen(Theme::textDim());
    p.drawText(QRect(0, y, w - 12, 24), Qt::AlignRight | Qt::AlignVCenter, expr_);
  }
  y += 24;

  // 结果行
  if (!result_.isEmpty()) {
    int size = 30;
    QFont f;
    f.setPointSize(size);
    f.setBold(true);
    while (size > 11 && QFontMetrics(f).horizontalAdvance(result_) > w - 12) {
      f.setPointSize(--size);
    }
    p.setFont(f);
    p.setPen(error_ ? Theme::errorColor() : Theme::text());
    p.drawText(QRect(0, y, w - 12, 56), Qt::AlignRight | Qt::AlignVCenter, result_);
  }
  y += 56;

  // 次要信息行
  QFont sf;
  sf.setPointSize(9);
  p.setFont(sf);
  p.setPen(Theme::textDim());
  for (const QString& line : alt_) {
    p.drawText(QRect(0, y, w - 12, 22), Qt::AlignRight | Qt::AlignVCenter, line);
    y += 22;
  }
}
