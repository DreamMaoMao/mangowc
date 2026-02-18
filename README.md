# Mango Wayland Compositor
<div>
  <img src="https://github.com/DreamMaoMao/mangowc/blob/main/assets/mango-transparency-256.png" alt="MangoWC Logo" width="120"/>
</div>

This project's development is based on [dwl](https://codeberg.org/dwl/dwl/).


1. **Lightweight & Fast Build**

   - _Mango_ is as lightweight as _dwl_, and can be built completely within a few seconds. Despite this, _Mango_ does not compromise on functionality.

2. **Feature Highlights**
   - In addition to basic WM functionality, Mango provides:
     - Excellent xwayland support.
     - Base tags not workspaces (supports separate window layouts for each tag)
     - Smooth and customizable complete animations (window open/move/close, tag enter/leave,layer open/close/move)
     - Excellent input method support (text input v2/v3)
     - Flexible window layouts with easy switching (scroller, master-stack, monocle,center-master, etc.)
     - Rich window states (swallow, minimize, maximize, unglobal, global, fakefullscreen, overlay, etc.)
     - Simple yet powerful external configuration(support shortcuts hot-reload)
     - Sway-like scratchpad and named scratchpad
     - Ipc support(get/send message from/to compositor by external program)
     - Hycov-like overview
     - Window effects from scenefx (blur, shadow, corner radius, opacity)
     - Zero flickering - every frame is perfect.

https://github.com/user-attachments/assets/bb83004a-0563-4b48-ad89-6461a9b78b1f

# Quick Start Guide

## What is MangoWC?

MangoWC is a **Wayland compositor** - a program that manages windows and displays on modern Linux systems using the Wayland protocol. If you're familiar with window managers like i3, dwm, or awesome, MangoWC provides similar tiling window management functionality but for Wayland instead of X11.

## First Steps After Installation

1. **Copy the default configuration:**
   ```bash
   mkdir -p ~/.config/mango
   cp /usr/share/mango/config.conf ~/.config/mango/config.conf
   ```

2. **Launch MangoWC:**
   - From a display manager: Select "Mango" session
   - From TTY: Run `mango`

3. **Essential first keybindings to know:**
   - `Alt + Return` - Open terminal (foot)
   - `Alt + Space` - Open application launcher (rofi)
   - `Alt + Q` - Close focused window
   - `Super + M` - Exit MangoWC
   - `Super + R` - Reload configuration (after making changes)

4. **Create an autostart script** (optional):
   ```bash
   # Create ~/.config/mango/autostart.sh
   #!/bin/bash
   
   # Set wallpaper
   swaybg -i ~/Pictures/wallpaper.jpg &
   
   # Start status bar
   waybar &
   
   # Start notification daemon
   swaync &
   ```
   Make it executable: `chmod +x ~/.config/mango/autostart.sh`

## Key Concepts

### Tags vs Workspaces

Unlike traditional workspaces, **tags** are more flexible:

- **Workspaces**: A window belongs to one workspace. Switching workspaces shows a different set of windows.
- **Tags**: A window can have multiple tags. You can view multiple tags at once or filter to specific tags.

Think of tags as labels you can attach to windows. You can view windows with tag 1, or tag 2, or both tags 1 and 2 simultaneously.

**Default behavior:**
- `Ctrl + 1-9` - View tag 1-9
- `Alt + 1-9` - Move current window to tag 1-9
- Each tag can have its own layout (tile, scroller, grid, etc.)

### Window Layouts

MangoWC supports 9 different layouts:

| Layout | Description | Best For |
|--------|-------------|----------|
| **tile** | Master-stack tiling (left master, right stack) | General multitasking |
| **scroller** | Horizontal scrolling columns | Wide content, terminals |
| **monocle** | One window fullscreen at a time | Focus, presentations |
| **grid** | Windows arranged in grid | Many small windows |
| **deck** | Stack of windows, one visible | Cycling through tasks |
| **center_tile** | Master centered, stack on sides | Symmetrical layout |
| **vertical_tile** | Master top, stack bottom | Wide monitors |
| **vertical_scroller** | Vertical scrolling rows | Document review |
| **vertical_grid** | Vertical grid arrangement | Vertical content |

