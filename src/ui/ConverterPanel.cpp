#include "ConverterPanel.h"
#include "CalcButton.h"
#include "Theme.h"
#include "DisplayWidget.h"
#include "../Commands.h"
#include "../core/UnitConverter.h"
#include "../core/NumberFormat.h"
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QFont>
#include <algorithm>

// ============================================================
// 单位转换器面板（Qt 版）：类别 + 双单位卡片，双向即时换算
// ============================================================
namespace {

// 单向换算：srcCombo -> dstCombo，srcEdit 输入、dstEdit 输出。
// Qt 信号机制天然防重入：textEdited 只在用户编辑时发出，
// setText 编程赋值不触发它，故不存在旧版 EN_CHANGE 双向死循环。
void convertOne(QLineEdit* srcEdit, QLineEdit* dstEdit, QComboBox* srcCombo,
                QComboBox* dstCombo, const QString& catId,
                DisplayWidget* display, bool& updating) {
  if (updating) return;

  QString clean = srcEdit->text();
  clean.remove(QLatin1Char(','));
  clean.remove(QLatin1Char(' '));
  const std::wstring cat = catId.toStdWString();
  const std::wstring fromUnit = srcCombo->currentText().toStdWString();
  const std::wstring toUnit = dstCombo->currentText().toStdWString();

  // 换算率 alt 行：1 源单位 = 换算率 目标单位
  const auto updateRateLine = [&]() {
    double rate = 0;
    if (UnitConv::Convert(cat, fromUnit, toUnit, 1.0, rate)) {
      display->setAltLines(
          {QStringLiteral("1 ") + QString::fromStdWString(fromUnit) +
           QStringLiteral(" = ") +
           QString::fromStdWString(NumFmt::FormatDouble(rate)) +
           QStringLiteral(" ") + QString::fromStdWString(toUnit)});
    }
  };

  if (clean.isEmpty()) {
    updating = true;  // 防御性屏蔽：即使 setText 不触发 textEdited 也保持旧版语义
    dstEdit->clear();
    updating = false;
    updateRateLine();
    return;
  }
  bool ok = false;
  const double v = clean.toDouble(&ok);
  if (!ok) {
    updating = true;
    dstEdit->clear();
    updating = false;
    display->setResult(QStringLiteral("输入无效"), true);
    return;
  }
  double out = 0;
  if (!UnitConv::Convert(cat, fromUnit, toUnit, v, out)) {
    updating = true;
    dstEdit->clear();
    updating = false;
    display->setResult(QStringLiteral("输入无效"), true);
    return;
  }
  updating = true;
  dstEdit->setText(QString::fromStdWString(NumFmt::FormatDouble(out)));
  updating = false;

  display->setExpr(clean + QLatin1Char(' ') + QString::fromStdWString(fromUnit));
  display->setResult(QString::fromStdWString(NumFmt::FormatDouble(out)) +
                         QLatin1Char(' ') + QString::fromStdWString(toUnit),
                     false);
  updateRateLine();
}

}  // namespace

