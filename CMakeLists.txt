cmake_minimum_required(VERSION 4.1)
project(cad_common LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_library(cad_common STATIC)
add_library(cad_common::cad_common ALIAS cad_common)

set_target_properties(cad_common PROPERTIES AUTOMOC ON)
target_compile_features(cad_common PUBLIC cxx_std_20)
target_include_directories(cad_common PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(cad_common PUBLIC
        Qt6::Core Qt6::Gui Qt6::Widgets
)


file(GLOB_RECURSE CAD_COMMON_HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/include/cad_common/*.hxx"
)

file(GLOB_RECURSE CAD_COMMON_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/src/*.cxx"
)

target_sources(cad_common PRIVATE ${CAD_COMMON_SOURCES})

target_sources(cad_common PUBLIC
        FILE_SET HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
        FILES ${CAD_COMMON_HEADERS}
)
