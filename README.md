# MangoWC

<img width="255" height="256" alt="mango-transparency-256" src="https://github.com/user-attachments/assets/54caff2c-932f-4998-a090-2a5292ebbfa4" />


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

Master-Stack Layout

https://github.com/user-attachments/assets/a9d4776e-b50b-48fb-94ce-651d8a749b8a

Scroller Layout

https://github.com/user-attachments/assets/c9bf9415-fad1-4400-bcdc-3ad2d76de85a

Layer animaiton

https://github.com/user-attachments/assets/014c893f-115c-4ae9-8342-f9ae3e9a0df0


# Our discord
[mangowc](https://discord.gg/CPjbDxesh5)

# Supported layouts

- tile
- scroller
- monocle
- grid
- deck
- center_tile
- vertical_tile
- vertical_grid
- vertical_scroller

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

## Arch Linux
The package is in the Arch User Repository and is availble for manual download [here](https://aur.archlinux.org/packages/mangowc-git) or through a AUR helper like yay:
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

## Other

```bash
git clone -b 0.19.1 https://gitlab.freedesktop.org/wlroots/wlroots.git
cd wlroots
meson build -Dprefix=/usr
sudo ninja -C build install

git clone https://github.com/wlrfx/scenefx.git
cd scenefx
meson build -Dprefix=/usr
sudo ninja -C build install

git clone https://github.com/DreamMaoMao/mangowc.git
cd mangowc
meson build -Dprefix=/usr
sudo ninja -C build install
```

## Suggested Tools

- Application launcher (rofi, bemenu, wmenu, fuzzel)
- Terminal emulator (foot, wezterm, alacritty, kitty, ghostty)
- Status bar (waybar, eww, quickshell, ags), waybar is preferred
- Wallpaper setup (swww, swaybg)
- Notification daemon (swaync, dunst)
- Desktop portal (xdg-desktop-portal, xdg-desktop-portal-wlr, xdg-desktop-portal-gtk)
- Clipboard (wl-clipboard, wl-clip-persist, cliphist)
- Gamma control/night light (wlsunset, gammastep)
- Miscellaneous (xfce-polkit, wlogout)

## Some Common Default Keybindings

- alt+return: open foot terminal
- alt+space: open rofi launcher
- alt+q: kill client
- alt+left/right/up/down: focus direction
- super+m: quit mango

## My Dotfiles

- Dependencies

```bash
yay -S rofi foot xdg-desktop-portal-wlr swaybg waybar wl-clip-persist cliphist wl-clipboard wlsunset xfce-polkit swaync pamixer wlr-dpms sway-audio-idle-inhibit-git swayidle dimland-git brightnessctl swayosd wlr-randr grim slurp satty swaylock-effects-git wlogout sox
```

- use my config

```bash
git clone https://github.com/DreamMaoMao/mango-config.git ~/.config/mango
```


## Config Documentation

Refer to the [wiki](https://github.com/DreamMaoMao/mango/wiki/)

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
    mango.url = "github:DreamMaoMao/mango";
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


# Sponsor

My current device is a bit outdated and doesn't support certain features like HDR or VRR. If you'd like to support this project, here's how you can help. Thanks!

![Screenshot_2025-10-13-20-06-49-26_ee1cec40dcf6eb3](https://github.com/user-attachments/assets/240a0727-9eb5-4212-a84c-10fa9f093147)


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
