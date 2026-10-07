// ============================================================
// Qt 重构版计算器入口
// ============================================================
#include <QApplication>
#include <QFont>
#include "ui/Theme.h"
#include "ui/MainWindow.h"
#include "Commands.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName(QStringLiteral("QtCalculator"));
  app.setOrganizationName(QStringLiteral("W11Calc"));
  QFont f(QStringLiteral("Segoe UI Variable Display"));
  f.setPointSize(11);
  app.setFont(f);
  Theme::applyTheme();

  MainWindow w;
  w.setWindowTitle(QStringLiteral("计算器"));
  w.resize(420, 700);
  // 支持启动参数指定模式：--mode=standard|scientific|graphing|programmer|date|converter
  const QStringList args = app.arguments();
  int mode = ID_NAV_STANDARD;
  if (args.contains(QStringLiteral("--mode=scientific"))) mode = ID_NAV_SCIENTIFIC;
  else if (args.contains(QStringLiteral("--mode=graphing"))) mode = ID_NAV_GRAPHING;
  else if (args.contains(QStringLiteral("--mode=programmer"))) mode = ID_NAV_PROGRAMMER;
  else if (args.contains(QStringLiteral("--mode=date"))) mode = ID_NAV_DATE;
  else if (args.contains(QStringLiteral("--mode=converter"))) mode = ID_NAV_CONV_LENGTH;
  w.switchMode(mode);
  w.show();
  return app.exec();
}
