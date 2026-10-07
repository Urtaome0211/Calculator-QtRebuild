#pragma once
#include <QString>
#include <QColor>

// ============================================================
// Qt 版主题：浅/深两套调色板 + 全局 QSS 样式表
// ============================================================
namespace Theme {
enum class Kind { Number, Function, Accent, Memory, Toggle };

bool isDark();
void setDark(bool dark);
void applyTheme();          // 读取系统主题并应用 qApp 样式表 + Fusion 风格

QColor bg(); QColor card(); QColor num(); QColor fn(); QColor accent();
QColor text(); QColor textDim(); QColor accentText(); QColor border();
QColor hoverNum(); QColor hoverFn(); QColor grid(); QColor curve(int i);
QColor errorColor();

QString colorCss(const QColor& c);
QString appStyleSheet();    // 依据当前主题生成全局样式表
}  // namespace Theme
