#include "HistoryStore.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

// ============================================================
// 计算历史（Qt 版）：内存列表 + QStandardPaths 持久化（UTF-8）
// 文件格式与旧版一致：UTF-8（带 BOM），每行 expr、result、time 以 '\t' 分隔
// ============================================================
namespace History {

Store::Store() {
  path_ = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
              .filePath(QStringLiteral("history.txt"))
              .toStdWString();
  Load();
}

namespace {
std::wstring NowTime() {
  return QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss")).toStdWString();
}

std::wstring Sanitize(std::wstring s) {
  for (auto& c : s) {
    if (c == L'\t' || c == L'\r' || c == L'\n') c = L' ';
  }
  return s;
}
}  // namespace

void Store::Load() {
  entries_.clear();
  QFile f(QString::fromStdWString(path_));
  if (!f.open(QIODevice::ReadOnly)) return;
  QByteArray data = f.readAll();
  f.close();
  // 跳过 UTF-8 BOM
  if (data.startsWith("\xEF\xBB\xBF")) data.remove(0, 3);
  const QString text = QString::fromUtf8(data);
  const QStringList lines = text.split(QLatin1Char('\n'));
  for (const QString& line : lines) {
    const QStringList fields = line.split(QLatin1Char('\t'));
    if (fields.size() < 3) continue;
    entries_.push_back({fields[0].toStdWString(), fields[1].toStdWString(),
                        fields[2].toStdWString()});
  }
}

void Store::Save() {
  const QFileInfo info(QString::fromStdWString(path_));
  QDir().mkpath(info.absolutePath());
  QFile f(QString::fromStdWString(path_));
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
  QStringList lines;
  for (const auto& e : entries_) {
    lines << QString::fromStdWString(Sanitize(e.expr)) + QLatin1Char('\t') +
                 QString::fromStdWString(Sanitize(e.result)) + QLatin1Char('\t') +
                 QString::fromStdWString(Sanitize(e.time));
  }
  f.write("\xEF\xBB\xBF");   // UTF-8 BOM
  if (!lines.isEmpty()) f.write(lines.join(QLatin1Char('\n')).toUtf8() + "\n");
  f.close();
}

void Store::Add(const std::wstring& expr, const std::wstring& result) {
  entries_.insert(entries_.begin(), {expr, result, NowTime()});
  while (entries_.size() > kMax) entries_.pop_back();
  Save();
}

void Store::Clear() {
  entries_.clear();
  Save();
}

void Store::Remove(size_t index) {
  if (index >= entries_.size()) return;
  entries_.erase(entries_.begin() + (ptrdiff_t)index);
  Save();
}

}  // namespace History
