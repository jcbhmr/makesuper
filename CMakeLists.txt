cmake_minimum_required(VERSION 3.29)

project(superapp
    VERSION 0.1.0
    LANGUAGES C CXX
)

include(cmake/CPM.cmake)

# Can't get cosmocc to work with libraries. It fails to link for some reason.
# CPMAddPackage(
#   GITHUB_REPOSITORY fmtlib/fmt
#   GIT_TAG 11.0.2
#   OPTIONS "FMT_OS OFF" "FMT_UNICODE OFF"
# )

add_executable(app src/main.cpp)
target_compile_features(app PRIVATE cxx_std_23 c_std_23)
target_compile_options(app PRIVATE -mcosmo -fexceptions)
# target_link_options(app PRIVATE $<$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>:-mdbg> $<$<STREQUAL:${CMAKE_BUILD_TYPE},Release>:-mtiny>)
# add_custom_command(
#   TARGET app
#   POST_BUILD
#   COMMAND zip -Ar $<TARGET_FILE:app> .
#   WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/zip"
# )
# target_link_libraries(app fmt::fmt)
