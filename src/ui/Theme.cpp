#include "Theme.h"
#include <QApplication>
#include <QSettings>

// ============================================================
// Qt 版主题：浅/深调色板 + 全局 QSS
// ============================================================
namespace Theme {
namespace {

bool g_dark = false;

struct Pal {
  QColor bg, card, num, fn, accent, text, textDim, accentText, border,
      hoverNum, hoverFn, grid, error;
  QColor curve[3];
};

const Pal kLight = {
    QColor(243, 243, 243), QColor(255, 255, 255), QColor(255, 255, 255),
    QColor(249, 249, 249), QColor(0, 103, 192), QColor(0, 0, 0),
    QColor(87, 87, 87), QColor(255, 255, 255), QColor(229, 229, 229),
    QColor(240, 240, 240), QColor(237, 237, 237), QColor(216, 216, 216),
    QColor(196, 43, 28),
    {QColor(0, 103, 192), QColor(196, 43, 28), QColor(15, 123, 15)},
};

const Pal kDark = {
    QColor(32, 32, 32), QColor(43, 43, 43), QColor(46, 46, 46),
    QColor(50, 50, 50), QColor(76, 194, 255), QColor(255, 255, 255),
    QColor(158, 158, 158), QColor(0, 0, 0), QColor(58, 58, 58),
    QColor(63, 63, 63), QColor(67, 67, 67), QColor(58, 58, 58),
    QColor(255, 153, 164),
    {QColor(76, 194, 255), QColor(255, 153, 164), QColor(108, 203, 95)},
};

const Pal& P() { return g_dark ? kDark : kLight; }

bool systemDark() {
#ifdef Q_OS_WIN
  QSettings s(
      QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"),
      QSettings::NativeFormat);
  return s.value(QStringLiteral("AppsUseLightTheme"), 1).toInt() == 0;
#else
  return false;
#endif
}

}  // namespace

bool isDark() { return g_dark; }
void setDark(bool dark) { g_dark = dark; }

void applyTheme() {
  g_dark = systemDark();
  QApplication::setStyle(QStringLiteral("Fusion"));
  qApp->setStyleSheet(appStyleSheet());
}

QColor bg() { return P().bg; }
QColor card() { return P().card; }
QColor num() { return P().num; }
QColor fn() { return P().fn; }
QColor accent() { return P().accent; }
QColor text() { return P().text; }
QColor textDim() { return P().textDim; }
QColor accentText() { return P().accentText; }
QColor border() { return P().border; }
QColor hoverNum() { return P().hoverNum; }
QColor hoverFn() { return P().hoverFn; }
QColor grid() { return P().grid; }
QColor curve(int i) { return P().curve[i % 3]; }
QColor errorColor() { return P().error; }

QString colorCss(const QColor& c) {
  return QStringLiteral("rgb(%1,%2,%3)").arg(c.red()).arg(c.green()).arg(c.blue());
}

QString appStyleSheet() {
  const Pal& p = P();
  const QString bgC = colorCss(p.bg), cardC = colorCss(p.card), numC = colorCss(p.num);
  const QString fnC = colorCss(p.fn), accC = colorCss(p.accent), txtC = colorCss(p.text);
  const QString dimC = colorCss(p.textDim), accTxtC = colorCss(p.accentText);
  const QString bdC = colorCss(p.border), hovNumC = colorCss(p.hoverNum);
  const QString hovFnC = colorCss(p.hoverFn);
  const QString accHover = colorCss(p.accent.lighter(112));
  return QStringLiteral(
      "QWidget { background-color: %1; color: %2; "
      "font-family: \"Segoe UI Variable Display\", \"Segoe UI\", \"Microsoft YaHei\"; }"
      "QPushButton[kind=\"num\"] { background-color: %3; color: %2; border: 1px solid %7; "
      "border-radius: 4px; font-size: 15pt; min-height: 34px; }"
      "QPushButton[kind=\"num\"]:hover { background-color: %9; }"
      "QPushButton[kind=\"num\"]:pressed { background-color: %7; }"
      "QPushButton[kind=\"fn\"] { background-color: %4; color: %2; border: 1px solid %7; "
      "border-radius: 4px; font-size: 15pt; min-height: 34px; }"
      "QPushButton[kind=\"fn\"]:hover { background-color: %10; }"
      "QPushButton[kind=\"fn\"]:pressed { background-color: %7; }"
      "QPushButton[kind=\"accent\"] { background-color: %5; color: %8; border: none; "
      "border-radius: 4px; font-size: 15pt; font-weight: bold; min-height: 34px; }"
      "QPushButton[kind=\"accent\"]:hover { background-color: %11; }"
      "QPushButton[kind=\"accent\"]:pressed { background-color: %5; }"
      "QPushButton[kind=\"mem\"] { background-color: transparent; color: %6; border: none; "
      "font-size: 10pt; min-height: 24px; }"
      "QPushButton[kind=\"mem\"]:hover { background-color: %9; }"
      "QPushButton[kind=\"toggle\"] { background-color: %3; color: %2; border: 1px solid %7; "
      "border-radius: 4px; font-size: 10pt; min-height: 24px; }"
      "QPushButton[kind=\"toggle\"]:hover { background-color: %9; }"
      "QPushButton[kind=\"toggle\"]:checked { background-color: %5; color: %8; border: none; }"
      "QPushButton[small=\"true\"] { font-size: 10pt; min-height: 22px; padding: 2px; }"
      "QLineEdit { background-color: %3; border: 1px solid %7; border-radius: 4px; "
      "padding: 4px 8px; font-size: 11pt; selection-background-color: %5; }"
      "QComboBox { background-color: %3; border: 1px solid %7; border-radius: 4px; "
      "padding: 4px 8px; font-size: 11pt; }"
      "QComboBox QAbstractItemView { background-color: %3; color: %2; "
      "selection-background-color: %9; }"
      "QGroupBox { background-color: %3; border: none; border-radius: 6px; "
      "font-size: 9pt; color: %6; margin-top: 12px; padding: 8px; }"
      "QGroupBox::title { subcontrol-origin: margin; left: 10px; }"
      "QListWidget { background-color: %3; border: none; font-size: 11pt; }"
      "QListWidget::item { padding: 6px; border-radius: 4px; }"
      "QListWidget::item:selected { background-color: %9; color: %2; }"
      "QMenu { background-color: %3; border: 1px solid %7; padding: 4px; }"
      "QMenu::item { padding: 6px 24px; border-radius: 4px; }"
      "QMenu::item:selected { background-color: %9; }"
      "QMenu::separator { height: 1px; background-color: %7; margin: 4px 8px; }"
      "QDockWidget { titlebar-close-icon: none; titlebar-normal-icon: none; }"
      "QDockWidget::title { background-color: %4; padding: 6px; font-size: 10pt; }"
      "QToolButton { background-color: transparent; border: none; border-radius: 4px; "
      "font-size: 12pt; padding: 4px 8px; }"
      "QToolButton:hover { background-color: %9; }"
      "QScrollBar:vertical { background: transparent; width: 10px; }"
      "QScrollBar::handle:vertical { background: %7; border-radius: 5px; min-height: 24px; }"
      "QScrollBar::add-line, QScrollBar::sub-line { height: 0; }")
      .arg(bgC, txtC, numC, fnC, accC, dimC, bdC, accTxtC, hovNumC, hovFnC, accHover);
}

}  // namespace Theme
