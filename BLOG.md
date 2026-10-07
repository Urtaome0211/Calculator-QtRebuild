# 从 253KB 到 498KB：我用 Qt 6 重写了那个 5700 行的 Win32 计算器

> 一个能跑的程序为什么要重写？因为"能跑"和"好维护"之间，隔着一整层自绘控件的距离。
> 本文记录把纯 Win32 + GDI+ 计算器重构为 Qt 6 Widgets 版的完整过程：核心引擎零改动复用、UI 全部推倒重来，以及两个让我半夜盯着退出码怀疑人生的新 Bug。

---

## 1. 起点：一个"小而美"的 Win32 账本

上一篇文章里，我用纯 Win32 + GDI+ 从零写了一个仿 Windows 11 计算器。它的成绩单：

| 指标 | 数值 |
| --- | --- |
| 源码 | 45 个文件、5703 行（`core` 引擎 1754 行 / `ui` 界面 3818 行） |
| 可执行文件 | 253 KB，零运行时依赖，双击即用 |
| 功能 | 标准/科学/图形/程序员/日期计算 6 模式 + 14 类 113 单位转换器 + 历史记录 |
| 质量 | 61 项核心单元测试全过，但 UI 层有 3818 行手绘代码要维护 |

真正的问题在第 5 行：**3818 行 UI 代码里，几乎每一行都在跟 Win32 搏斗**。按钮要自己画圆角、自己处理悬停态；键盘要写布局算法；下拉框深色主题永远差一口气。功能都齐了，但每加一个模式都要再战一场 `WM_PAINT`。

于是需求来了：**用 Qt 重构，原文件一字不动**。

## 2. 重构策略：引擎零改动，UI 推倒重来

幸运的是，第一版就坚持了"核心引擎与 UI 分离"的设计——`src/core` 里 12 个文件全是纯计算逻辑，连一个 `HWND` 都没有。这让重构变成了一次干净的换皮手术：

| 层 | 处置 | 说明 |
| --- | --- | --- |
| `ExpressionEngine`（分词→调度场→逆波兰求值） | ✅ 原样复用 | 零改动 |
| `IntegerEngine`（任意进制 + 位运算） | ✅ 原样复用 | 零改动 |
| `UnitConverter`（14 类 113 单位） | ✅ 原样复用 | 零改动 |
| `DateCalculator` | ✏️ 改 1 处 | `Today()` 从 `GetLocalTime` 换成 `<ctime>` |
| `HistoryStore` | ✏️ 重写 | 文件 API 换 `QStandardPaths + QFile` |
| `NumberFormat` | ✅ 原样复用 | 零改动 |
| 全部 UI | 🔥 推倒重写 | Win32 自绘 → Qt Widgets + QSS |

核心层唯一的硬伤是那个 `#include "Common.h"`（里面拖着 `windows.h`）——用一个只含标准库的 `CalcCommon.h` 换掉即可。**"引擎不依赖 UI"这六个字，值回全部架构设计的时间。**

## 3. UI 重写：3818 行手绘代码去哪儿了

### 3.1 QSS 替掉了整本"自绘控件集"

旧版 `CtrlButton` 要处理悬停、按下、选中三态，圆角还要用 GDI+ 路径一条条画。Qt 版一个属性选择器搞定：

```css
QPushButton[kind="num"] { background-color: rgb(255,255,255); border: 1px solid #E5E5E5;
                           border-radius: 4px; font-size: 15pt; min-height: 34px; }
QPushButton[kind="num"]:hover  { background-color: rgb(240,240,240); }
QPushButton[kind="accent"] { background-color: rgb(0,103,192); color: white; border: none; }
QPushButton[kind="toggle"]:checked { background-color: rgb(0,103,192); color: white; }
```

深浅主题只是两套调色板 + 重新生成样式表——旧版要实现这个，得在每个控件的绘制代码里判断 `IsDark()`。

### 3.2 声明式键盘从手写布局变成 QGridLayout

旧版 `Keypad` 要自己算列宽、行高、MoveWindow 布局。Qt 版保留 `KeyCell/KeyRow` 声明式规格，布局交给 QGridLayout 自动铺满，代码直接少一半。

### 3.3 只留三处"必须自绘"

- **显示区**（表达式/结果/次要行 + 字号自适应收缩）：`QPainter::drawText` + `QFontMetrics` 循环缩号；
- **位翻转网格**（64 个可点击方块）：`paintEvent` + `hitTest`；
- **函数曲线画布**（网格/刻度/曲线断线）：`QPainter::drawLine`，滚轮缩放、拖拽平移逻辑照搬旧版。

## 4. 旧 Bug 的 Qt 答案：`textEdited` 救了转换器

旧版有个上过博客"坑六"的经典崩溃：转换器的 `Convert()` 用 `SetWindowTextW` 清空对面输入框 → 同步触发 `EN_CHANGE` 通知 → 反向 `Convert()` → 再清另一侧 → **无限递归，栈溢出（0xC00000FD）**，最后靠 `updating_` 重入锁才压住。

Qt 版没写一行锁，因为信号语义变了：

