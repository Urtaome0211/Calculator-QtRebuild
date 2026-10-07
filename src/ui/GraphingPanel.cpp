#include "GraphingPanel.h"
#include "Theme.h"
#include "CalcButton.h"
#include "DisplayWidget.h"
#include "../Commands.h"
#include "../core/ExpressionEngine.h"
#include "../core/NumberFormat.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <limits>

// ============================================================
// 图形模式（Qt 版）：函数曲线画布 + 3 条方程输入
// ============================================================

// ---------------- GraphCanvas ----------------
GraphCanvas::GraphCanvas(QWidget* parent) : QWidget(parent) {
  setMinimumHeight(220);
  setMouseTracking(true);
}

void GraphCanvas::setEquations(const QStringList& exprs, const QVector<bool>& enabled) {
  exprs_ = exprs;
  enabled_ = enabled;
  replot();
}

void GraphCanvas::setColors(const QVector<QColor>& colors) {
  colors_ = colors;
  update();
}

void GraphCanvas::resetView() {
  viewSet_ = false;
  replot();
}

void GraphCanvas::replot() {
  for (int i = 0; i < 3; i++) {
    ys_[i].clear();
    ok_[i] = false;
    if (i >= exprs_.size() || !enabled_.value(i, false)) continue;
    const QString expr = exprs_[i].trimmed();
    if (expr.isEmpty()) {
      emit plotError(expr);
      continue;
    }
    bool any = false;
    for (int j = 0; j < kSamples; j++) {
      const double x = xmin_ + (xmax_ - xmin_) * j / (kSamples - 1);
      double y = 0;
      std::wstring err;
      if (Expr::EvaluateX(expr.toStdWString(), x, y, err, Expr::AngleMode::Rad)) {
        ys_[i] << y;
        any = true;
      } else {
        ys_[i] << std::numeric_limits<double>::quiet_NaN();
      }
    }
    if (!any) {
      ok_[i] = false;
      emit plotError(expr);
    } else {
      ok_[i] = true;
    }
  }
  // 自动适配 y 轴
  if (!viewSet_) {
    double ymin = 1e300, ymax = -1e300;
    for (int i = 0; i < 3; i++) {
      if (!ok_[i]) continue;
      for (double y : ys_[i]) {
        if (std::isfinite(y)) {
          ymin = std::min(ymin, y);
          ymax = std::max(ymax, y);
        }
      }
    }
    if (ymin > ymax) {
      ymin_ = -10;
      ymax_ = 10;
    } else {
      const double pad = (ymax - ymin) * 0.1;
      ymin_ = ymin - pad;
      ymax_ = ymax + pad;
      if (ymin_ == ymax_) {
        ymin_ -= 1;
        ymax_ += 1;
      }
    }
  }
  update();
}

void GraphCanvas::zoomAt(int cx, double factor) {
  const int left = 56, right = 16;
  const int plotW = width() - left - right;
  if (plotW <= 0) return;
  const double xc = xmin_ + (xmax_ - xmin_) * std::clamp((double)(cx - left), 0.0, (double)plotW) / plotW;
  xmin_ = xc - (xc - xmin_) * factor;
  xmax_ = xc + (xmax_ - xc) * factor;
  const double yc = (ymin_ + ymax_) / 2.0;
  ymin_ = yc - (yc - ymin_) * factor;
  ymax_ = yc + (ymax_ - yc) * factor;
  viewSet_ = true;
  replot();
}

void GraphCanvas::pan(int dx, int dy) {
  const int left = 56, right = 16, top = 28, bottom = 44;
  const int plotW = width() - left - right;
  const int plotH = height() - top - bottom;
  if (plotW <= 0 || plotH <= 0) return;
  const double dxv = (double)dx / plotW * (xmax_ - xmin_);
  xmin_ -= dxv;
  xmax_ -= dxv;
  const double dyv = (double)dy / plotH * (ymax_ - ymin_);
  ymin_ += dyv;
  ymax_ += dyv;
  viewSet_ = true;
  replot();
}

