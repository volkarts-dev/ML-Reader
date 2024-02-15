set(BUILD_WITH_QT5 OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/qtkeychain)

add_library(qt5keychain-iface INTERFACE)
add_library(qtkeychain::qt5keychain ALIAS qt5keychain-iface)

target_include_directories(qt5keychain-iface SYSTEM INTERFACE third_party/qtkeychain)
target_compile_definitions(qt5keychain-iface INTERFACE QTKEYCHAIN_NO_EXPORT)
target_link_libraries(qt5keychain-iface INTERFACE qt5keychain)
