{
  lib,
  libX11,
  libinput,
  libxcb,
  libxkbcommon,
  pcre2,
  pixman,
  pkg-config,
  stdenv,
  wayland,
  wayland-protocols,
  wayland-scanner,
  xcbutilwm,
  xwayland,
  enableXWayland ? true,
  meson,
  ninja,
  scenefx,
  wlroots_0_19,
  libGL,
}: let
  pname = "mango";
in
  stdenv.mkDerivation {
    inherit pname;
    version = "nightly";

    src = builtins.path {
      path = ../.;
      name = "source";
    };

    nativeBuildInputs = [
      meson
      ninja
      pkg-config
      wayland-scanner
    ];

    buildInputs =
      [
        libinput
        libxcb
        libxkbcommon
        pcre2
        pixman
        wayland
        wayland-protocols
        wlroots_0_19
        scenefx
        libGL
      ]
      ++ lib.optionals enableXWayland [
        libX11
        xcbutilwm
        xwayland
      ];

    passthru = {
      providedSessions = ["mango"];
    };

    meta = {
      mainProgram = "mango";
      description = "A streamlined but feature-rich Wayland compositor";
      homepage = "https://github.com/DreamMaoMao/mango";
      license = lib.licenses.gpl3Plus;
      maintainers = [];
      platforms = lib.platforms.unix;
    };
  }
