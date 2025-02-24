{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      flake-parts,
      treefmt-nix,
      ...
    }@inputs:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [
        inputs.flake-parts.flakeModules.easyOverlay
      ];

      flake.hmModules.maomaowm = import ./nix/hm-modules.nix;

      perSystem =
        {
          config,
          pkgs,
          ...
        }:
        let
          inherit (pkgs)
            callPackage
            ;
          maomaowm = callPackage ./nix { };
          treefmtEval = treefmt-nix.lib.evalModule pkgs ./treefmt.nix;
        in
        {
          packages.default = maomaowm;
          overlayAttrs = {
            inherit (config.packages) maomaowm;
          };
          packages = {
            inherit maomaowm;
          };

          formatter = treefmtEval.config.build.wrapper;
        };
      systems = [ "x86_64-linux" ];
    };
}