ConverterPanel::ConverterPanel(AppContext& ctx, QWidget* parent)
    : ModePanel(ctx, parent) {
  auto* root = new QVBoxLayout(this);
  root->setContentsMargins(8, 8, 8, 8);
  root->setSpacing(6);

  // 类别下拉：填充全部类别名并选中 catId_ 对应项
  comboCat_ = new QComboBox(this);
  const auto& cats = UnitConv::Categories();
  const std::wstring curId = catId_.toStdWString();
  int catSel = 0;
  for (size_t i = 0; i < cats.size(); i++) {
    comboCat_->addItem(QString::fromStdWString(cats[i].name));
    if (cats[i].id == curId) catSel = static_cast<int>(i);
  }
  comboCat_->setCurrentIndex(catSel);
  root->addWidget(comboCat_);

  // 中间：左卡片 + 交换按钮 + 右卡片
  auto* cardsRow = new QHBoxLayout();
  cardsRow->setSpacing(6);

  const auto makeCard = [this](QComboBox*& combo, QLineEdit*& edit) {
    auto* card = new QGroupBox(this);
    card->setTitle(QString());
    auto* v = new QVBoxLayout(card);
    v->setContentsMargins(8, 8, 8, 8);
    v->setSpacing(6);
    combo = new QComboBox(card);
    edit = new QLineEdit(card);
    QFont f = edit->font();
    f.setPointSize(16);
    edit->setFont(f);
    v->addWidget(combo);
    v->addWidget(edit);
    return card;
  };

  QGroupBox* cardFrom = makeCard(comboFrom_, editFrom_);
  QGroupBox* cardTo = makeCard(comboTo_, editTo_);
  editFrom_->setText(QStringLiteral("1"));

  auto* btnSwap = new CalcButton(QStringLiteral("⇄"), IDK_CV_SWAP,
                                 Theme::Kind::Function, false, this);
  btnSwap->setFixedWidth(44);

  cardsRow->addWidget(cardFrom, 1);
  cardsRow->addWidget(btnSwap, 0, Qt::AlignVCenter);
  cardsRow->addWidget(cardTo, 1);
  root->addLayout(cardsRow);
  root->addStretch(1);

  // 信号：currentIndexChanged / textEdited 只在用户操作时发出，
  // 程序 setText / setCurrentIndex 期间由 updating_ 屏蔽，杜绝重入
  connect(comboCat_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [this](int index) {
            const auto& list = UnitConv::Categories();
            if (index < 0 || index >= static_cast<int>(list.size())) return;
            catId_ = QString::fromStdWString(
                list[static_cast<size_t>(index)].id);
            reloadUnits();
            editFrom_->setText(QStringLiteral("1"));
            convert();
          });
  connect(comboFrom_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &ConverterPanel::convert);
  connect(comboTo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &ConverterPanel::convert);
  connect(editFrom_, &QLineEdit::textEdited, this, &ConverterPanel::convert);
  connect(editTo_, &QLineEdit::textEdited, this,
          &ConverterPanel::convertReverse);
  connect(btnSwap, &QPushButton::clicked, this, &ConverterPanel::swapUnits);

  reloadUnits();
  convert();
}

void ConverterPanel::onShow() { convert(); }

void ConverterPanel::setCategory(const QString& id) {
  catId_ = id;
  const auto& cats = UnitConv::Categories();
  const std::wstring wid = id.toStdWString();
  for (size_t i = 0; i < cats.size(); i++) {
    if (cats[i].id == wid) {
      // setCurrentIndex 触发槽自动 reloadUnits + convert，无需重复
      comboCat_->setCurrentIndex(static_cast<int>(i));
      return;
    }
  }
  // 下拉中找不到对应类别时直接重载单位
  reloadUnits();
}

void ConverterPanel::reloadUnits() {
  const UnitConv::Category* cat = UnitConv::Find(catId_.toStdWString());
  if (!cat) return;
  updating_ = true;  // 屏蔽期间 addItem / setCurrentIndex 可能触发的信号
  comboFrom_->clear();
  comboTo_->clear();
  for (const auto& u : cat->units) {
    const QString name = QString::fromStdWString(u);
    comboFrom_->addItem(name);
    comboTo_->addItem(name);
  }
  comboFrom_->setCurrentIndex(0);
  comboTo_->setCurrentIndex(
      std::min<int>(1, static_cast<int>(cat->units.size()) - 1));
  editFrom_->setText(QStringLiteral("1"));
  editTo_->clear();
  updating_ = false;
}

void ConverterPanel::convert() {
  convertOne(editFrom_, editTo_, comboFrom_, comboTo_, catId_, ctx_.display,
             updating_);
}

void ConverterPanel::convertReverse() {
  convertOne(editTo_, editFrom_, comboTo_, comboFrom_, catId_, ctx_.display,
             updating_);
}

void ConverterPanel::swapUnits() {
  updating_ = true;
  const int iFrom = comboFrom_->currentIndex();
  const int iTo = comboTo_->currentIndex();
  const QString vFrom = editFrom_->text();
  const QString vTo = editTo_->text();
  comboFrom_->setCurrentIndex(iTo);
  comboTo_->setCurrentIndex(iFrom);
  editFrom_->setText(vTo);
  editTo_->setText(vFrom);
  updating_ = false;
  convert();
}
