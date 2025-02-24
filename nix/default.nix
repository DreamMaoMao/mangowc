{
  lib,
  libX11,
  libinput,
  libxcb,
  libxkbcommon,
  pixman,
  pkg-config,
  stdenv,
  wayland,
  wayland-protocols,
  wayland-scanner,
  wlroots_0_17,
  xcbutilwm,
  xwayland,
  enableXWayland ? true,
  meson,
  ninja,
}:
let
  pname = "maomaowm";
in
stdenv.mkDerivation {
  inherit pname;
  version = "nightly";

  src = ../.;

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
      pixman
      wayland
      wayland-protocols
      wlroots_0_17
    ]
    ++ lib.optionals enableXWayland [
      libX11
      xcbutilwm
      xwayland
    ];

  meta = {
    mainProgram = "maomao";
    description = "A streamlined but feature-rich Wayland compositor";
    homepage = "https://github.com/DreamMaoMao/maomaowm";
    license = lib.licenses.mit;
    maintainers = with lib.maintainers; [ ];
    platforms = lib.platforms.unix;
  };
}
