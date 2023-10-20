set(STATIC_LIB ON)
set(USE_QT4 OFF)
set(BUILD_TESTS OFF)
add_subdirectory(third_party/qtcsv)

add_library(qtcsv::qtcsv ALIAS qtcsv)
