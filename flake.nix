{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    mmsg = {
      url = "github:DreamMaoMao/mmsg";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    scenefx = {
      url = "github:wlrfx/scenefx";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = {
    self,
    flake-parts,
    ...
  } @ inputs:
    flake-parts.lib.mkFlake {inherit inputs;} {
      imports = [
        inputs.flake-parts.flakeModules.easyOverlay
      ];

      flake = {
        hmModules.mango = import ./nix/hm-modules.nix self;
        nixosModules.mango = import ./nix/nixos-modules.nix self;
      };

      perSystem = {
        config,
        pkgs,
        ...
      }: let
        inherit
          (pkgs)
          callPackage
          ;
        mango = callPackage ./nix {
          inherit (inputs.mmsg.packages.${pkgs.system}) mmsg;
          inherit (inputs.scenefx.packages.${pkgs.system}) scenefx;
        };
        shellOverride = old: {
          nativeBuildInputs = old.nativeBuildInputs ++ [];
          buildInputs = old.buildInputs ++ [];
        };
      in {
        packages.default = mango;
        overlayAttrs = {
          inherit (config.packages) mango;
        };
        packages = {
          inherit mango;
        };
        devShells.default = mango.overrideAttrs shellOverride;
        formatter = pkgs.alejandra;
      };
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
    };
}
