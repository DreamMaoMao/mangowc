self: {
  config,
  lib,
  pkgs,
  ...
}: let
  cfg = config.programs.mango;
in {
  options = {
    programs.mango = {
      enable = lib.mkEnableOption "mango, a wayland compositor based on dwl";
      package = lib.mkOption {
        type = lib.types.package;
        default = self.packages.${pkgs.system}.mango;
        description = "The mango package to use";
      };
    };
  };

  config = lib.mkIf cfg.enable {
    environment.systemPackages =
      [
        cfg.package
      ];

    xdg.portal = {
      enable = lib.mkDefault true;

      wlr.enable = lib.mkDefault true;

      configPackages = [cfg.package];
    };

    security.polkit.enable = lib.mkDefault true;

    programs.xwayland.enable = lib.mkDefault true;

    services = {
      displayManager.sessionPackages = [cfg.package];

      graphical-desktop.enable = lib.mkDefault true;
    };
  };
}
