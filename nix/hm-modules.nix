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

  config = lib.mkIf config.programs.maomaowm.enable {
    home.packages = [ maomaowm ];

    xdg.configFile = {
      "maomaowm/config.conf" = {
        text = config.programs.maomaowm.settings;
      };
      "maomaowm/autostart.sh" = {
        text = config.programs.maomaowm.settings;
      };
    };
  };
}