void GraphCanvas::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.fillRect(rect(), Theme::card());

  const int left = 56, right = 16, top = 28, bottom = 44;
  const int plotW = width() - left - right;
  const int plotH = height() - top - bottom;
  if (plotW <= 0 || plotH <= 0) return;
  const QRect plotRect(left, top, plotW, plotH);

  // 网格
  p.setPen(QPen(Theme::grid(), 1));
  for (int i = 0; i <= 8; i++) {
    const int x = left + plotW * i / 8;
    p.drawLine(x, top, x, top + plotH);
  }
  for (int i = 0; i <= 5; i++) {
    const int y = top + plotH * i / 5;
    p.drawLine(left, y, left + plotW, y);
  }
  // 边框
  p.setPen(QPen(Theme::border(), 1));
  p.drawRect(plotRect);

  // 坐标轴
  if (ymin_ < 0 && ymax_ > 0) {
    const int y0 = top + (int)((ymax_ / (ymax_ - ymin_)) * plotH);
    p.drawLine(left, y0, left + plotW, y0);
  }
  if (xmin_ < 0 && xmax_ > 0) {
    const int x0 = left + (int)((-xmin_ / (xmax_ - xmin_)) * plotW);
    p.drawLine(x0, top, x0, top + plotH);
  }

  // 刻度标签
  QFont tf = font();
  tf.setPointSize(8);
  p.setFont(tf);
  p.setPen(Theme::textDim());
  for (int i = 0; i <= 8; i++) {
    const double x = xmin_ + (xmax_ - xmin_) * i / 8;
    const int px = left + plotW * i / 8;
    const QString label = QString::fromStdWString(NumFmt::FormatCompact(x));
    p.drawText(QRect(px - 40, top + plotH + 4, 80, 16), Qt::AlignHCenter | Qt::AlignTop, label);
  }
  for (int i = 0; i <= 5; i++) {
    const double y = ymax_ - (ymax_ - ymin_) * i / 5;
    const int py = top + plotH * i / 5;
    const QString label = QString::fromStdWString(NumFmt::FormatCompact(y));
    p.drawText(QRect(0, py - 8, left - 6, 16), Qt::AlignRight | Qt::AlignVCenter, label);
  }

  // 曲线
  for (int i = 0; i < 3; i++) {
    if (!ok_[i] || ys_[i].size() < 2) continue;
    const QColor color = colors_.value(i, Theme::curve(i));
    p.setPen(QPen(color, 2.5));
    bool drawing = false;
    QPointF prev;
    for (int j = 0; j < ys_[i].size(); j++) {
      const double y = ys_[i][j];
      const double x = xmin_ + (xmax_ - xmin_) * j / (kSamples - 1);
      const double px = left + (x - xmin_) / (xmax_ - xmin_) * plotW;
      const double py = top + (ymax_ - y) / (ymax_ - ymin_) * plotH;
      if (std::isfinite(y)) {
        if (drawing) p.drawLine(prev, QPointF(px, py));
        prev = QPointF(px, py);
        drawing = true;
      } else {
        drawing = false;
      }
    }
  }

  // 图例
  int lx = left + 12, ly = top + 12;
  for (int i = 0; i < exprs_.size() && i < 3; i++) {
    if (!enabled_.value(i, false) || !ok_[i]) continue;
    const QColor color = colors_.value(i, Theme::curve(i));
    p.setPen(QPen(color, 2.5));
    p.drawLine(lx, ly + 8, lx + 18, ly + 8);
    p.setPen(Theme::textDim());
    p.drawText(QRect(lx + 24, ly, 220, 18), Qt::AlignLeft | Qt::AlignVCenter, exprs_[i]);
    ly += 20;
  }
}

void GraphCanvas::wheelEvent(QWheelEvent* event) {
  const double factor = std::pow(1.1, -event->angleDelta().y() / 120.0);
  zoomAt(event->position().x(), factor);
  event->accept();
}

void GraphCanvas::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    dragging_ = true;
    last_ = event->pos();
    event->accept();
  }
}

void GraphCanvas::mouseMoveEvent(QMouseEvent* event) {
  if (dragging_) {
    pan(event->pos().x() - last_.x(), event->pos().y() - last_.y());
    last_ = event->pos();
    event->accept();
  }
}

void GraphCanvas::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) dragging_ = false;
}

void GraphCanvas::mouseDoubleClickEvent(QMouseEvent*) { resetView(); }

// ---------------- GraphingPanel ----------------
GraphingPanel::GraphingPanel(AppContext& ctx, QWidget* parent)
    : ModePanel(ctx, parent) {
  auto* root = new QVBoxLayout(this);
  root->setContentsMargins(8, 4, 8, 8);
  root->setSpacing(4);

  // 3 条方程行
  for (int i = 0; i < 3; i++) {
    auto* row = new QHBoxLayout();
    toggles_[i] = new QCheckBox(QString::number(i + 1), this);
    toggles_[i]->setChecked(i == 0);
    edits_[i] = new QLineEdit(this);
    edits_[i]->setPlaceholderText(QStringLiteral("输入关于 x 的表达式，如 x^2"));
    row->addWidget(toggles_[i]);
    row->addWidget(edits_[i], 1);
    root->addLayout(row);
    connect(toggles_[i], &QCheckBox::checkStateChanged, this, &GraphingPanel::updatePlot);
    connect(edits_[i], &QLineEdit::returnPressed, this, &GraphingPanel::updatePlot);
  }
  edits_[0]->setText(QStringLiteral("x^2"));
  edits_[1]->setText(QStringLiteral("sin(x)"));

  // 按钮行
  auto* btnRow = new QHBoxLayout();
  auto* plotBtn = new CalcButton(QStringLiteral("绘图"), IDK_GR_PLOT, Theme::Kind::Accent, false, this);
  auto* resetBtn = new CalcButton(QStringLiteral("重置视图"), IDK_GR_RESET, Theme::Kind::Function, false, this);
  plotBtn->setFixedWidth(110);
  resetBtn->setFixedWidth(110);
  btnRow->addWidget(plotBtn);
  btnRow->addWidget(resetBtn);
  btnRow->addStretch(1);
  root->addLayout(btnRow);
  connect(plotBtn, &QPushButton::clicked, this, &GraphingPanel::updatePlot);
  connect(resetBtn, &QPushButton::clicked, this, [this] { canvas_->resetView(); });

  // 画布
  canvas_ = new GraphCanvas(this);
  root->addWidget(canvas_, 1);
  connect(canvas_, &GraphCanvas::plotError, this, [this](const QString& expr) {
    ctx_.display->setResult(QStringLiteral("输入无效: ") + expr, true);
  });
}

void GraphingPanel::updatePlot() {
  QStringList exprs;
  QVector<bool> enabled;
  QVector<QColor> colors;
  for (int i = 0; i < 3; i++) {
    exprs << edits_[i]->text().trimmed();
    enabled << toggles_[i]->isChecked();
    colors << Theme::curve(i);
  }
  canvas_->setEquations(exprs, enabled);
  canvas_->setColors(colors);

  // 显示区
  QStringList shown;
  for (int i = 0; i < 3; i++) {
    if (enabled[i] && !exprs[i].isEmpty()) shown << QStringLiteral("y = ") + exprs[i];
  }
  ctx_.display->setExpr(shown.join(QStringLiteral(" ; ")));
  ctx_.display->setResult(QString(), false);
}

void GraphingPanel::onShow() { updatePlot(); }
