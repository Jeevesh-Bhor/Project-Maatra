# Installing Maatra (fcitx5-marathi-transliterate)

## A. Fresh machine (end user) — from a package

Four steps on any distro. Tested on Fedora 44 / GNOME (Wayland); packages are
built and unit-tested in clean containers of every distro listed.

### 1. Install the package for your distro

| Distro family | Package | Install command |
|---|---|---|
| **Fedora** 43+, Nobara, Ultramarine | `dist/fedora/*.fc44.x86_64.rpm` | `sudo dnf install ./dist/fedora/fcitx5-marathi-transliterate-1.0.0-1.fc44.x86_64.rpm` |
| **RHEL / AlmaLinux / Rocky** 9+ (EPEL enabled) | rebuild the `.spec` (see §C) | `sudo dnf install ./fcitx5-marathi-transliterate-1.0.0-1.el9.x86_64.rpm` |
| **openSUSE** Tumbleweed / Leap | `dist/suse/*.x86_64.rpm` | `sudo zypper install ./fcitx5-marathi-transliterate-1.0.0-1.x86_64.rpm` |
| **Debian** 12+ | `dist/debian/*.deb` | `sudo apt install ./fcitx5-marathi-transliterate_1.0.0_amd64.deb` |
| **Ubuntu** 24.04+, Mint, Pop!_OS, elementary, Zorin | `dist/ubuntu/*.deb` | `sudo apt install ./fcitx5-marathi-transliterate_1.0.0_amd64.deb` |
| **Arch**, Manjaro, EndeavourOS, CachyOS | `dist/arch/*.pkg.tar.zst` | `sudo pacman -U ./fcitx5-marathi-transliterate-1.0.0-1-x86_64.pkg.tar.zst` |

The package manager pulls in everything needed automatically: the Fcitx5
framework, the GTK and Qt integrations (so apps actually see the input
method), and the Devanagari font for the popup.

**Why a separate `.deb` for Debian and Ubuntu, and a separate `.rpm` for
Fedora and openSUSE?** The format is the same within a family; the contents
are not. The Debian `.deb` is compiled against Fcitx5 5.0.21 / glibc 2.36 and
declares those minimums, so it installs on Ubuntu too — but the Ubuntu `.deb`
(Fcitx5 5.1.7 / glibc 2.38) will not install on Debian 12. Fedora and openSUSE
name their dependencies differently (`fcitx5-gtk` vs `fcitx5-gtk3`/`-gtk4`,
`google-noto-sans-devanagari-vf-fonts` vs `noto-sans-devanagari-fonts`,
`fcitx5-autostart` only on Fedora), so an RPM built for one fails dependency
resolution on the other. Always take the package built for your own distro.

**Debian / Ubuntu only — one extra command.** Debian-family distros use
`im-config` to decide which input-method framework the session starts and
which environment variables apps get. Tell it to use Fcitx5 once:

```bash
im-config -n fcitx5
```

(On Fedora, openSUSE and Arch the `fcitx5` package itself installs the
session autostart entry, so nothing extra is needed. Fedora splits it into
`fcitx5-autostart`, which the RPM depends on.)

**Not available as Flatpak / Snap / AppImage** — those are sandboxed *app*
formats, and this is a plugin that must load inside the system's `fcitx5`
process. A native package is the only way to install an input method.

### 2. Log out and log back in

This is required once. It starts Fcitx5 as your session's input-method
framework and sets the environment variables apps read at launch. (Starting it
by hand from a terminal is *not* enough for a normal daily setup.)

### 3. Add the input method

Open **Fcitx5 Configuration** (`fcitx5-configtool` from the terminal, or find
it in the app grid):

1. In the **Input Method** tab, untick **"Only Show Current Language"**.
2. Search for **Marathi (Transliteration)** in the right-hand list.
3. Select it and press the **<** (add) arrow so it appears in the left-hand
   "Current Input Method" list under your normal keyboard.
4. **Apply**.

Command-line alternative (no GUI):

```bash
gdbus call --session --dest org.fcitx.Fcitx5 --object-path /controller \
  --method org.fcitx.Fcitx.Controller1.SetInputMethodGroupInfo \
  "Default" "us" "[('keyboard-us',''),('mr-translit','')]"
```

### 4. Type

Click into any text field and press **Ctrl+Space** to switch between your
normal keyboard and Marathi. A candidate popup appears as you type; see
`README.md` for the typing guide and keys.

### Optional: switch with Super+Space (like GNOME's own layout switcher)

Fcitx5's switch key is **Ctrl+Space** by default. To also use **Super+Space**
you need two things, because on GNOME/Wayland GNOME itself grabs Super+Space
for its own layout switcher and would swallow the key before Fcitx5 sees it:

1. **Give the key to Fcitx5**: `fcitx5-configtool` → **Global Config** →
   **Trigger Input Method** → add `Super+space` (keep `Control+space` too).
   Also clear **Enumerate Input Method Group Forward/Backward** if they show
   `Super+space` / `Shift+Super+space`, so one key isn't bound to two things.
