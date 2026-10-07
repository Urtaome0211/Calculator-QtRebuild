#include "DatePanel.h"
#include "CalcButton.h"
#include "DisplayWidget.h"
#include "Theme.h"
#include "../Commands.h"
#include "../core/DateCalculator.h"
#include "../core/NumberFormat.h"
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QComboBox>

// ============================================================
// 日期计算面板（Qt 版）：日期差值 / 基准日期加减（天/周/月/年）
// ============================================================
namespace {

// 头文件中没有声明切换按钮成员，按命令 ID 从子控件中查找
CalcButton* findButton(QWidget* panel, int cmd) {
  const auto buttons = panel->findChildren<CalcButton*>();
  for (CalcButton* b : buttons) {
    if (b->cmd() == cmd) return b;
  }
  return nullptr;
}

QString toText(const std::wstring& s) { return QString::fromStdWString(s); }

}  // namespace

DatePanel::DatePanel(AppContext& ctx, QWidget* parent)
    : ModePanel(ctx, parent) {
  using K = Theme::Kind;
  auto* root = new QVBoxLayout(this);
  root->setContentsMargins(8, 8, 8, 8);
  root->setSpacing(6);

  // 顶部模式切换行
  auto* modeRow = new QHBoxLayout();
  modeRow->setContentsMargins(0, 0, 0, 0);
  modeRow->setSpacing(6);
  auto* btnDiff = new CalcButton(QStringLiteral("日期差值"), IDK_DT_MODE_DIFF, K::Toggle, true, this);
  auto* btnAdd = new CalcButton(QStringLiteral("加减天数"), IDK_DT_MODE_ADD, K::Toggle, true, this);
  modeRow->addWidget(btnDiff, 1);
  modeRow->addWidget(btnAdd, 1);
  root->addLayout(modeRow);
  connect(btnDiff, &QPushButton::clicked, this, [this] {
    modeDiff_ = true;
    applyMode();
  });
  connect(btnAdd, &QPushButton::clicked, this, [this] {
    modeDiff_ = false;
    applyMode();
  });

  DateCalc::Date today = DateCalc::Today();

  // 日期差页面：开始日期 / 结束日期
  diffPage_ = new QWidget(this);
  auto* diffLayout = new QHBoxLayout(diffPage_);
  diffLayout->setContentsMargins(0, 0, 0, 0);
  diffLayout->setSpacing(6);
  auto* fromBox = new QGroupBox(QStringLiteral("开始日期"), diffPage_);
  auto* fromLay = new QVBoxLayout(fromBox);
  fromLay->setContentsMargins(4, 4, 4, 4);
  editFrom_ = new QLineEdit(fromBox);
  editFrom_->setText(toText(DateCalc::Format(DateCalc::AddDays(today, -30))));
  fromLay->addWidget(editFrom_);
  diffLayout->addWidget(fromBox, 1);
  auto* toBox = new QGroupBox(QStringLiteral("结束日期"), diffPage_);
  auto* toLay = new QVBoxLayout(toBox);
  toLay->setContentsMargins(4, 4, 4, 4);
  editTo_ = new QLineEdit(toBox);
  editTo_->setText(toText(DateCalc::Format(today)));
  toLay->addWidget(editTo_);
  diffLayout->addWidget(toBox, 1);
  root->addWidget(diffPage_);

  // 加减页面：基准日期 / 加减 / 数量 / 单位
  addPage_ = new QWidget(this);
  auto* addLayout = new QHBoxLayout(addPage_);
  addLayout->setContentsMargins(0, 0, 0, 0);
  addLayout->setSpacing(6);
  auto* baseBox = new QGroupBox(QStringLiteral("基准日期"), addPage_);
  auto* baseLay = new QVBoxLayout(baseBox);
  baseLay->setContentsMargins(4, 4, 4, 4);
  editBase_ = new QLineEdit(baseBox);
  editBase_->setText(toText(DateCalc::Format(today)));
  baseLay->addWidget(editBase_);
  addLayout->addWidget(baseBox, 1);
  auto* signBox = new QGroupBox(QStringLiteral("加减"), addPage_);
  auto* signLay = new QVBoxLayout(signBox);
  signLay->setContentsMargins(4, 4, 4, 4);
  comboSign_ = new QComboBox(signBox);
  comboSign_->addItem(QStringLiteral("+"));
  comboSign_->addItem(QStringLiteral("−"));
  comboSign_->setCurrentIndex(0);
  signLay->addWidget(comboSign_);
  addLayout->addWidget(signBox, 1);
  auto* amountBox = new QGroupBox(QStringLiteral("数量"), addPage_);
  auto* amountLay = new QVBoxLayout(amountBox);
  amountLay->setContentsMargins(4, 4, 4, 4);
  editAmount_ = new QLineEdit(amountBox);
  editAmount_->setText(QStringLiteral("1"));
  amountLay->addWidget(editAmount_);
  addLayout->addWidget(amountBox, 1);
  auto* unitBox = new QGroupBox(QStringLiteral("单位"), addPage_);
  auto* unitLay = new QVBoxLayout(unitBox);
  unitLay->setContentsMargins(4, 4, 4, 4);
  comboUnit_ = new QComboBox(unitBox);
  comboUnit_->addItem(QStringLiteral("天"));
  comboUnit_->addItem(QStringLiteral("周"));
  comboUnit_->addItem(QStringLiteral("月"));
  comboUnit_->addItem(QStringLiteral("年"));
  comboUnit_->setCurrentIndex(0);
  unitLay->addWidget(comboUnit_);
  addLayout->addWidget(unitBox, 1);
  root->addWidget(addPage_);

  // 计算 / 重置按钮行
  auto* btnRow = new QHBoxLayout();
  btnRow->setContentsMargins(0, 0, 0, 0);
  btnRow->setSpacing(6);
  auto* btnCalc = new CalcButton(QStringLiteral("计算"), IDK_DT_CALC, K::Accent, false, this);
  auto* btnReset = new CalcButton(QStringLiteral("重置"), IDK_DT_RESET, K::Function, false, this);
  btnRow->addWidget(btnCalc, 1);
  btnRow->addWidget(btnReset, 1);
  root->addLayout(btnRow);
  connect(btnCalc, &QPushButton::clicked, this, &DatePanel::doCalc);
  connect(btnReset, &QPushButton::clicked, this, &DatePanel::resetInputs);

  root->addStretch(1);

  applyMode();
}

