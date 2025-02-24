{
  lib,
  config,
  pkgs,
  ...
}:

let
  maomaowm = pkgs.callPackage ./. { };
in
{
  options = {
    wayland.windowManager.maomaowm = {
      enable = lib.mkOption {
        type = lib.types.bool;
        default = false;
      };

      settings = lib.mkOption {
        type = lib.types.str;
        default = "";
      };

      autostart_sh = lib.mkOption {
        type = lib.types.str;
        default = "";
      };
    };
  };

  config = lib.mkIf config.wayland.windowManager.maomaowm.enable {
    home.packages = [ maomaowm ];

    xdg.configFile = {
      "maomaowm/config.conf" = {
        text = config.wayland.windowManager.maomaowm.settings;
      };
      "maomaowm/autostart.sh" = {
        text = config.wayland.windowManager.maomaowm.autostart_sh;
      };
    };
  };
}
