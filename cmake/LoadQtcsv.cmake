set(STATIC_LIB ON CACHE BOOL "Build qtcsv static lib" FORCE)
set(USE_QT4 OFF CACHE BOOL "Don't build qtcsv against qt4" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "Don't build qtcsv tests" FORCE)
add_subdirectory(third_party/qtcsv)

add_library(qtcsv::qtcsv ALIAS qtcsv)
