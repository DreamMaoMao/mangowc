project('maomao', ['c', 'cpp'], version: '0.1.5')

subdir('protocols')

cc = meson.get_compiler('c')
libm = cc.find_library('m')
xcb = dependency('xcb', required: get_option('xwayland'))
xlibs = dependency('xcb-icccm', required: get_option('xwayland'))
wayland_server_dep = dependency('wayland-server')
wlroots_dep = dependency('wlroots', version: ['>=0.17.0', '<0.18.0'])
xkbcommon_dep = dependency('xkbcommon')
libinput_dep = dependency('libinput')
libwayland_client_dep = dependency('wayland-client')

c_args = [
  '-g',
  '-Wno-unused-function',
  '-DWLR_USE_UNSTABLE',
  '-D_POSIX_C_SOURCE=200809L',
  '-DVERSION="@0@"'.format(meson.project_version()),
]

if xcb.found() and xlibs.found()
  c_args += '-DXWAYLAND'
  c_args += '-DIM'
endif

executable(
  'maomao',
  'maomao.c',
  'util.c',
  wayland_sources,
  dependencies: [
    libm,
    xcb,
    xlibs,
    wayland_server_dep,
    wlroots_dep,
    xkbcommon_dep,
    libinput_dep,
    libwayland_client_dep,
  ],
  install: true,
  c_args: c_args,
)

sysconfdir = get_option('sysconfdir')
prefix = get_option('prefix')

config = configuration_data()
config.set('sysconfdir', sysconfdir)
config.set('prefix', prefix)
configure_file(
  configuration: config,
  input: 'config.in',
  output: 'config.conf',
  install_dir: join_paths(sysconfdir, 'maomao'),
)

desktop_install_dir = join_paths(prefix, 'share/wayland-sessions')
install_data('maomao.desktop', install_dir: desktop_install_dir)
