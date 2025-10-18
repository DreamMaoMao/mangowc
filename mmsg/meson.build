wayland_scanner = find_program('wayland-scanner')

dwl_ipc_protocol = 'protocols/dwl-ipc-unstable-v2.xml'

dwl_ipc_header = custom_target(
  'dwl-ipc-header',
  input: dwl_ipc_protocol,
  output: 'dwl-ipc-unstable-v2-protocol.h',
  command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
  build_by_default: true,
)

dwl_ipc_source = custom_target(
  'dwl-ipc-source',
  input: dwl_ipc_protocol,
  output: 'dwl-ipc-unstable-v2-protocol.c',
  command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
  build_by_default: true,
)

executable('mmsg',
  'src/mmsg.c',
  dwl_ipc_source,
  dependencies: [
    dependency('wayland-client'),
  ],
  include_directories: include_directories('.'),
  install: true,
  c_args: [
    '-g',
    '-Wno-unused-function',
  ],
)