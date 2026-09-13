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

# Fcitx5 tells us where this distro keeps addons and data (lib64 on Fedora,
# lib/<triplet> on Debian, ...). Fall back to GNUInstallDirs if unset.
if(NOT FCITX_INSTALL_ADDONDIR)
    set(FCITX_INSTALL_ADDONDIR "${CMAKE_INSTALL_LIBDIR}/fcitx5")
endif()
if(NOT FCITX_INSTALL_PKGDATADIR)
    set(FCITX_INSTALL_PKGDATADIR "${CMAKE_INSTALL_DATADIR}/fcitx5")
endif()

install(TARGETS marathitranslit DESTINATION "${FCITX_INSTALL_ADDONDIR}")
install(FILES data/marathitranslit.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")
install(FILES data/mr-translit.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")
# Candidate-popup theme (selectable in fcitx5-configtool > Addons > Classic UI).
install(DIRECTORY data/themes/maatra-dark DESTINATION "${FCITX_INSTALL_PKGDATADIR}/themes")
