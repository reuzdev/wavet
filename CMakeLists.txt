cmake_minimum_required(VERSION 3.20)

project(wavet LANGUAGES CXX)

include(GNUInstallDirs)

set(CONFIG_HEADER_IN "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in")
set(CONFIG_HEADER_OUT "${CMAKE_CURRENT_BINARY_DIR}/config.h")

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/image.cpp
    src/terminal.cpp
    src/animation.cpp
    src/arguments.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(${PROJECT_NAME} PRIVATE thirdparty)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /permissive-
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_LIST_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

add_dependencies(${PROJECT_NAME} copy_assets)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}
)

set(INSTALLED_PATH_FOR_CPP "")

if(WIN32)
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION .
    )
    install(DIRECTORY assets/
        DESTINATION .
    )
    set(INSTALLED_PATH_FOR_CPP "assets")
else()
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(DIRECTORY assets/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
        PATTERN ".DS_Store" EXCLUDE
    )
    set(INSTALLED_PATH_FOR_CPP "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
endif()

set(APP_ASSET_DIR ${INSTALLED_PATH_FOR_CPP})
configure_file(
    "${CONFIG_HEADER_IN}"
    "${CONFIG_HEADER_OUT}"
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
