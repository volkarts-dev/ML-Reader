add_library(mlclient STATIC "")

target_sources(mlclient PRIVATE
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

configure_mocs(mlclient
    MlClient.moc
)

target_link_libraries(mlclient PRIVATE
    project_config
)

target_link_libraries(mlclient PUBLIC
    Qt5::Core
    Qt5::Network
)

target_include_directories(mlclient INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)






add_executable(mlclient_test "")

target_sources(mlclient_test PRIVATE
    TestMain.cpp
)

target_link_libraries(mlclient_test PRIVATE
    project_config
)

target_link_libraries(mlclient_test PUBLIC
    mlclient
)
