
#

Master-Stack Layout



https://github.com/user-attachments/assets/a9d4776e-b50b-48fb-94ce-651d8a749b8a



Scroller Layout


https://github.com/user-attachments/assets/c9bf9415-fad1-4400-bcdc-3ad2d76de85a



# Maomaowm

This project is developed based on `dwl(0.5)` ,
adding many operation that supported in hyprland and a hyprland-like keybinds,
niri-like scroll layout and sway-like scratchpad.
See below for more features.

# separate window layout for each workspace(tags), with separate workspace parameters
## support layout
- tile(master)
- scroller
- monocle
- grid
- dwindle
- spiral

# window open rules
## options
- appid: type-string if match it or title, the rule match
- title: type-string if match it or appid, the rule match
- tags: type-num(1-9) which tags to open the window
- isfloating: type-num(0 or 1)
- isfullscreen: type-num(0 or 1)
- scroller_proportion: type-float(0.1-1.0)
- animation_type : type-string(zoom,slide)
- isnoborder : type-num(0 or 1)
- monitor  : type-num(0-99999)
- width : type-num(0-9999)
- height : type-num(0-9999)

# some special feature
- hycov like overview
- foreign-toplevel support(dunst,waybar wlr taskbar)
- minimize window to waybar(like hych)
- sway scratchpad support (minimize window to scratchpad)
- window pin mode support
- text-input-v2/v3 for fcitx5
- window move/open animaition
- workspace switch animaition
- fade in animation
- alt-tab switch window like gnome
- niri like scroller layout
- fadeout animation

## suggest tools
```
yay -S rofi foot xdg-desktop-portal-wlr swaybg waybar wl-clip-persist cliphist wl-clipboard wlsunset

```

# install
# wlroots(0.17)
```
git clone -b 0.17.4 https://gitlab.freedesktop.org/wlroots/wlroots.git
cd wlroots
meson build -Dprefix=/usr
sudo ninja -C build install

git clone https://github.com/DreamMaoMao/maomaowm.git
cd maomaowm
meson build -Dprefix=/usr
sudo ninja -C build install

# set your autostart app ih this
mkdir -p ~/.config/maomao/


```


# config
you can use `MAOMAOCONFIG` env to set the config-folder-path and the autostart-folder-patch
like `MAOMAOCONFIG=/home/xxx/maomao`

- the only default keybinds is ctrl+alt+[F1-F12] to change tty

- the default config path is `~/.config/maomao/config.conf`

- the default autostart path is `~/.config/maomao/autostart.sh`

- the fallback config path is in `/etc/maomao/config.conf`, you can find the default config here


# NixOS+Home-manager
```nix
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    home-manager = {
      url = "github:nix-community/home-manager";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    flake-parts.url = "github:hercules-ci/flake-parts";
    maomaowm.url = "github:DreamMaoMao/maomaowm";

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
                          wayland.windowManager.maomaowm = {
                            enable = true;
                            settings = ''
                              # see config.conf
                            '';
                            autostart_sh = ''
                              # see autostart.sh
                            '';
                          };
                        }
                      )
                    ]
                    ++ [
                      # Add maomaowm hm module
                      inputs.maomaowm.hmModules.maomaowm
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


# my dotfile
[maomao-config](https://github.com/DreamMaoMao/dotfile/tree/main/maomao)

# thanks for some refer repo
- https://github.com/dqrk0jeste/owl - for basal window animaition

- https://github.com/djpohly/dwl - for basal dwl feature

- https://github.com/guyuming76/dwl - for text-input

- https://github.com/swaywm/sway - for foreign-toplevel
