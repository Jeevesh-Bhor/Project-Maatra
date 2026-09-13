Name:           fcitx5-marathi-transliterate
Version:        1.0.0
Release:        1%{?dist}
Summary:        Maatra — phonetic Marathi transliteration input method for Fcitx5

License:        GPL-3.0-or-later
URL:            https://github.com/Jeevesh-Bhor/Project-Maatra
Source0:        %{url}/archive/refs/tags/v%{version}.tar.gz#/Project-Maatra-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  fcitx5-devel
%if 0%{?suse_version}
BuildRequires:  gtest
%else
BuildRequires:  gtest-devel
%endif

Requires:       fcitx5
# Toolkit integrations so GTK and Qt apps see the IME, and a Devanagari font
# for the candidate popup. Package names differ per distro family.
%if 0%{?suse_version}
Requires:       fcitx5-gtk3
Requires:       fcitx5-gtk4
Requires:       fcitx5-qt6
Requires:       noto-sans-devanagari-fonts
%else
# Fedora / RHEL (EPEL)
Requires:       fcitx5-gtk
Requires:       fcitx5-qt
%if 0%{?fedora}
# Starts fcitx5 with the GUI session; without it the IME is not running after
# login and Marathi typing silently does nothing.
Requires:       fcitx5-autostart
Requires:       google-noto-sans-devanagari-vf-fonts
%else
Requires:       google-noto-sans-devanagari-fonts
%endif
%endif
# Lets the user add the input method from a GUI.
Recommends:     fcitx5-configtool

%description
Maatra lets you type Marathi phonetically on a QWERTY keyboard and get
Devanagari, with a ranked candidate popup, sentence-context disambiguation
(bigram language model), Devanagari digits, and per-user learning. Fully
offline.

%prep
%autosetup -n Project-Maatra-%{version}

%build
%cmake
%cmake_build

%check
%ctest

%install
%cmake_install

%files
%license LICENSE LICENSES.md
%doc README.md INSTALL.md
%{_libdir}/fcitx5/libmarathitranslit.so
%{_datadir}/fcitx5/addon/marathitranslit.conf
%{_datadir}/fcitx5/inputmethod/mr-translit.conf
%{_datadir}/fcitx5/themes/maatra-dark/
%dir %{_datadir}/fcitx5-marathitranslit
%{_datadir}/fcitx5-marathitranslit/marathi_rules.txt
%{_datadir}/fcitx5-marathitranslit/mr_words.txt
%{_datadir}/fcitx5-marathitranslit/mr_bigrams.txt

%changelog
* Sun Sep 13 2026 Maatra maintainers <nobody@localhost> - 1.0.0-1
- Initial release: rule engine, dictionary + frequency + bigram ranking,
  candidate popup, Devanagari digits, and per-user learning.