```cpp
// 只在"用户编辑"时发出；程序 setText 不触发 —— 死循环在机制上不存在
connect(editFrom_, &QLineEdit::textEdited, this, &ConverterPanel::convert);
connect(editTo_,   &QLineEdit::textEdited, this, &ConverterPanel::convertReverse);
```

程序化写入不会反向触发换算，双向清空的递归链从根源上被掐断。**同样的架构问题，换一个框架可能直接消失——这是重构最大的红利。**

## 5. 新坑一：构造函数里的"纯虚函数"幽灵

编译很顺利，启动即崩。退出码 `0xC0000409`，Qt 日志冷冷地打出一行：

```
libc++abi: Pure virtual function called!
```

我盯着这行看了三分钟——哪来的纯虚函数？答案藏在 C++ 最反直觉的规则里：**构造函数中虚派发不生效**。基类 `KeypadPanel` 的构造函数里调用了 `buildLayout()` 来搭键盘——而它是纯虚函数，被子类 `StandardPanel` 覆写。基类构造时对象还不是"子类"，调用直接撞上纯虚实现 → abort。

修复是标准的两阶段初始化：

```cpp
// 基类构造函数：只建空壳，不碰虚函数
KeypadPanel::KeypadPanel(AppContext& ctx, QWidget* parent) : ModePanel(ctx, parent) {}

// 子类构造函数体末尾显式调用（此时虚派发已生效）
StandardPanel::StandardPanel(AppContext& ctx, QWidget* parent)
    : KeypadPanel(ctx, parent) { setupPanel(); }
```

教训一句话：**基类构造函数里永远不要调用虚函数**——编译器不报错，运行期直接送你一个 0xC0000409。

## 6. 新坑二：llvm-mingw 的部署盲区

编译出来的 `QtCalculator.exe` 双击就退。排查记录：

1. 退出码指向运行库缺失，但 `llvm-objdump` 显示依赖的是 **`libc++.dll`**——llvm-mingw 套件用 libc++ 而非 MinGW 惯例的 libstdc++，直觉补的库全错了；
2. `windeployqt` 报了句 "Unable to find the platform plugin" 就罢工，`platforms\qwindows.dll` 得手动拷；
3. 最后凑齐：`Qt6Core/Gui/Widgets.dll` + `libc++/libunwind/libwinpthread` + `platforms\qwindows.dll`，exe 才脱离 PATH 独立运行。

（顺带一个小插曲：有次用相对路径拷 DLL 时工作目录不对，四个 DLL 被送进了**原项目**的 build 目录——幸好及时清掉，没破坏"原文件一字不动"的约定。教训：脚本里永远用绝对路径。）

## 7. 验证：不信"能编译"，只信"能算账"

- **六模式独立启动**：新增 `--mode=` 启动参数，六个模式各起一遍，窗口尺寸符合各自最小值（436/736/656/776 px）——模式切换与最小尺寸逻辑全过；
- **端到端计算**：模拟真实键盘输入 `12 + 3 =`，历史文件如实写下 `12 + 3 → 15`；
- **转换器**：旧版的崩溃重灾区，切换 + 输入交互全程无恙；
- **主题切换**：背景像素从浅色 `#F3F3F3` 变成深色 `#202020`，一键生效。

## 8. 两版对比：重构值不值？

| 维度 | Win32 + GDI+ | Qt 6 Widgets |
| --- | --- | --- |
| 源码规模 | 45 文件 / 5703 行 | 45 文件（核心 12 个复用，UI 显著更短） |
| exe 体积 | 253 KB，零依赖 | 498 KB + 运行库部署（约 20 MB） |
| 样式/主题 | 全手绘，深浅主题改到怀疑人生 | QSS 两套调色板，切换一行代码 |
| 原生控件（下拉/输入框） | 深色适配不完整 | Qt 统一接管 |
| 坑的类型 | 宏污染、聚合初始化、消息递归 | 虚派发、运行库部署 |
| 可维护性 | 每像素可控，代价是每像素都要管 | 平台能力接住 90%，专注业务 |

结论：**如果是教学、极致体积或完全掌控每像素——Win32 无出其右；如果要功能迭代、主题切换、跨平台——Qt 让"写界面"重新变成一件不痛苦的事。** 而无论选哪个，把计算引擎做成 UI 无关的独立层，都是这次重构能半天完成的唯一原因。

## 9. 结语

重构后的感受很奇妙：5703 行代码里最值钱的 1754 行，一行没动；推倒重来的 3818 行界面，在 Qt 手里变得温顺。这大概就是架构的复利——**当初多花半小时把引擎和 UI 拆开，今天就能用一顿饭的功夫换掉整个界面层**。

完整代码在 `QtRebuild/`（`CMakeLists.txt` 一键构建，`--mode=` 直达任意模式），原版 Win32 项目原封不动保留在隔壁。两个版本、同一颗计算引擎，欢迎对照阅读。

> 彩蛋：重构中顺手发现旧版博客"坑六"的 EN_CHANGE 递归崩溃，在 Qt 的信号语义下根本不可能发生——**换框架不是逃避 Bug，是换个角度重新理解 Bug。**
