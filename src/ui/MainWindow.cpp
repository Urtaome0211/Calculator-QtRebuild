#include "MainWindow.h"
#include "Theme.h"
#include "StandardPanel.h"
#include "ScientificPanel.h"
#include "ProgrammerPanel.h"
#include "GraphingPanel.h"
#include "DatePanel.h"
#include "ConverterPanel.h"
#include "../Commands.h"
#include "../core/UnitConverter.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QAbstractSpinBox>
#include <QActionGroup>

// ============================================================
// Qt 版主窗口
// ============================================================
namespace {

bool isConvCmd(int cmd) {
  return cmd >= ID_NAV_CONV_CURRENCY && cmd <= ID_NAV_CONV_FREQ;
}

QString convCatId(int cmd) {
  switch (cmd) {
    case ID_NAV_CONV_CURRENCY: return QStringLiteral("currency");
    case ID_NAV_CONV_VOLUME: return QStringLiteral("volume");
    case ID_NAV_CONV_LENGTH: return QStringLiteral("length");
    case ID_NAV_CONV_WEIGHT: return QStringLiteral("weight");
    case ID_NAV_CONV_TEMP: return QStringLiteral("temperature");
    case ID_NAV_CONV_AREA: return QStringLiteral("area");
    case ID_NAV_CONV_SPEED: return QStringLiteral("speed");
    case ID_NAV_CONV_TIME: return QStringLiteral("time");
    case ID_NAV_CONV_ENERGY: return QStringLiteral("energy");
    case ID_NAV_CONV_PRESSURE: return QStringLiteral("pressure");
    case ID_NAV_CONV_POWER: return QStringLiteral("power");
    case ID_NAV_CONV_ANGLE: return QStringLiteral("angle");
    case ID_NAV_CONV_DATA: return QStringLiteral("data");
    case ID_NAV_CONV_FREQ: return QStringLiteral("frequency");
  }
  return QStringLiteral("length");
}

}  // namespace

MainWindow::MainWindow() {
  // 上下文
  ctx_.display = nullptr;
  ctx_.history = &store_;
  ctx_.memory = &memory_;
  ctx_.addHistory = [this](const std::wstring& e, const std::wstring& r) {
    store_.Add(e, r);
    refreshHistory();
  };
  ctx_.refreshMemory = [this] { refreshMemory(); };
  ctx_.refreshHistory = [this] { refreshHistory(); };

  // 中央部件
  auto* central = new QWidget(this);
  auto* vbox = new QVBoxLayout(central);
  vbox->setContentsMargins(0, 0, 0, 0);
  vbox->setSpacing(0);

  // 顶栏
  auto* topBar = new QWidget(central);
  auto* h = new QHBoxLayout(topBar);
  h->setContentsMargins(4, 4, 8, 4);
  h->setSpacing(4);
  btnNav_ = new QToolButton(topBar);
  btnNav_->setText(QStringLiteral("☰"));
  btnNav_->setAutoRaise(true);
  btnTheme_ = new QToolButton(topBar);
  btnTheme_->setText(Theme::isDark() ? QStringLiteral("☀") : QStringLiteral("🌙"));
  btnTheme_->setAutoRaise(true);
  btnHistory_ = new QToolButton(topBar);
  btnHistory_->setText(QStringLiteral("🕘"));
  btnHistory_->setAutoRaise(true);
  modeLabel_ = new QLabel(topBar);
  modeLabel_->setStyleSheet(QStringLiteral("font-weight: bold;"));
  h->addWidget(btnNav_);
  h->addWidget(modeLabel_);
  h->addStretch(1);
  h->addWidget(btnTheme_);
  h->addWidget(btnHistory_);
  vbox->addWidget(topBar);

  // 显示区
  display_ = new DisplayWidget(central);
  ctx_.display = display_;
  vbox->addWidget(display_);

  // 模式栈
  stack_ = new QStackedWidget(central);
  vbox->addWidget(stack_, 1);
  setCentralWidget(central);

  buildNavMenu();
  connect(btnNav_, &QToolButton::clicked, this, [this] {
    navMenu_->exec(btnNav_->mapToGlobal(QPoint(0, btnNav_->height())));
  });
  connect(btnTheme_, &QToolButton::clicked, this, &MainWindow::toggleTheme);
  connect(btnHistory_, &QToolButton::clicked, this, &MainWindow::toggleHistory);

  // 历史停靠栏
  historyDock_ = new QDockWidget(QStringLiteral("历史记录"), this);
  historyDock_->setObjectName(QStringLiteral("historyDock"));
  historyDock_->setAllowedAreas(Qt::RightDockWidgetArea);
  historyPanel_ = new HistoryPanel(historyDock_);
  historyDock_->setWidget(historyPanel_);
  addDockWidget(Qt::RightDockWidgetArea, historyDock_);
  historyDock_->hide();
  connect(historyPanel_, &HistoryPanel::loadRequested, this, &MainWindow::onHistoryLoad);
  connect(historyPanel_, &HistoryPanel::clearRequested, this, &MainWindow::onHistoryClear);
  connect(historyPanel_, &HistoryPanel::deleteRequested, this, &MainWindow::onHistoryDelete);

  switchMode(ID_NAV_STANDARD);
}

