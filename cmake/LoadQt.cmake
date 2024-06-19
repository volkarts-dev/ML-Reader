find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

qt_standard_project_setup()
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)

add_library(qt_config INTERFACE)
target_compile_definitions(qt_config INTERFACE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
)
