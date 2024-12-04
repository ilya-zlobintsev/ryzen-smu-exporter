project(
  'ryzen_smu_exporter',
  'c',
  'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++20'],
)

incdir = include_directories('lib/ryzen-smu/lib', is_system: true)
libsmu = static_library('libsmu', 'lib/ryzen-smu/lib/libsmu.c', include_directories: incdir)
libsmu_dep = declare_dependency(link_with: libsmu)

cmake = import('cmake')

prometheus_proj = cmake.subproject('prometheus-cpp')
prometheus_core = prometheus_proj.dependency('core')
prometheus_pull = prometheus_proj.dependency('pull')

exe = executable(
  'ryzen_smu_exporter',
  'ryzen_smu_exporter.cpp',
  install: true,
  dependencies: [libsmu_dep, prometheus_core, prometheus_pull],
  include_directories: incdir,
)

test('basic', exe)