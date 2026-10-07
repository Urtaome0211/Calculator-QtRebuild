#pragma once
#include <QWidget>
#include <QStringList>

// ============================================================
// 显示区：表达式行 + 大字号结果 + 次要信息行 + M 徽标
// ============================================================
class DisplayWidget : public QWidget {
  Q_OBJECT
public:
  explicit DisplayWidget(QWidget* parent = nullptr);
  void setExpr(const QString& s);
  void setResult(const QString& s, bool error = false);
  void setAltLines(const QStringList& lines);
  void setMemory(bool on);
  int heightHint() const;
  QSize sizeHint() const override { return QSize(320, heightHint()); }
protected:
  void paintEvent(QPaintEvent* event) override;
private:
  QString expr_, result_;
  QStringList alt_;
  bool error_ = false, mem_ = false;
};
