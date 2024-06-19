add_executable(mlreader
    Application.cpp
    Application.h
    CsvRawData.cpp
    CsvRawData.h
    CsvReader.cpp
    CsvReader.h
    DataModel.cpp
    DataModel.h
    DynamicForm.cpp
    DynamicForm.h
    EditorPage.cpp
    EditorPage.h
    EditorPage.ui
    EndpointConfig.cpp
    EndpointConfig.h
    EndpointConfigEditDlg.cpp
    EndpointConfigEditDlg.h
    EndpointConfigEditDlg.ui
    EndpointConfigItemDelegate.cpp
    EndpointConfigItemDelegate.h
    EndpointConfigModel.cpp
    EndpointConfigModel.h
    EndpointSelector.cpp
    EndpointSelector.h
    EndpointSelector.ui
    InputDropZone.cpp
    InputDropZone.h
    ListWidgetInput.cpp
    ListWidgetInput.h
    LoaderPage.cpp
    LoaderPage.h
    LoaderPage.ui
    Main.cpp
    Main.qrc
    Main.rc
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    MlClientTools.cpp
    MlClientTools.h
    PasswordStore.cpp
    PasswordStore.h
    QueryPage.cpp
    QueryPage.h
    QueryPage.ui
    Tools.cpp
    Tools.h
    UserSettings.cpp
    UserSettings.h
    Version.h.in
)

setup_git_version_config(mlreader Version.h.in)

target_link_libraries(mlreader PRIVATE
    project_config
    qt_config
    mlclient
    qtkeychain::qt6keychain
    qtcsv::qtcsv
    Qt6::Core
    Qt6::Widgets
)

set_target_properties(mlreader PROPERTIES
    WIN32_EXECUTABLE ON
)
