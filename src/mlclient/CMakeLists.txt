add_library(mlclient STATIC
    HttpBody.cpp
    HttpBody.h
    HttpClient.cpp
    HttpClient.h
    HttpRequest.cpp
    HttpRequest.h
    HttpResponse.cpp
    HttpResponse.h
    HttpUserDelegate.h
    MlClient.cpp
    MlClient.h
    MlTokens.cpp
    MlTokens.h
    Tools.cpp
    Tools.h
)

target_link_libraries(mlclient PRIVATE
    project_config
    qt_config
)

target_link_libraries(mlclient PUBLIC
    Qt6::Core
    Qt6::Network
)

target_include_directories(mlclient INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Test exe
add_executable(mlclient_test TestMain.cpp)
target_link_libraries(mlclient_test PRIVATE project_config qt_config)
target_link_libraries(mlclient_test PUBLIC mlclient)
