(define-module (mangowc)
  #:use-module (guix download)
  #:use-module (guix gexp)
  #:use-module (guix packages)
  #:use-module (guix utils)
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


(define-public mangowc-git
  (package
    (name "mangowc")
    (version "0.10.4")
    (source (local-file "." "mangowc-checkout"
                        #:recursive? #t
                        #:select? (or (git-predicate (current-source-directory))
                                      (const #t))))
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

mangowc-git
