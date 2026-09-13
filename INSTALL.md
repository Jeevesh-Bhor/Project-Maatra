# Installing Maatra

Maatra (`fcitx5-marathi-transliterate`) is installed like any other package on
your distribution. Setup takes about two minutes:

1. [Install the package](#1-install-the-package)
2. [Log out and back in](#2-log-out-and-back-in)
3. [Add the input method](#3-add-the-input-method)
4. [Start typing](#4-start-typing)

Then, if you want them: [optional settings](#optional-settings),
[uninstalling](#uninstalling), and [troubleshooting](#troubleshooting).

## Requirements

- A Linux distribution listed below, with a graphical desktop (GNOME, KDE
  Plasma, XFCE, Cinnamon, Sway, …) on Wayland or X11.
- x86_64. Everything else — Fcitx5 itself, the GTK/Qt integrations, and a
  Devanagari font — is pulled in automatically by the package manager.

## 1. Install the package

Download the package for your distribution from the
[Releases page](https://github.com/Jeevesh-Bhor/Project-Maatra/releases),
then install it from the directory you saved it to:

| Distribution | Also covers | Command |
|---|---|---|
| **Fedora** 43+ | Nobara, Ultramarine | `sudo dnf install ./fcitx5-marathi-transliterate-*.fc44.x86_64.rpm` |
| **RHEL** 9+ | AlmaLinux, Rocky, CentOS Stream — [EPEL](https://docs.fedoraproject.org/en-US/epel/) must be enabled | `sudo dnf install ./fcitx5-marathi-transliterate-*.el9.x86_64.rpm` |
| **openSUSE** | Tumbleweed, Leap, Slowroll | `sudo zypper install ./fcitx5-marathi-transliterate-*.x86_64.rpm` |
| **Debian** 12+ | Devuan, MX Linux, Kali | `sudo apt install ./fcitx5-marathi-transliterate_*_amd64.deb` |
| **Ubuntu** 24.04+ | Mint, Pop!_OS, elementary, Zorin, KDE neon, Kubuntu | `sudo apt install ./fcitx5-marathi-transliterate_*_amd64.deb` |
| **Arch** | Manjaro, EndeavourOS, CachyOS, Garuda | `sudo pacman -U ./fcitx5-marathi-transliterate-*-x86_64.pkg.tar.zst` |

Take the package built for **your** distribution: a Debian `.deb` and an
Ubuntu `.deb` are the same format but are compiled against different library
versions, and a Fedora `.rpm` and an openSUSE `.rpm` name their dependencies
differently. See [Why one package per distribution](#why-one-package-per-distribution).

**Debian and Ubuntu only** — tell the desktop session to use Fcitx5 (other
distributions do this automatically when Fcitx5 is installed):

```bash
im-config -n fcitx5
```

> Maatra is not available as a Flatpak, Snap or AppImage. Those are sandboxed
> application formats; an input method is a plugin that must load inside the
> system's Fcitx5 process, so a native package is the only way to install one.

## 2. Log out and back in

Required once. This starts Fcitx5 with your session and sets the environment
variables applications read when they launch.

## 3. Add the input method

Open **Fcitx5 Configuration** from your application menu (or run
`fcitx5-configtool`):

1. In the **Input Method** tab, untick **Only Show Current Language**.
2. Search for **Marathi (Transliteration)** in the right-hand list.
3. Select it and press **<** to add it to the left-hand list, below your normal
   keyboard layout.
4. Click **Apply**.

Without the GUI, the same thing from a terminal (replace `us` with your
keyboard layout if different):

```bash
gdbus call --session --dest org.fcitx.Fcitx5 --object-path /controller \
  --method org.fcitx.Fcitx.Controller1.SetInputMethodGroupInfo \
  "Default" "us" "[('keyboard-us',''),('mr-translit','')]"
```

## 4. Start typing

Click into any text field and press **Ctrl+Space**. Type `namaskar` — you
should see **नमस्कार**. Press **Ctrl+Space** again to return to your normal
keyboard.

The [README](README.md#typing-guide) has the full typing guide and key
reference.

---

## Optional settings

### Switch with Super+Space

Fcitx5 switches on **Ctrl+Space** by default. To use **Super+Space** as well:

1. `fcitx5-configtool` → **Global Config** → **Trigger Input Method** → add
   `Super+space` (keep `Control+space` too). If **Enumerate Input Method Group
   Forward/Backward** show `Super+space` / `Shift+Super+space`, clear them so
   one key isn't bound twice.
2. **GNOME only** — GNOME reserves Super+Space for its own layout switcher and
   would swallow it before Fcitx5 sees it. Hand the key over:

   ```bash
   gsettings set org.gnome.desktop.wm.keybindings switch-input-source "['XF86Keyboard']"
   gsettings set org.gnome.desktop.wm.keybindings switch-input-source-backward "['<Shift>XF86Keyboard']"
   ```

   KDE Plasma: System Settings → Shortcuts → check nothing else uses
   Meta+Space.

### Larger popup with the Maatra Dark theme

The package ships a roomy dark theme with full-width highlighting and a larger
Devanagari font. Enable it in `fcitx5-configtool` → **Addons** → **Classic
User Interface**: set **Theme** to *Maatra Dark* and **Font** to *Noto Sans
Devanagari 16*.

Or add the following to `~/.config/fcitx5/conf/classicui.conf` and run
`fcitx5-remote -r`:

```ini
Theme=maatra-dark
Font="Noto Sans Devanagari 16"
Vertical Candidate List=True
PerScreenDPI=True
```

---

## Uninstalling

### 1. Remove the package

```bash
sudo dnf remove fcitx5-marathi-transliterate       # Fedora / RHEL
sudo zypper remove fcitx5-marathi-transliterate    # openSUSE
sudo apt remove fcitx5-marathi-transliterate       # Debian / Ubuntu
sudo pacman -R fcitx5-marathi-transliterate        # Arch
```

This removes the input method, its dictionary and language-model data, and
the Maatra Dark theme. Fcitx5 itself and its GTK/Qt integrations are left in
place because other input methods may use them; remove them the same way if
you no longer want Fcitx5 at all (`fcitx5`, and on Fedora `fcitx5-autostart`).

Restart Fcitx5 (`fcitx5-remote -r`) or log out and back in. "Marathi
(Transliteration)" disappears from the input-method list automatically.

### 2. Remove your personal data (optional)

Uninstalling never touches files in your home directory. To also remove what
Maatra learned from your typing:

```bash
rm -rf ~/.local/share/fcitx5/marathitranslit
```

### 3. Revert optional settings (only if you changed them)

- **Super+Space on GNOME:**
  `gsettings reset org.gnome.desktop.wm.keybindings switch-input-source` and
  `gsettings reset org.gnome.desktop.wm.keybindings switch-input-source-backward`.
- **Theme:** in `~/.config/fcitx5/conf/classicui.conf` delete the `Theme=`
  line (or set `Theme=default`), then `fcitx5-remote -r`.

---

## Troubleshooting

**Marathi doesn't appear in the input-method list.** The package is installed
but Fcitx5 was started before it; run `fcitx5-remote -r` and reopen
`fcitx5-configtool`.

**Ctrl+Space does nothing.** Fcitx5 isn't running in your session. Check with
`fcitx5-remote` — it prints `1` or `2` when running, or an error if not. If it
isn't running, make sure you logged out and back in after installing (on
Debian/Ubuntu, that you ran `im-config -n fcitx5` first).

**Typing works in some apps but not others.** The app was launched before
Fcitx5 was set up and never received the environment; close it fully and
reopen it. Apps that were already running when you installed Maatra all need
this once.

**The popup shows boxes instead of Devanagari.** The Devanagari font didn't get
installed; install `google-noto-sans-devanagari-vf-fonts` (Fedora),
`noto-sans-devanagari-fonts` (openSUSE), `fonts-noto-core` (Debian/Ubuntu) or
`noto-fonts` (Arch).

---

## Why one package per distribution

The *file format* is shared within a family — Debian and Ubuntu both use the
same `.deb`; Fedora, RHEL and openSUSE all use the same `.rpm`. What makes a
package "for Ubuntu" or "for openSUSE" is what was baked in when it was built:

- **Library versions it was compiled against.** The Debian 12 `.deb` is built
  on Fcitx5 5.0 / glibc 2.36 and declares those as minimums, so it also
  installs on newer Ubuntu; the Ubuntu 24.04 `.deb` (Fcitx5 5.1 / glibc 2.38)
  will *not* install on Debian 12. Newer can run older, not the reverse.
- **What the dependencies are called.** Fedora's GTK/Qt bridges are
  `fcitx5-gtk` and `fcitx5-qt`; openSUSE splits them into `fcitx5-gtk3`,
  `fcitx5-gtk4`, `fcitx5-qt6`. The Devanagari font package has a different
  name on each. Fedora needs a separate `fcitx5-autostart`; the others bundle
  it. A Fedora RPM on openSUSE fails with "nothing provides fcitx5-gtk" — not
  because the binary wouldn't run, but because that name doesn't exist there.

Pick the package built for your distribution and the package manager resolves
everything correctly.

---

Building the packages, or installing from source for development, is covered
in [packaging/README.md](packaging/README.md).