void DatePanel::applyMode() {
  diffPage_->setVisible(modeDiff_);
  addPage_->setVisible(!modeDiff_);
  if (CalcButton* b = findButton(this, IDK_DT_MODE_DIFF)) b->setChecked(modeDiff_);
  if (CalcButton* b = findButton(this, IDK_DT_MODE_ADD)) b->setChecked(!modeDiff_);
  ctx_.display->setExpr(QString());
  ctx_.display->setResult(QString(), false);
  ctx_.display->setAltLines({QString(), QString()});
}

void DatePanel::doCalc() {
  if (modeDiff_) {
    DateCalc::Date a, b;
    if (!DateCalc::Parse(editFrom_->text().toStdWString(), a) ||
        !DateCalc::Parse(editTo_->text().toStdWString(), b) ||
        !DateCalc::IsValid(a) || !DateCalc::IsValid(b)) {
      ctx_.display->setResult(QStringLiteral("无效日期"), true);
      return;
    }
    long long total;
    int years, months, days;
    DateCalc::Diff(a, b, total, years, months, days);
    const QString fa = toText(DateCalc::Format(a));
    const QString fb = toText(DateCalc::Format(b));
    ctx_.display->setExpr(fa + QStringLiteral(" → ") + fb);
    ctx_.display->setResult(
        toText(NumFmt::FormatDouble((double)total)) + QStringLiteral(" 天"), false);
    QString detail;
    if (years > 0) detail += QString::number(years) + QStringLiteral(" 年 ");
    if (months > 0 || years > 0) detail += QString::number(months) + QStringLiteral(" 月 ");
    detail += QString::number(days) + QStringLiteral(" 天");
    ctx_.display->setAltLines({
        QStringLiteral("相差 ") + detail,
        fa + QStringLiteral(" (") + toText(DateCalc::WeekdayName(a)) +
            QStringLiteral(") → ") + fb + QStringLiteral(" (") +
            toText(DateCalc::WeekdayName(b)) + QStringLiteral(")")});
  } else {
    DateCalc::Date base;
    if (!DateCalc::Parse(editBase_->text().toStdWString(), base) ||
        !DateCalc::IsValid(base)) {
      ctx_.display->setResult(QStringLiteral("无效日期"), true);
      return;
    }
    QString clean = editAmount_->text();
    clean.remove(QLatin1Char(','));
    clean.remove(QLatin1Char(' '));
    bool ok = false;
    const double amount = clean.toDouble(&ok);
    if (clean.isEmpty() || !ok) {
      ctx_.display->setResult(QStringLiteral("无效数字"), true);
      return;
    }
    const int sign = comboSign_->currentIndex() == 0 ? 1 : -1;
    const long long n = (long long)amount * sign;
    QString unitName = QStringLiteral("天");
    DateCalc::Date result;
    switch (comboUnit_->currentIndex()) {
      case 0: result = DateCalc::AddDays(base, n); break;
      case 1: result = DateCalc::AddDays(base, n * 7); unitName = QStringLiteral("周"); break;
      case 2: result = DateCalc::AddMonths(base, n); unitName = QStringLiteral("月"); break;
      default: result = DateCalc::AddYears(base, n); unitName = QStringLiteral("年"); break;
    }
    const long long totalDays = DateCalc::ToDays(result) - DateCalc::ToDays(base);
    const QString fb = toText(DateCalc::Format(base));
    ctx_.display->setExpr(fb + QStringLiteral(" ") +
                          (sign > 0 ? QStringLiteral("+") : QStringLiteral("−")) +
                          QStringLiteral(" ") + clean + QStringLiteral(" ") + unitName);
    ctx_.display->setResult(toText(DateCalc::Format(result)) + QStringLiteral(" (") +
                                toText(DateCalc::WeekdayName(result)) + QStringLiteral(")"),
                            false);
    ctx_.display->setAltLines({
        QStringLiteral("共加 ") + QString::number(totalDays) + QStringLiteral(" 天"),
        QStringLiteral("基准日期 ") + fb});
  }
}

void DatePanel::resetInputs() {
  DateCalc::Date today = DateCalc::Today();
  editFrom_->setText(toText(DateCalc::Format(DateCalc::AddDays(today, -30))));
  editTo_->setText(toText(DateCalc::Format(today)));
  editBase_->setText(toText(DateCalc::Format(today)));
  editAmount_->setText(QStringLiteral("1"));
  comboUnit_->setCurrentIndex(0);
  comboSign_->setCurrentIndex(0);
  applyMode();
}

void DatePanel::onShow() { applyMode(); }
