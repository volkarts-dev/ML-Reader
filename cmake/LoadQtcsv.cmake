set(QTCSV_STATIC_LIB ON CACHE BOOL "Build qtcsv static lib" FORCE)
set(QTCSV_BUILD_TESTS OFF CACHE BOOL "Don't build qtcsv tests" FORCE)
add_subdirectory(third_party/qtcsv)

add_library(qtcsv::qtcsv ALIAS qtcsv)
