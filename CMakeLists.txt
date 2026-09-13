cmake_minimum_required(VERSION 3.20)
project(fcitx5-marathi-transliterate CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

add_subdirectory(engine)

find_package(Fcitx5Core QUIET)
if(Fcitx5Core_FOUND)
    add_subdirectory(fcitx5)
else()
    message(WARNING "Fcitx5Core not found — skipping fcitx5/ addon target. Install fcitx5-devel and re-run cmake.")
endif()
