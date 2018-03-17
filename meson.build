project('hbci4java-glib', 'c')

gnome = import('gnome')
pkg = import('pkgconfig')

# declare dependencies:
fedora_java_home = '/usr/lib/jvm/java'
flatpak_java_home = '/usr/lib/sdk/openjdk9/jvm/openjdk-9'
if run_command('[', '-d', fedora_java_home, ']').returncode() == 0
  java_home = fedora_java_home
  java_lib_dir = java_home + '/jre/lib/amd64/server'
elif run_command('[', '-d', flatpak_java_home, ']').returncode() == 0
  java_home = flatpak_java_home
  java_lib_dir = java_home + '/lib/server'
elif
  error('Could not find java home')
endif
message('Using '+java_home+' as java home')

java_inc = include_directories(java_home + '/include', java_home + '/include/linux')
java_dep = declare_dependency(
  link_args: ['-Wl,-R'+java_lib_dir, '-L'+java_lib_dir, '-ljvm'],
  include_directories : java_inc) 

gobject_dep = dependency('gobject-2.0')
gio_dep = dependency('gio-2.0')

# list source files
public_headers = ['ghbci/ghbci-statement.h', 
	'ghbci/ghbci-account.h',
	'ghbci/ghbci-context.h']

private_headers = [
	'ghbci/ghbci-statement-private.h',
	'ghbci/ghbci-account-private.h',
	'ghbci/ghbci-context-private.h']

source_c = [
	'ghbci/ghbci-statement.c',
	'ghbci/ghbci-account.c',
	'ghbci/ghbci-context.c']

marshall_sources = gnome.genmarshal(
  'ghbci-marshal',
  prefix: 'ghbci_marshal',
  sources: ['ghbci/marshal.list'])

ghbci = shared_library(
  'ghbci-0.1',
  source_c + public_headers + private_headers + marshall_sources,
  dependencies: [java_dep, gobject_dep, gio_dep],
  c_args: '-DDATA_DIR="'+get_option('datadir')+'"',
  install_rpath: java_lib_dir,
  install: true)

gnome.generate_gir(
  ghbci,
  sources: source_c + public_headers + marshall_sources,
  includes: ['GObject-2.0'],
  namespace: 'GHbci',
  nsversion: '0.1',
  symbol_prefix: 'ghbci',
  header: 'ghbci.h',
  install: true)

pkg.generate(
  name: 'ghbci',
  description: 'A GObject wrapper of the HBCI4Java library',
  version: '0.1',
  filebase: 'ghbci-0.1',
  libraries: ghbci,
  subdirs: ['ghbci'],
  requires: ['glib-2.0', 'gobject-2.0'])

install_headers(public_headers + ['ghbci/ghbci.h'], subdir : 'ghbci')

install_data(
  'ghbci/hbci4java.jar',
  install_dir: join_paths(get_option('datadir'), 'ghbci'))


# tests

tests = executable(
  'test-statement',
  'tests/test-statement.c',
  dependencies: [java_dep, gobject_dep, gio_dep],
  link_with: [ghbci])
test('test-statement', tests)

# doc

gnome.gtkdoc(
  'ghbci',
  main_xml: join_paths(meson.source_root(), 'doc', 'reference', 'ghbci-docs.xml'),
  src_dir: [join_paths(meson.source_root(), 'ghbci')],
  ignore_headers: private_headers)