2. **Release GNOME's grab** (you keep the dedicated keyboard hardware key):

   ```bash
   gsettings set org.gnome.desktop.wm.keybindings switch-input-source "['XF86Keyboard']"
   gsettings set org.gnome.desktop.wm.keybindings switch-input-source-backward "['<Shift>XF86Keyboard']"
   ```

   To undo: `gsettings reset org.gnome.desktop.wm.keybindings switch-input-source`
   (and `...-backward`).

### Optional: the nicer popup theme

The package ships a roomy dark theme. To enable it: `fcitx5-configtool` →
**Addons** → **Classic User Interface** → set **Theme** to *Maatra Dark* and
**Font** to *Noto Sans Devanagari 16*. (Or put the following in
`~/.config/fcitx5/conf/classicui.conf` and restart Fcitx5.)

```ini
Theme=maatra-dark
Font="Noto Sans Devanagari 16"
Vertical Candidate List=True
PerScreenDPI=True
```

### Uninstall

```bash
sudo dnf remove fcitx5-marathi-transliterate       # Fedora / RHEL
sudo zypper remove fcitx5-marathi-transliterate    # openSUSE
sudo apt remove fcitx5-marathi-transliterate       # Debian / Ubuntu
sudo pacman -R fcitx5-marathi-transliterate        # Arch
```

Your personal learning data (`~/.local/share/fcitx5/marathitranslit/`) and
Fcitx5 config are left untouched; delete them by hand if you want a full reset.

---

## B. The development machine — switching from the hand-built install to the `.rpm`

During development the addon was installed straight from the build tree
(`sudo cmake --install build`), Fcitx5 was started by hand each time, and a
number of build-only tools and data-generation artifacts were added. Here is
what to do so the machine ends up in exactly the same state a fresh end user
gets, with nothing left dangling.

### 1. Remove the hand-installed files

They are not owned by any package, so `dnf remove` can't see them. Delete them
explicitly (installing the RPM over them would also work, but this keeps
package ownership clean):

```bash
sudo rm -f /usr/lib64/fcitx5/libmarathitranslit.so \
           /usr/share/fcitx5/addon/marathitranslit.conf \
           /usr/share/fcitx5/inputmethod/mr-translit.conf
sudo rm -rf /usr/share/fcitx5-marathitranslit
```

### 2. Install the package (steps A.1 → A.2)

```bash
sudo dnf install ./dist/fedora/fcitx5-marathi-transliterate-1.0.0-1.fc44.x86_64.rpm
```

Then **log out and back in**. This also installs `fcitx5-autostart`, which
fixes the one real gap in the dev setup: until now Fcitx5 only ran because it
was launched manually, so Marathi typing would have stopped working after a
reboot.

### 3. Nothing to redo in your config

Your existing `~/.config/fcitx5/profile` already lists `mr-translit` (step A.3
is already done), your `classicui.conf` font/theme settings carry over, and
your learned choices in `~/.local/share/fcitx5/marathitranslit/learned.tsv`
are kept. The theme you had under `~/.local/share/fcitx5/themes/maatra-dark`
is now also shipped system-wide by the package; the user copy is harmless and
can be deleted.

### 4. Optional cleanup of build-only things

None of these are needed at runtime. Remove them only if you don't want them
around:

```bash
# Build/dev tooling and the source dictionaries used to *generate* the data:
sudo dnf remove fcitx5-devel gtest-devel extra-cmake-modules \
                hunspell-mr aspell-mr rpm-build rpmdevtools

# Raw training corpus (~19 GB) and RPM build scratch:
rm -rf ~/mr_corpus ~/rpmbuild/BUILD ~/rpmbuild/BUILDROOT
```

The installable packages for every distro are in `dist/<distro>/` and can be
regenerated any time with `packaging/build-all.sh` (see §C). The project
source tree can stay for future development.

---

## C. Building the packages yourself

Everything is built from one source tree; the per-format metadata lives in
`packaging/rpm/` (Fedora, RHEL, openSUSE — one spec with distro conditionals),
`packaging/arch/PKGBUILD`, and `debian/`.

`packaging/build-all.sh` builds every format inside a clean container of its
target distro (needs `podman` or `docker`) and runs the unit tests in each:

```bash
packaging/build-all.sh              # all: fedora deb ubuntu arch suse
packaging/build-all.sh deb arch     # a subset
```

Output lands in `dist/<distro>/`. For RHEL, run the Fedora recipe against a
`almalinux:9` image with EPEL enabled, or `rpmbuild -bb packaging/rpm/*.spec`
on the target machine.

To build only for the machine you are on, without containers:

```bash
# Fedora / RHEL / openSUSE
rpmdev-setuptree && cp packaging/rpm/*.spec ~/rpmbuild/SPECS/ && \
  tar --transform 's,^,Project-Maatra-1.0.0/,' -czf ~/rpmbuild/SOURCES/Project-Maatra-1.0.0.tar.gz \
      --exclude=build --exclude=dist --exclude=.git . && \
  rpmbuild -bb ~/rpmbuild/SPECS/fcitx5-marathi-transliterate.spec

# Debian / Ubuntu
sudo apt install build-essential debhelper cmake libfcitx5core-dev libfcitx5config-dev libfcitx5utils-dev libgtest-dev
dpkg-buildpackage -us -uc -b        # .deb appears in the parent directory

# Arch
cd packaging/arch && makepkg -si    # fetches the GitHub tag tarball
```
