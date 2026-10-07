#pragma once
#include <QWidget>
#include <QListWidget>
#include <vector>
#include "../core/HistoryStore.h"

// ============================================================
// 历史记录侧栏：列表 + 清除/删除，双击载入数值
// ============================================================
class HistoryPanel : public QWidget {
  Q_OBJECT
public:
  explicit HistoryPanel(QWidget* parent = nullptr);
  void refresh(const History::Store& store);
signals:
  void loadRequested(const QString& value);
  void clearRequested();
  void deleteRequested(int index);
private:
  QListWidget* list_ = nullptr;
  std::vector<History::Entry> entries_;
};
