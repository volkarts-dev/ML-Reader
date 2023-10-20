add_executable(mlreader "")

target_sources(mlreader PRIVATE
    Application.cpp
    Application.h
    Configuration.cpp
    Configuration.h
    CsvRawData.cpp
    CsvRawData.h
    CsvReader.cpp
    CsvReader.h
    DataModel.cpp
    DataModel.h
    EndpointConfig.cpp
    EndpointConfig.h
    EndpointConfigEditDlg.cpp
    EndpointConfigEditDlg.h
    EndpointConfigEditDlg.ui
    EndpointConfigModel.cpp
    EndpointConfigModel.h
    ListWidgetInput.cpp
    ListWidgetInput.h
    LoaderPage.cpp
    LoaderPage.h
    LoaderPage.ui
    Main.cpp
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    PasswordStore.cpp
    PasswordStore.h
    Tools.cpp
    Tools.h
)

configure_mocs(mlreader)

target_link_libraries(mlreader PRIVATE
    project_config
    mlclient
    Qt5Keychain::Qt5Keychain
    qtcsv::qtcsv
    Qt5::Core
    Qt5::Widgets
)

get_target_property(mlreader_src mlreader SOURCES)
source_group("" FILES ${mlreader_src})
