{
  lib,
  stdenv,
  fetchFromGitLab,
  meson,
  ninja,
  pkg-config,
  wayland-scanner,
  libGL,
  wayland,
  wayland-protocols,
  libinput,
  libxkbcommon,
  pixman,
  libcap,
  libgbm,
  xorg,
  libpng,
  ffmpeg,
  hwdata,
  seatd,
  vulkan-loader,
  glslang,
  libliftoff,
  libdisplay-info,
  lcms2,
  nixosTests,
  testers,
  cmake,

  enableXWayland ? true,
  xwayland ? null,
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "wlroots";
  version = "master";

  inherit enableXWayland;

  src = fetchFromGitLab {
    domain = "gitlab.freedesktop.org";
    owner = "wlroots";
    repo = "wlroots";
    rev = "99da6ccc87c2439b19c9298df6b72b29bbab89bb";
    hash = "sha256-mIRUDyZYLrrxTfGq+vlLvgx3wwCbX4ogHESavCZr3TU=";
  };

  # $out for the library and $examples for the example programs (in examples):
  outputs = [
    "out"
    "examples"
  ];

  strictDeps = true;
  depsBuildBuild = [
    pkg-config
  ];

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
    cmake
    wayland-scanner
    glslang
    hwdata
  ];

  buildInputs = [
    ffmpeg
    libliftoff
    libdisplay-info
    libGL
    libcap
    libinput
    libpng
    libxkbcommon
    libgbm
    pixman
    seatd
    vulkan-loader
    wayland
    wayland-protocols
    xorg.libX11
    xorg.xcbutilerrors
    xorg.xcbutilimage
    xorg.xcbutilrenderutil
    xorg.xcbutilwm
    lcms2
  ] ++ lib.optional finalAttrs.enableXWayland xwayland;

  mesonFlags = lib.optional (!finalAttrs.enableXWayland) "-Dxwayland=disabled";

  postFixup = ''
    # Install ALL example programs to $examples:
    # screencopy dmabuf-capture input-inhibitor layer-shell idle-inhibit idle
    # screenshot output-layout multi-pointer rotation tablet touch pointer
    # simple
    mkdir -p $examples/bin
    cd ./examples
    for binary in $(find . -executable -type f -printf '%P\n' | grep -vE '\.so'); do
      cp "$binary" "$examples/bin/wlroots-$binary"
    done
  '';

  # Test via TinyWL (the "minimum viable product" Wayland compositor based on wlroots):
  passthru.tests = {
    tinywl = nixosTests.tinywl;
    pkg-config = testers.hasPkgConfigModules {
      package = finalAttrs.finalPackage;
    };
  };

  meta = {
    description = "Modular Wayland compositor library";
    longDescription = ''
      Pluggable, composable, unopinionated modules for building a Wayland
      compositor; or about 50,000 lines of code you were going to write anyway.
    '';
    inherit (finalAttrs.src.meta) homepage;
    license = lib.licenses.mit;
    platforms = lib.platforms.linux;
    maintainers = with lib.maintainers; [
      primeos
      synthetica
      rewine
    ];
    pkgConfigModules = [
      (
        if lib.versionOlder finalAttrs.version "0.18" then
          "wlroots"
        else
          "wlroots-${lib.versions.majorMinor finalAttrs.version}"
      )
    ];
  };
})
