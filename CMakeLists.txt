cmake_minimum_required(VERSION 3.12)
project(service_template CXX)
set(CMAKE_CXX_STANDARD 17)


include(third_party/userver/cmake/SetupEnvironment.cmake)
include(GNUInstallDirs)

add_subdirectory(third_party/userver)


# Common sources
add_library(${PROJECT_NAME}_objs OBJECT
    src/infra_version_handler.cpp
    src/infra_version_handler.hpp
    src/utils/gitlab.hpp
    src/utils/gitlab.cpp
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC userver-core)


# The Service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)



# Functional Tests
include(UserverTestsuite)

userver_testsuite_add_simple(
    REQUIREMENTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/requirements.txt"
)


# Install
if(DEFINED ENV{PREFIX})
  message(STATUS "Set install prefix: $ENV{PREFIX}")
  file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
  set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

set(CONFIG_FALLBACK_PATH
    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}/dynamic_config_fallback.json")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/configs/config_vars.template.yaml"
    "${CMAKE_CURRENT_SOURCE_DIR}/configs/config_vars.yaml"
)

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})
