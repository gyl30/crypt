
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

aux_source_directory(${PROJECT_SOURCE_DIR}/qt CLI_SOURCE_FILE)
aux_source_directory(${PROJECT_SOURCE_DIR}/file CLI_SOURCE_FILE)
aux_source_directory(${PROJECT_SOURCE_DIR}/crypt CLI_SOURCE_FILE)
aux_source_directory(${PROJECT_SOURCE_DIR}/log CLI_SOURCE_FILE)
aux_source_directory(${PROJECT_SOURCE_DIR}/net CLI_SOURCE_FILE)
aux_source_directory(${PROJECT_SOURCE_DIR}/protocol CLI_SOURCE_FILE)
add_executable(cli cli.cpp plain_websocket_client.cpp ${CLI_SOURCE_FILE})
list(APPEND 
    CLI_INCLUDE_DIRS 
    ${PROJECT_SOURCE_DIR}/qt 
    ${PROJECT_SOURCE_DIR}/file 
    ${PROJECT_SOURCE_DIR}/crypt 
    ${PROJECT_SOURCE_DIR}/log
    ${PROJECT_SOURCE_DIR}/net
    ${PROJECT_SOURCE_DIR}/protocol)
target_include_directories(cli PUBLIC "$<BUILD_INTERFACE:${CLI_INCLUDE_DIRS}>")
target_link_libraries(cli ${LINK_LIBS} Qt${QT_VERSION_MAJOR}::Widgets)
