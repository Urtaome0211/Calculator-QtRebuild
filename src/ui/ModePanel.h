#pragma once
#include <QWidget>
#include <QString>
#include <functional>
#include "../core/HistoryStore.h"

class DisplayWidget;

// ============================================================
// 模式面板基类：共享应用上下文与统一接口
// ============================================================
struct AppContext {
  DisplayWidget* display = nullptr;
  History::Store* history = nullptr;
  double* memory = nullptr;
  std::function<void(const std::wstring&, const std::wstring&)> addHistory;
  std::function<void()> refreshMemory;
  std::function<void()> refreshHistory;
};

class ModePanel : public QWidget {
  Q_OBJECT
public:
  ModePanel(AppContext& ctx, QWidget* parent = nullptr);
  virtual void onShow() {}
  virtual void loadValue(const QString& v) { Q_UNUSED(v); }
  virtual int altLineCount() const { return 0; }
  virtual bool handleKey(int key) { Q_UNUSED(key); return false; }     // Qt::Key_*
  virtual bool handleChar(const QString& c) { Q_UNUSED(c); return false; }
protected:
  AppContext& ctx_;
};
