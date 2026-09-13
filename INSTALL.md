# Installing Maatra (fcitx5-marathi-transliterate)

## A. Fresh machine (end user) — from the `.rpm`

Tested on Fedora 44 / GNOME (Wayland). Four steps.

### 1. Install the package

```bash
sudo dnf install ./fcitx5-marathi-transliterate-1.0.0-1.fc44.x86_64.rpm
```

`dnf` pulls in everything needed automatically: the Fcitx5 framework,
`fcitx5-autostart` (so Fcitx5 starts with your login session), the GTK and Qt
integrations (so apps actually see the input method), and the Devanagari font
for the popup.

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
sudo dnf remove fcitx5-marathi-transliterate
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
sudo dnf install ./fcitx5-marathi-transliterate-1.0.0-1.fc44.x86_64.rpm
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

Keep `~/rpmbuild/RPMS/x86_64/*.rpm` (or copy it elsewhere) — that's your
installable package. The project source tree in
`~/Documents/marathi-transliteration` can stay for future development.
