cmake_minimum_required(VERSION 3.16)
project(QtCalculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(QtCalculator WIN32
  src/main.cpp
  src/core/NumberFormat.cpp
  src/core/ExpressionEngine.cpp
  src/core/IntegerEngine.cpp
  src/core/UnitConverter.cpp
  src/core/DateCalculator.cpp
  src/core/HistoryStore.cpp
  src/ui/Theme.cpp
  src/ui/CalcButton.cpp
  src/ui/Keypad.cpp
  src/ui/DisplayWidget.cpp
  src/ui/BitGrid.cpp
  src/ui/ModePanel.cpp
  src/ui/KeypadPanel.cpp
  src/ui/StandardPanel.cpp
  src/ui/ScientificPanel.cpp
  src/ui/ProgrammerPanel.cpp
  src/ui/GraphingPanel.cpp
  src/ui/DatePanel.cpp
  src/ui/ConverterPanel.cpp
  src/ui/HistoryPanel.cpp
  src/ui/MainWindow.cpp
)

target_include_directories(QtCalculator PRIVATE src)
target_link_libraries(QtCalculator PRIVATE Qt6::Widgets)