void MainWindow::buildNavMenu() {
  navMenu_ = new QMenu(this);
  auto addItem = [this](QMenu* menu, const QString& text, int cmd) {
    QAction* a = menu->addAction(text);
    connect(a, &QAction::triggered, this, [this, cmd] { switchMode(cmd); });
  };
  addItem(navMenu_, QStringLiteral("标准"), ID_NAV_STANDARD);
  addItem(navMenu_, QStringLiteral("科学"), ID_NAV_SCIENTIFIC);
  addItem(navMenu_, QStringLiteral("图形"), ID_NAV_GRAPHING);
  addItem(navMenu_, QStringLiteral("程序员"), ID_NAV_PROGRAMMER);
  addItem(navMenu_, QStringLiteral("日期计算"), ID_NAV_DATE);
  navMenu_->addSeparator();
  QMenu* conv = navMenu_->addMenu(QStringLiteral("转换器"));
  addItem(conv, QStringLiteral("货币"), ID_NAV_CONV_CURRENCY);
  addItem(conv, QStringLiteral("体积"), ID_NAV_CONV_VOLUME);
  addItem(conv, QStringLiteral("长度"), ID_NAV_CONV_LENGTH);
  addItem(conv, QStringLiteral("重量"), ID_NAV_CONV_WEIGHT);
  addItem(conv, QStringLiteral("温度"), ID_NAV_CONV_TEMP);
  addItem(conv, QStringLiteral("面积"), ID_NAV_CONV_AREA);
  addItem(conv, QStringLiteral("速度"), ID_NAV_CONV_SPEED);
  addItem(conv, QStringLiteral("时间"), ID_NAV_CONV_TIME);
  addItem(conv, QStringLiteral("能量"), ID_NAV_CONV_ENERGY);
  addItem(conv, QStringLiteral("压力"), ID_NAV_CONV_PRESSURE);
  addItem(conv, QStringLiteral("功率"), ID_NAV_CONV_POWER);
  addItem(conv, QStringLiteral("角度"), ID_NAV_CONV_ANGLE);
  addItem(conv, QStringLiteral("数据存储"), ID_NAV_CONV_DATA);
  addItem(conv, QStringLiteral("频率"), ID_NAV_CONV_FREQ);
  navMenu_->addSeparator();
  QAction* theme = navMenu_->addAction(Theme::isDark() ? QStringLiteral("浅色模式") : QStringLiteral("深色模式"));
  connect(theme, &QAction::triggered, this, &MainWindow::toggleTheme);
}

ModePanel* MainWindow::panelFor(int modeCmd) {
  if (panels_.contains(modeCmd)) return panels_.value(modeCmd);
  ModePanel* panel = nullptr;
  switch (modeCmd) {
    case ID_NAV_STANDARD: panel = new StandardPanel(ctx_); break;
    case ID_NAV_SCIENTIFIC: panel = new ScientificPanel(ctx_); break;
    case ID_NAV_GRAPHING: panel = new GraphingPanel(ctx_); break;
    case ID_NAV_PROGRAMMER: panel = new ProgrammerPanel(ctx_); break;
    case ID_NAV_DATE: panel = new DatePanel(ctx_); break;
    case ID_NAV_CONVERTER: panel = new ConverterPanel(ctx_); break;
    default: return nullptr;
  }
  stack_->addWidget(panel);
  panels_.insert(modeCmd, panel);
  return panel;
}

