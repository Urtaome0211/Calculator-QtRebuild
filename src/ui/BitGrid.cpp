// ============================================================
// BitGrid.cpp —— 程序员模式位网格控件实现（Qt 自绘）
// 布局：每行 16 个方格，最高位在最左、最低位在最右，行间位号递减；
// 点击方格翻转对应位；悬停高亮；底部每 4 位一组标注组内最高位序号。
// 仅依赖 BitGrid.h / Theme.h。
// ============================================================
#include "BitGrid.h"
#include "Theme.h"
#include <QPainter>
#include <QMouseEvent>
#include <algorithm>

namespace {
// 网格布局参数（像素）
struct Layout {
  int left = 6;     // 左/右边距
  int top = 4;      // 上边距
  int gap = 4;      // 方格间距
  int labelH = 18;  // 底部标签区高度
  int rows = 1;     // 行数
  qreal cellW = 0;  // 方格宽度
  qreal cellH = 0;  // 方格高度
};

Layout CalcLayout(int w, int h, int bits) {
  Layout L;
  L.rows = std::max(1, bits / 16);
  L.cellW = (w - 2 * L.left - 15 * L.gap) / 16.0;
  L.cellH = (h - L.top - L.labelH - (L.rows - 1) * L.gap) / qreal(L.rows);
  return L;
}

// 位号 i（0 为最低位）所在的行列：最高位在 0 行 0 列，行内从左到右位号递减
void CellPos(int bits, int i, int& row, int& col) {
  int pos = bits - 1 - i;  // 从最高位起算的序号
  row = pos / 16;
  col = pos % 16;
}
}  // namespace

// ------------------------------------------------------------
// 构造：开启鼠标跟踪，无需按下即可收到移动事件用于悬停
// ------------------------------------------------------------
BitGrid::BitGrid(QWidget* parent) : QWidget(parent) {
  setMouseTracking(true);
}

// ------------------------------------------------------------
// 设置数值 / 字长（仅 8/16/32/64，其余按 64 处理）
// ------------------------------------------------------------
void BitGrid::setValue(quint64 v) {
  value_ = v;
  update();
}

void BitGrid::setBits(int bits) {
  bits_ = (bits == 8 || bits == 16 || bits == 32 || bits == 64) ? bits : 64;
  update();
}

QSize BitGrid::sizeHint() const {
  return QSize(320, std::max(1, bits_ / 16) * 32 + 24);
}

// ------------------------------------------------------------
// 绘制：卡片色背景 + 圆角方格 + 底部分组标签
// ------------------------------------------------------------
void BitGrid::paintEvent(QPaintEvent* /*event*/) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  const int w = width(), h = height();

  // 1) 背景
  p.fillRect(rect(), Theme::card());

  const Layout L = CalcLayout(w, h, bits_);
  if (L.cellW <= 0 || L.cellH <= 0) return;

  // 2) 方格：只绘制 bits_ 个有效位，其余留白
  p.setPen(Qt::NoPen);
  for (int i = 0; i < bits_; ++i) {
    int row, col;
    CellPos(bits_, i, row, col);
    const QRectF r(L.left + col * (L.cellW + L.gap),
                   L.top + row * (L.cellH + L.gap),
                   L.cellW, L.cellH);
    QColor c;
    if (i == hover_) {
      c = Theme::hoverNum();                 // 悬停叠加
    } else if (value_ & (1ull << i)) {
      c = Theme::accent();                   // 置位
    } else {
      c = Theme::fn();                       // 未置位
    }
    p.setBrush(c);
    p.drawRoundedRect(r, 3.0, 3.0);
  }

  // 3) 底部标签：每 4 个方格一组，居中标注该组最高位序号
  QFont f = font();
  f.setPointSize(9);
  p.setFont(f);
  p.setPen(Theme::textDim());
  const int groups = (bits_ + 3) / 4;
  for (int gi = 0; gi < groups; ++gi) {
    const qreal x0 = L.left + gi * 4 * (L.cellW + L.gap);
    const qreal gw = 4.0 * L.cellW + 3.0 * L.gap;
    const QRectF lr(x0 - L.gap * 0.5,
                    qreal(h - L.labelH),
                    gw + L.gap,
                    qreal(L.labelH));
    p.drawText(lr, Qt::AlignCenter, QString::number(bits_ - 1 - gi * 4));
  }
}

// ------------------------------------------------------------
// 命中检测：把控件坐标换算成位号；落在间隙或无效位返回 -1
// ------------------------------------------------------------
int BitGrid::hitTest(const QPoint& pt) const {
  const Layout L = CalcLayout(width(), height(), bits_);
  if (L.cellW <= 0 || L.cellH <= 0) return -1;

  const qreal offX = pt.x() - L.left;
  const qreal offY = pt.y() - L.top;
  if (offX < 0 || offY < 0) return -1;

  const int col = int(offX / (L.cellW + L.gap));
  const int row = int(offY / (L.cellH + L.gap));
  if (col < 0 || col > 15 || row < 0 || row >= L.rows) return -1;
  if (offX - col * (L.cellW + L.gap) >= L.cellW) return -1;  // 落在横向间隙
  if (offY - row * (L.cellH + L.gap) >= L.cellH) return -1;  // 落在纵向间隙

  const int i = bits_ - 1 - (row * 16 + col);  // 行列 → 位号（最高位在左上角）
  return (i >= 0 && i < bits_) ? i : -1;
}

// ------------------------------------------------------------
// 交互：悬停跟踪 / 离开清除 / 左键翻转位并发出信号
// ------------------------------------------------------------
void BitGrid::mouseMoveEvent(QMouseEvent* event) {
  const int idx = hitTest(event->pos());
  if (idx != hover_) {
    hover_ = idx;
    update();
  }
}

void BitGrid::leaveEvent(QEvent* /*event*/) {
  if (hover_ != -1) {
    hover_ = -1;
    update();
  }
}

void BitGrid::mousePressEvent(QMouseEvent* event) {
  if (event->button() != Qt::LeftButton) return;
  const int i = hitTest(event->pos());
  if (i < 0) return;
  value_ ^= (1ull << i);
  emit bitToggled(value_);
  update();
}
