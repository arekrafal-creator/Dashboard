cmake_minimum_required(VERSION 3.16)
project(WarehouseDashboard VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- Qt6 (Widgets + SQL) -----------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Core Gui)

add_executable(WarehouseDashboard
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/DatabaseManager.h
    src/DatabaseManager.cpp
    src/ConfigManager.h
    src/ConfigManager.cpp
    src/QueryPanel.h
    src/QueryPanel.cpp
    src/ClockWidget.h
    src/ClockWidget.cpp
    src/ConnectionSettingsDialog.h
    src/ConnectionSettingsDialog.cpp
    src/PanelEditDialog.h
    src/PanelEditDialog.cpp
    src/PanelManagerDialog.h
    src/PanelManagerDialog.cpp
)

target_link_libraries(WarehouseDashboard PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Core
    Qt6::Gui
)

# Po zbudowaniu kopiujemy pliki konfiguracyjne obok pliku wykonywalnego,
# żeby program od razu znalazł queries.json / db_config.json / style.qss.
add_custom_command(TARGET WarehouseDashboard POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/config $<TARGET_FILE_DIR:WarehouseDashboard>/config
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:WarehouseDashboard>/resources
)