**Switch layouts:**
- `Super + N` - Cycle through layouts for current tag
- Each tag can have its own default layout (set in `config.conf`)

### Scratchpad

The **scratchpad** is a hidden workspace for temporary windows:

- `Alt + Z` - Toggle scratchpad (hide/show)
- Windows in scratchpad are hidden but not closed
- Perfect for calculator, music player, notes, etc.
- Scratchpad windows float centered on screen

**Usage example:**
1. Open a terminal (`Alt + Return`)
2. Move it to scratchpad (`Alt + Z`)
3. It disappears
4. Press `Alt + Z` again - it appears over your current work
5. Press `Alt + Z` again - it hides again

### Overview Mode

**Overview** mode shows all windows at once (like Alt+Tab visualization):

- `Alt + Tab` - Toggle overview mode
- See all windows across all tags
- Click a window to focus it
- Move windows in overview
- Hotarea: Move mouse to top edge of screen to trigger overview

### Window States

Windows can have multiple states:

- **Floating**: Window floats above tiled windows, can be moved/resized freely
  - `Alt + \` - Toggle floating
  - `Super + Left-drag` - Move floating window
  - `Super + Right-drag` - Resize floating window

- **Fullscreen**: Window covers entire screen, hides all others
  - `Alt + F` - Toggle fullscreen
  - `Alt + Shift + F` - Toggle fake fullscreen (fullscreen but stays in layout)

- **Maximized**: Window fills screen but shows borders/gaps
  - `Alt + A` - Toggle maximize

- **Minimized**: Window hidden but not in scratchpad
  - `Super + I` - Minimize current window
  - `Super + Shift + I` - Restore last minimized window

- **Global**: Window visible on all tags
  - `Super + G` - Toggle global state

- **Overlay**: Window stays on top of others
  - `Super + O` - Toggle overlay state

## Day-to-Day Usage

### Typical Workflow

1. **Open applications:**
   ```
   Alt + Space          → Application launcher
   Alt + Return         → Terminal
   ```

2. **Navigate windows:**
   ```
   Alt + Arrow Keys     → Focus window in direction
   Super + Tab          → Focus next window in stack
   ```

3. **Organize windows:**
   ```
   Super + Shift + Arrows  → Swap window positions
   Alt + \                 → Toggle floating
   Alt + 1-9               → Move to specific tag
   ```

4. **Adjust layout:**
   ```
   Super + N               → Change layout
   Alt + Shift + X/Z       → Increase/decrease gaps
   ```

5. **Multi-monitor:**
   ```
   Alt + Shift + Left/Right  → Focus other monitor
   Super + Alt + Left/Right  → Move window to other monitor
   ```

### Common Use Cases

**Web browsing + Terminal:**
- Open browser on tag 1, terminal on tag 2
- Use `Ctrl + 1` and `Ctrl + 2` to switch between them

**Development workflow:**
- Tag 1: Code editor (center_tile layout)
- Tag 2: Browser (monocle layout)
- Tag 3: Terminals (tile or scroller layout)
- Scratchpad: Calculator, notes

**Keeping a window visible everywhere:**
- Open music player or chat app
- Press `Super + G` to make it global
- It now appears on all tags

## IPC Control with mmsg

MangoWC includes `mmsg` - a command-line tool to control the compositor:

### Get Information

```bash
# Get current tag info
mmsg -t

# Get layout information
mmsg -L

# Get output (monitor) information
mmsg -o

# Watch for changes (real-time updates)
mmsg -w -t    # Watch tags
```

### Send Commands

```bash
# Switch to tag 3
mmsg -d view 3

# Move window to tag 5
mmsg -d tag 5

# Change layout
mmsg -d setlayout tile

# Reload configuration
mmsg -d reload_config

# Spawn application
mmsg -d spawn firefox
```

### Scripting Examples

**Auto-save workspace state:**
```bash
#!/bin/bash
# Save current tags to file
mmsg -t > ~/mango-state.txt
```

**Tag-specific wallpapers:**
```bash
#!/bin/bash
# In a loop, change wallpaper based on active tag
while true; do
    TAG=$(mmsg -t | grep "seltag" | cut -d: -f2)
    swaybg -i ~/wallpapers/tag${TAG}.jpg &
    sleep 1
