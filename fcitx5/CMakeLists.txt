find_package(Fcitx5Core REQUIRED)
find_package(Fcitx5Utils REQUIRED)

add_library(marathitranslit MODULE
    src/marathi_engine.cpp
    src/learn_store.cpp
)
target_link_libraries(marathitranslit PRIVATE
    Fcitx5::Core
    marathi-translit-engine
)

install(TARGETS marathitranslit DESTINATION "${CMAKE_INSTALL_LIBDIR}/fcitx5")
install(FILES data/marathitranslit.conf DESTINATION "${CMAKE_INSTALL_DATADIR}/fcitx5/addon")
install(FILES data/mr-translit.conf DESTINATION "${CMAKE_INSTALL_DATADIR}/fcitx5/inputmethod")
# Candidate-popup theme (selectable in fcitx5-configtool > Addons > Classic UI).
install(DIRECTORY data/themes/maatra-dark DESTINATION "${CMAKE_INSTALL_DATADIR}/fcitx5/themes")
