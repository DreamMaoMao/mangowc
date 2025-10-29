(define-module (mangowc)
  #:use-module (guix packages)
  #:use-module (guix download)
  #:use-module (guix git-download)
  #:use-module (gnu packages wm)
  #:use-module (gnu packages freedesktop)
  #:use-module (gnu packages xdisorg)
  #:use-module (gnu packages pciutils)
  #:use-module (gnu packages admin)
  #:use-module (gnu packages pcre)
  #:use-module (gnu packages xorg)
  #:use-module (gnu packages build-tools)
  #:use-module (gnu packages ninja)
  #:use-module (gnu packages pkg-config)
  #:use-module (guix build-system meson)
  #:use-module (guix licenses))


(define-public mangowc
  (package
    (name "mangowc")
    (version "0.10.4")
    (source
     (origin
       (method git-fetch)
       (uri (git-reference
             (url "https://github.com/DreamMaoMao/mangowc")
             (commit "0.10.4")))
       (sha256
        (base32 "0cayb2r69zcp5q810bqhq27xi0b5dlk81qwl6zj6aqjphh6yzpv9"))))
    (build-system meson-build-system)
    (inputs (list wayland
                  wayland-protocols
                  libinput
                  libdrm
                  libxkbcommon
                  pixman
                  libdisplay-info
                  libliftoff
                  hwdata
                  seatd
                  pcre2
                  libxcb
                  xcb-util-wm
                  wlroots
                  scenefx))
    (native-inputs (list meson ninja pkg-config))
    (home-page "https://github.com/DreamMaoMao/mangowc")
    (synopsis "Wayland compositor based on wlroots and scenefx")
    (description "A Wayland compositor based on wlroots and scenefx,
inspired by dwl but aiming to be more feature-rich.")
    (license gpl3)))
