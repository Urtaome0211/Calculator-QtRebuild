#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <QDockWidget>
#include <QHash>
#include "ModePanel.h"
#include "DisplayWidget.h"
#include "HistoryPanel.h"
#include "../core/HistoryStore.h"

// ============================================================
// 主窗口：顶栏（菜单/模式名/历史/主题）+ 显示区 + 模式栈 + 历史停靠栏
// ============================================================
class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow();
  void switchMode(int navCmd);
protected:
  void keyPressEvent(QKeyEvent* event) override;
private slots:
  void toggleHistory();
  void toggleTheme();
  void onHistoryLoad(const QString& v);
  void onHistoryClear();
  void onHistoryDelete(int index);
private:
  ModePanel* panelFor(int modeCmd);
  QString modeName(int navCmd) const;
  void refreshMemory();
  void refreshHistory();
  void buildNavMenu();

  AppContext ctx_;
  History::Store store_;
  double memory_ = 0;
  DisplayWidget* display_ = nullptr;
  QStackedWidget* stack_ = nullptr;
  QLabel* modeLabel_ = nullptr;
  QToolButton* btnNav_ = nullptr;
  QToolButton* btnTheme_ = nullptr;
  QToolButton* btnHistory_ = nullptr;
  QMenu* navMenu_ = nullptr;
  QDockWidget* historyDock_ = nullptr;
  HistoryPanel* historyPanel_ = nullptr;
  QHash<int, ModePanel*> panels_;
  ModePanel* active_ = nullptr;
  int currentMode_ = 0;
};
