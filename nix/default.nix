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
  meson,
  ninja,
  scenefx,
  wlroots_0_19,
  libGL,
  enableXWayland ? true,
  debug ? false,
}:
stdenv.mkDerivation {
  pname = "mango";
  version = "nightly";

  src = builtins.path {
    path = ../.;
    name = "source";
  };

  mesonFlags = [
    (lib.mesonEnable "xwayland" enableXWayland)
    (lib.mesonBool "asan" debug)
  ];

  preConfigure = ''
    rm -rf build
  '';

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
