set(BUILD_WITH_QT6 ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/qtkeychain)

add_library(qt6keychain-iface INTERFACE)
add_library(qtkeychain::qt6keychain ALIAS qt6keychain-iface)

target_include_directories(qt6keychain-iface SYSTEM INTERFACE third_party/qtkeychain)
target_compile_definitions(qt6keychain-iface INTERFACE QTKEYCHAIN_NO_EXPORT)
target_link_libraries(qt6keychain-iface INTERFACE qt6keychain)