done
```

**Quick window layout toggle:**
```bash
#!/bin/bash
# Toggle between tile and monocle layouts
CURRENT=$(mmsg -L | grep "layout_name:" | head -1)
if [[ $CURRENT == *"tile"* ]]; then
    mmsg -d setlayout monocle
else
    mmsg -d setlayout tile
fi
```

## Troubleshooting

### MangoWC won't start

1. **Check dependencies:**
   ```bash
   # Verify wlroots and scenefx are installed
   pkg-config --modversion wlroots scenefx
   ```

2. **Check logs:**
   ```bash
   # Run from terminal to see error messages
   mango
   ```

3. **XWayland issues:**
   ```bash
   # If X11 apps won't start, rebuild with XWayland
   meson configure build -Dxwayland=enabled
   ninja -C build install
   ```

### Config changes not applying

- Run `Super + R` to reload config
- Some settings (trackpad, mouse) require logout/login
- Check config syntax: `mango --validate-config` (if available)

### Keybindings not working

1. **Find correct key name:**
   ```bash
   # Install wev to see key names
   wev
   # Press keys and see their names
   ```

2. **Check for conflicts:**
   - Look for duplicate bindings in `config.conf`
   - Some apps capture keys (browsers, terminals)

### Applications not starting

1. **Missing required tools:**
   ```bash
   # Install suggested applications
   sudo pacman -S rofi foot waybar swaybg
   ```

2. **Check autostart script:**
   ```bash
   # Test autostart manually
   bash ~/.config/mango/autostart.sh
   ```

### Performance issues

1. **Disable effects:**
   ```conf
   # In config.conf
   animations=0
   blur=0
   shadows=0
   ```

2. **Check GPU drivers:**
   ```bash
   # Ensure proper graphics drivers are installed
   glxinfo | grep "OpenGL"
   ```

### Screen sharing not working

Install portal packages:
```bash
sudo pacman -S xdg-desktop-portal xdg-desktop-portal-wlr
```

## Getting Help

- **Discord**: [MangoWC Community](https://discord.gg/CPjbDxesh5)
- **Wiki**: [Full documentation](https://github.com/DreamMaoMao/mango/wiki/)
- **Website**: [mangowc.vercel.app/docs](https://mangowc.vercel.app/docs)
- **Issues**: [GitHub Issues](https://github.com/DreamMaoMao/mangowc/issues)

# Our discord
[mangowc](https://discord.gg/CPjbDxesh5)

---

# Installation

## Dependencies

- glibc
- wayland
- wayland-protocols
- libinput
- libdrm
- libxkbcommon
- pixman
- git
- meson
- ninja
- libdisplay-info
- libliftoff
- hwdata
- seatd
- pcre2
- xorg-xwayland
- libxcb

## Arch Linux
The package is in the Arch User Repository and is available for manual download [here](https://aur.archlinux.org/packages/mangowc-git) or through a AUR helper like yay:
```bash
yay -S mangowc-git

