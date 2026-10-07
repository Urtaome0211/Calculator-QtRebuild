# QtRebuild —— 计算器的 Qt 6 重构版

用 **Qt 6.12 (Widgets)** 重构的原 Win32/GDI+ 计算器。核心计算引擎（表达式解析、程序员整数引擎、单位换算、日期算法、历史记录）原样复用自原项目（仅做了最小适配：去掉 Windows 头依赖、历史存储改用 `QStandardPaths`），全部界面改用 Qt Widgets 重写。

## 功能

- **标准模式**：四则运算、百分比（`50+10% = 55`）、1/x、x²、√x、±、记忆（MC/MR/M+/M−/MS）
- **科学模式**：三角函数/反三角、幂/对数、阶乘、双参数函数（`xroot`、`logy`、`mod`）、2nd 切换、DEG/RAD/GRAD
- **图形模式**：3 条函数曲线（QPainter 自绘），滚轮缩放、拖拽平移、双击复位、y 轴自动适配
- **程序员模式**：HEX/DEC/OCT/BIN、QWORD/DWORD/WORD/BYTE、64 位位翻转网格、位运算、四种进制明细
- **日期计算**：日期差值（年/月/日分解 + 总天数）、基准日期加减天/周/月/年
- **单位转换器**：14 类 113 个单位（货币/体积/长度/重量/温度/面积/速度/时间/能量/压力/功率/角度/数据存储/频率），双向即时换算
- **历史记录**：按 `=` 自动记录，右侧面板双击载入，持久化到 `%APPDATA%\QtCalculator\history.txt`（最多 500 条）
- **深浅主题**：启动跟随系统，顶栏按钮一键切换（Fusion 风格 + 全局 QSS）

## 快捷键

数字/四则运算直接输入，`Enter` 等号、`Esc` 清除、`Backspace` 退格、`Del` 清当前项、`F9` 取反；科学模式 `p`→π、`e`→e、`^`→幂、`!`→阶乘；程序员模式 `a`~`f` 输入十六进制。焦点在输入框/下拉框内时按键作为文本输入处理。

## 编译

要求：Qt 6.12（llvm-mingw_64 套件）+ CMake ≥ 3.16 + Ninja。

```bat
set PATH=D:\Qt\6.12.0\llvm-mingw_64\bin;D:\Qt\Tools\llvm-mingw2217_64\bin;D:\Qt\Tools\CMake_64\bin;D:\Qt\Tools\Ninja;%PATH%
cmake -G Ninja -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=D:\Qt\6.12.0\llvm-mingw_64
cmake --build build
```

产物：`build\QtCalculator.exe`。也可用 Qt Creator 打开 `CMakeLists.txt` 直接构建（Kit 选择 llvm-mingw_64 套件；MinGW 套件缺 GCC 15.1 工具链暂不可用）。

## 启动参数

支持命令行指定启动模式（便于快捷方式/自动化测试）：

```
QtCalculator.exe --mode=standard|scientific|graphing|programmer|date|converter
```

## 目录结构

```text
QtRebuild/
├─ CMakeLists.txt             CMake 工程
├─ src/
│  ├─ main.cpp                入口：QApplication + 主题 + 主窗口
│  ├─ Commands.h              命令 ID 表（与原项目一致）
│  ├─ core/                   核心引擎（复用自原项目，纯标准 C++/QtCore）
│  │  ├─ CalcCommon.h         公共头（无 Windows 依赖）
│  │  ├─ ExpressionEngine     表达式解析（分词→调度场→逆波兰求值）
│  │  ├─ IntegerEngine        程序员整数引擎
│  │  ├─ UnitConverter        14 类 113 单位换算
│  │  ├─ DateCalculator       日期算法（Today 改用 <ctime>）
│  │  ├─ HistoryStore         历史持久化（QStandardPaths + QFile）
│  │  └─ NumberFormat         数字格式化
│  └─ ui/                     Qt Widgets 界面
│     ├─ Theme                浅/深调色板 + 全局 QSS
│     ├─ CalcButton/Keypad    按键控件 + 声明式键盘
│     ├─ DisplayWidget        显示区（QPainter）
│     ├─ BitGrid              位翻转网格
│     ├─ ModePanel/KeypadPanel 面板基类 + 键盘面板状态机
│     ├─ Standard/Scientific/Programmer/Graphing/Date/Converter 六面板
│     ├─ HistoryPanel         历史侧栏
│     └─ MainWindow           主窗口（菜单导航/模式栈/停靠栏/键盘路由）
└─ build/                     构建产物（QtCalculator.exe）
```

## 与原项目的差异

- 界面全部使用 Qt Widgets + QSS，自绘仅剩显示区、位网格与图形画布；
- 深色主题下原生下拉框弹出列表样式由 Qt 接管，无旧版限制；
- 历史文件位置从 `%APPDATA%\W11Calc\` 变为 `%APPDATA%\QtCalculator\`（QStandardPaths）；
- 货币汇率为内置参考汇率（非实时），与旧版一致。
