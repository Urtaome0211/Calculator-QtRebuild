#include "HistoryPanel.h"
#include "CalcButton.h"
#include "Commands.h"
#include <QBrush>
#include <QFont>
#include <QLabel>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>

// ============================================================
// 历史记录侧栏：列表 + 清除/删除，双击载入数值
// ============================================================
HistoryPanel::HistoryPanel(QWidget* parent) : QWidget(parent) {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);

  // 标题
  auto* title = new QLabel(QStringLiteral("历史记录"), this);
  QFont titleFont = title->font();
  titleFont.setBold(true);
  title->setFont(titleFont);
  layout->addWidget(title);

  // 记录列表：条目文本 "expr\n= result"，字号由 QSS（11pt）统一控制
  list_ = new QListWidget(this);
  list_->setStyleSheet(QStringLiteral("QListWidget { font-size: 11pt; }"));
  layout->addWidget(list_, 1);

  // 按钮行
  auto* btnRow = new QHBoxLayout();
  btnRow->setSpacing(4);
  auto* btnDel = new CalcButton(QStringLiteral("删除选中"), ID_HISTORY_DELETE,
                                Theme::Kind::Function, true, this);
  auto* btnClear = new CalcButton(QStringLiteral("清除全部"), ID_HISTORY_CLEAR,
                                  Theme::Kind::Function, true, this);
  btnRow->addWidget(btnDel);
  btnRow->addWidget(btnClear);
  layout->addLayout(btnRow);

  // 双击载入结果值（index 存于 Qt::UserRole）
  connect(list_, &QListWidget::itemDoubleClicked, this,
          [this](QListWidgetItem* item) {
            const int index = item->data(Qt::UserRole).toInt();
            if (index >= 0 && index < (int)entries_.size())
              emit loadRequested(
                  QString::fromStdWString(entries_[index].result));
          });

  // 删除选中：仅当前行有效时通知宿主
  connect(btnDel, &QPushButton::clicked, this, [this] {
    const int row = list_->currentRow();
    if (row >= 0 && row < (int)entries_.size()) emit deleteRequested(row);
  });

  // 清除全部
  connect(btnClear, &QPushButton::clicked, this, [this] { emit clearRequested(); });
}

void HistoryPanel::refresh(const History::Store& store) {
  entries_ = store.Entries();
  list_->clear();
  for (size_t i = 0; i < entries_.size(); ++i) {
    const History::Entry& e = entries_[i];
    auto* item = new QListWidgetItem(
        QString::fromStdWString(e.expr) + QStringLiteral("\n= ") +
        QString::fromStdWString(e.result));
    item->setData(Qt::UserRole, (int)i);
    list_->addItem(item);
  }
  if (entries_.empty()) {
    // 占位项：灰色、不可选中
    auto* ph = new QListWidgetItem(QStringLiteral("暂无历史记录"), list_);
    ph->setFlags(Qt::NoItemFlags);
    ph->setForeground(QBrush(Theme::textDim()));
    ph->setData(Qt::UserRole, -1);
  }
}