```

## Gentoo Linux
The package is in the community-maintained repository called GURU.
First, add GURU repository:

```bash
emerge --ask --verbose eselect-repository
eselect repository enable guru
emerge --sync guru
```

Then, add `gui-libs/scenefx` and `gui-wm/mangowc` to the `package.accept_keywords`.

Finally, install the package:

```bash
emerge --ask --verbose gui-wm/mangowc
```

## Fedora Linux
The package is in the third-party Terra repository.
First, add the [Terra Repository](https://terra.fyralabs.com/).

Then, install the package:

```bash
dnf install mangowc
```

## GuixSD
The package definition is described in the source repository.
First, add `mangowc` channel to `channels.scm` file:

```scheme
;; In $HOME/.config/guix/channels.scm
(cons (channel
        (name 'mangowc)
        (url "https://github.com/DreamMaoMao/mangowc.git"))
      ... ;; Your other channels
      %default-channels)
```

Then, run `guix pull` and after update you can either run
`guix install mangowc` or add it to your configuration via:

```scheme
(use-modules (mangowc)) ;; Add mangowc module

;; Add mangowc to packages list
(packages (cons 
            mangowc
            ... ;; Other packages you specified
            %base-packages))
```

And then rebuild your system.

## Other

```bash
git clone -b 0.19.2 https://gitlab.freedesktop.org/wlroots/wlroots.git
cd wlroots
meson build -Dprefix=/usr
sudo ninja -C build install

git clone -b 0.4.1 https://github.com/wlrfx/scenefx.git
cd scenefx
meson build -Dprefix=/usr
sudo ninja -C build install

git clone https://github.com/DreamMaoMao/mangowc.git
cd mangowc
meson build -Dprefix=/usr
sudo ninja -C build install
```

## Suggested Tools

### Hybrid component
- [dms-shell](https://github.com/AvengeMedia/DankMaterialShell)

### Independent component
- Application launcher (rofi, bemenu, wmenu, fuzzel)
- Terminal emulator (foot, wezterm, alacritty, kitty, ghostty)
- Status bar (waybar, eww, quickshell, ags), waybar is preferred
- Wallpaper setup (swww, swaybg)
- Notification daemon (swaync, dunst,mako)
- Desktop portal (xdg-desktop-portal, xdg-desktop-portal-wlr, xdg-desktop-portal-gtk)
- Clipboard (wl-clipboard, wl-clip-persist, cliphist)
- Gamma control/night light (wlsunset, gammastep)
- Miscellaneous (xfce-polkit, wlogout)

## Default Keybindings Reference

> **Note**: All keybindings can be customized in `~/.config/mango/config.conf`

### Essential Shortcuts

| Keybinding | Action | Description |
|------------|--------|-------------|
| `Alt + Return` | Open terminal | Launches foot terminal emulator |
| `Alt + Space` | Open launcher | Launches rofi application launcher |
| `Alt + Q` | Close window | Kill focused window |
| `Super + M` | Exit | Quit MangoWC |
| `Super + R` | Reload config | Apply config changes without restart |

### Window Management

| Keybinding | Action |
|------------|--------|
| `Alt + ←/→/↑/↓` | Focus window in direction |
| `Super + Tab` | Focus next window |
| `Super + Shift + ←/→/↑/↓` | Swap window with neighbor |
| `Alt + \` | Toggle floating/tiling |
| `Alt + F` | Toggle fullscreen |
| `Alt + Shift + F` | Toggle fake fullscreen |
| `Alt + A` | Toggle maximize |
| `Super + I` | Minimize window |
| `Super + Shift + I` | Restore minimized window |
| `Super + G` | Toggle global (visible all tags) |
| `Super + O` | Toggle overlay (always on top) |

### Tag (Workspace) Management

| Keybinding | Action |
|------------|--------|
| `Ctrl + 1-9` | Switch to tag 1-9 |
| `Alt + 1-9` | Move window to tag 1-9 (and follow) |
| `Super + ←/→` | Previous/next tag |
| `Ctrl + ←/→` | Previous/next tag with windows |
| `Ctrl + Super + ←/→` | Move window to previous/next tag |

### Layout Control

| Keybinding | Action |
|------------|--------|
| `Super + N` | Cycle through layouts |
| `Alt + E` | Set window to full width (scroller) |
| `Alt + X` | Cycle width presets (scroller) |
| `Alt + Shift + X/Z` | Increase/decrease gaps |
| `Alt + Shift + R` | Toggle gaps on/off |

### Special Features

| Keybinding | Action |
|------------|--------|
| `Alt + Tab` | Toggle overview mode |
| `Alt + Z` | Toggle scratchpad |
| `Super + Scroll Up/Down` | Switch tags with scroll wheel |

### Multi-Monitor

| Keybinding | Action |
|------------|--------|
| `Alt + Shift + ←/→` | Focus adjacent monitor |
| `Super + Alt + ←/→` | Move window to adjacent monitor |

### Floating Window Adjustment

| Keybinding | Action |
|------------|--------|
| `Ctrl + Shift + ←/→/↑/↓` | Move floating window by pixels |
| `Ctrl + Alt + ←/→/↑/↓` | Resize floating window by pixels |
| `Super + Left-drag` | Move floating window with mouse |
| `Super + Right-drag` | Resize floating window with mouse |
| `Middle-click` | Maximize window |

> **Tip**: Press `Super + R` after editing your config to reload without restarting!

## My Dotfiles

### Daily
- Dependencies

```bash
yay -S rofi foot xdg-desktop-portal-wlr swaybg waybar wl-clip-persist cliphist wl-clipboard wlsunset xfce-polkit swaync pamixer wlr-dpms sway-audio-idle-inhibit-git swayidle dimland-git brightnessctl swayosd wlr-randr grim slurp satty swaylock-effects-git wlogout sox
```

### Dms
- Dependencies
```bash
yay -S foot xdg-desktop-portal-wlr swaybg wl-clip-persist cliphist wl-clipboard sway-audio-idle-inhibit-git brightnessctl grim slurp satty matugen-bin dms-shell-git

```
- use my dms config

```bash
git clone -b dms https://github.com/DreamMaoMao/mango-config.git ~/.config/mango
```
- use my daily config

```bash
git clone https://github.com/DreamMaoMao/mango-config.git ~/.config/mango
```


## Config Documentation

Refer to the repo wiki [wiki](https://github.com/DreamMaoMao/mango/wiki/)

or the website docs [docs](https://mangowc.vercel.app/docs)

# NixOS + Home-manager

The repo contains a flake that provides a NixOS module and a home-manager module for mango.
Use the NixOS module to install mango with other necessary components of a working Wayland environment.
Use the home-manager module to declare configuration and autostart for mango.

Here's an example of using the modules in a flake:

```nix
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    home-manager = {
      url = "github:nix-community/home-manager";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    flake-parts.url = "github:hercules-ci/flake-parts";
    mango = {
      url = "github:DreamMaoMao/mango";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };
  outputs =
    inputs@{ self, flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      debug = true;
      systems = [ "x86_64-linux" ];
      flake = {
        nixosConfigurations = {
          hostname = inputs.nixpkgs.lib.nixosSystem {
            system = "x86_64-linux";
            modules = [
              inputs.home-manager.nixosModules.home-manager

              # Add mango nixos module
              inputs.mango.nixosModules.mango
              {
                programs.mango.enable = true;
              }
              {
                home-manager = {
                  useGlobalPkgs = true;
                  useUserPackages = true;
                  backupFileExtension = "backup";
                  users."username".imports =
                    [
                      (
                        { ... }:
                        {
                          wayland.windowManager.mango = {
                            enable = true;
                            settings = ''
                              # see config.conf
                            '';
                            autostart_sh = ''
                              # see autostart.sh
                              # Note: here no need to add shebang
                            '';
                          };
                        }
                      )
                    ]
                    ++ [
                      # Add mango hm module
                      inputs.mango.hmModules.mango
                    ];
                };
              }
            ];
          };
        };
      };
    };
}
```

# Packaging mango

To package mango for other distributions, you can check the reference setup for:

- [nix](https://github.com/DreamMaoMao/mangowc/blob/main/nix/default.nix)
- [arch](https://aur.archlinux.org/cgit/aur.git/tree/PKGBUILD?h=mangowc-git).
- [gentoo](https://data.gpo.zugaina.org/guru/gui-wm/mangowc)

You might need to package `scenefx` for your distribution, check availability [here](https://github.com/wlrfx/scenefx.git).

If you encounter build errors when packaging `mango`, feel free to create an issue and ask a question, but
Read The Friendly Manual on packaging software in your distribution first.

# Thanks to These Reference Repositories

- https://gitlab.freedesktop.org/wlroots/wlroots - Implementation of Wayland protocol

- https://github.com/dqrk0jeste/owl - Basal window animation

- https://codeberg.org/dwl/dwl - Basal dwl feature

- https://github.com/swaywm/sway - Sample of Wayland protocol

- https://github.com/wlrfx/scenefx - Make it simple to add window effect.


# Sponsor
At present, I can only accept sponsorship through an encrypted connection.
If you find this project helpful to you, you can offer sponsorship in the following ways.

<img width="650" height="870" alt="image" src="https://github.com/user-attachments/assets/8c860317-90d2-4071-971d-f1a92b674469" />


Thanks to the following friends for their sponsorship of this project

[@tonybanters](https://github.com/tonybanters)
