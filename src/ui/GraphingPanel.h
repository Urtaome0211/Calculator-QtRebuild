#pragma once
#include <QWidget>
#include <QStringList>
#include <QVector>
#include <QColor>
#include <QPoint>
#include <QCheckBox>
#include <QLineEdit>
#include "ModePanel.h"

// ============================================================
// 函数曲线画布：600 点采样、滚轮缩放、拖拽平移、双击复位
// ============================================================
class GraphCanvas : public QWidget {
  Q_OBJECT
public:
  explicit GraphCanvas(QWidget* parent = nullptr);
  void setEquations(const QStringList& exprs, const QVector<bool>& enabled);
  void setColors(const QVector<QColor>& colors);
  void resetView();
signals:
  void plotError(const QString& expr);
protected:
  void paintEvent(QPaintEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
private:
  void replot();
  void zoomAt(int cx, double factor);
  void pan(int dx, int dy);
  QStringList exprs_;
  QVector<bool> enabled_;
  QVector<QColor> colors_;
  double xmin_ = -10, xmax_ = 10, ymin_ = -10, ymax_ = 10;
  bool viewSet_ = false;
  static const int kSamples = 600;
  QVector<double> ys_[3];
  bool ok_[3] = {false, false, false};
  bool dragging_ = false;
  QPoint last_;
};

// ============================================================
// 图形模式：3 条方程输入 + 画布
// ============================================================
class GraphingPanel : public ModePanel {
  Q_OBJECT
public:
  GraphingPanel(AppContext& ctx, QWidget* parent = nullptr);
  void onShow() override;
private slots:
  void updatePlot();
private:
  QLineEdit* edits_[3] = {};
  QCheckBox* toggles_[3] = {};
  GraphCanvas* canvas_ = nullptr;
};
