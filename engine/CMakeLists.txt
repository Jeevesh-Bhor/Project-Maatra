add_library(marathi-translit-engine STATIC
    src/transliterator.cpp
)
target_include_directories(marathi-translit-engine PUBLIC src)
target_compile_features(marathi-translit-engine PUBLIC cxx_std_20)
set_target_properties(marathi-translit-engine PROPERTIES POSITION_INDEPENDENT_CODE ON)
set(MARATHI_DATADIR "${CMAKE_INSTALL_FULL_DATADIR}/fcitx5-marathitranslit")
target_compile_definitions(marathi-translit-engine PRIVATE
    # Installed data location (preferred at runtime by the packaged addon)...
    MARATHI_DATADIR="${MARATHI_DATADIR}"
    # ...falling back to the source tree for uninstalled development builds.
    MARATHI_SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Ship the language data to the installed data dir.
install(FILES
    src/marathi_rules.txt
    src/mr_words.txt
    src/mr_bigrams.txt
    DESTINATION "${CMAKE_INSTALL_DATADIR}/fcitx5-marathitranslit"
)

add_executable(mr-translit-cli cli/main.cpp)
target_link_libraries(mr-translit-cli PRIVATE marathi-translit-engine)

find_package(GTest QUIET)
if(GTest_FOUND)
    enable_testing()
    add_executable(transliterator_test tests/transliterator_test.cpp)
    target_link_libraries(transliterator_test PRIVATE marathi-translit-engine GTest::gtest GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(transliterator_test)
else()
    message(WARNING "GTest not found — transliterator_test target will not be built. Install gtest-devel.")
endif()