QString MainWindow::modeName(int navCmd) const {
  switch (navCmd) {
    case ID_NAV_STANDARD: return QStringLiteral("标准");
    case ID_NAV_SCIENTIFIC: return QStringLiteral("科学");
    case ID_NAV_GRAPHING: return QStringLiteral("图形");
    case ID_NAV_PROGRAMMER: return QStringLiteral("程序员");
    case ID_NAV_DATE: return QStringLiteral("日期计算");
  }
  if (isConvCmd(navCmd)) {
    const UnitConv::Category* cat = UnitConv::Find(convCatId(navCmd).toStdWString());
    if (cat) return QStringLiteral("转换器 · ") + QString::fromStdWString(cat->name);
  }
  return QStringLiteral("转换器");
}

void MainWindow::switchMode(int navCmd) {
  const int key = isConvCmd(navCmd) ? ID_NAV_CONVERTER : navCmd;
  ModePanel* panel = panelFor(key);
  if (!panel) return;
  currentMode_ = navCmd;
  active_ = panel;
  stack_->setCurrentWidget(panel);

  if (key == ID_NAV_CONVERTER) {
    auto* conv = qobject_cast<ConverterPanel*>(panel);
    if (conv) conv->setCategory(convCatId(navCmd));
  }

  display_->setExpr(QString());
  display_->setResult(QString(), false);
  display_->setAltLines(QStringList());
  display_->setMemory(memory_ != 0.0);
  panel->onShow();
  modeLabel_->setText(modeName(navCmd));

  // 按模式调整最小尺寸
  QSize min(320, 480);
  switch (key) {
    case ID_NAV_SCIENTIFIC: min = QSize(720, 560); break;
    case ID_NAV_GRAPHING: min = QSize(640, 560); break;
    case ID_NAV_PROGRAMMER: min = QSize(760, 620); break;
    case ID_NAV_DATE: min = QSize(420, 480); break;
    case ID_NAV_CONVERTER: min = QSize(400, 460); break;
    default: break;
  }
  resize(size().expandedTo(min));
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
  if (!active_) {
    QMainWindow::keyPressEvent(event);
    return;
  }
  // 焦点在文本输入类控件时不拦截
  QWidget* f = focusWidget();
  if (qobject_cast<QLineEdit*>(f) || qobject_cast<QComboBox*>(f) ||
      qobject_cast<QAbstractSpinBox*>(f)) {
    QMainWindow::keyPressEvent(event);
    return;
  }
  if (active_->handleKey(event->key())) {
    event->accept();
    return;
  }
  const QString text = event->text();
  if (!text.isEmpty() && active_->handleChar(text)) {
    event->accept();
    return;
  }
  QMainWindow::keyPressEvent(event);
}

void MainWindow::toggleHistory() {
  historyDock_->setVisible(!historyDock_->isVisible());
  if (historyDock_->isVisible()) refreshHistory();
}

void MainWindow::toggleTheme() {
  Theme::setDark(!Theme::isDark());
  static_cast<QApplication*>(QCoreApplication::instance())
      ->setStyleSheet(Theme::appStyleSheet());
  btnTheme_->setText(Theme::isDark() ? QStringLiteral("☀") : QStringLiteral("🌙"));
}

void MainWindow::refreshMemory() { display_->setMemory(memory_ != 0.0); }

void MainWindow::refreshHistory() { historyPanel_->refresh(store_); }

void MainWindow::onHistoryLoad(const QString& v) {
  if (active_) active_->loadValue(v);
}

void MainWindow::onHistoryClear() {
  store_.Clear();
  refreshHistory();
}

void MainWindow::onHistoryDelete(int index) {
  store_.Remove((size_t)index);
  refreshHistory();
}
