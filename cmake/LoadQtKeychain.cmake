set(BUILD_WITH_QT5 OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/qtkeychain)

add_library(qtkeychain::qt5keychain ALIAS qt5keychain)
