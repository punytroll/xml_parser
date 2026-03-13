project(
  'xml_parser',
  'cpp',
  version: '26.3.1',
  meson_version: '>=1.10.0',
  default_options: [
    'cpp_std=c++23',
    'warning_level=3',
    'werror=true'
  ]
)

xml_parser_library_include_directories = include_directories('include')

xml_parser_library = library(
  'xml_parser',
  sources: 'source/parser.cpp',
  include_directories: xml_parser_library_include_directories,
)

xml_parser_library_dependency = declare_dependency(
  include_directories: xml_parser_library_include_directories,
  link_with: xml_parser_library,
)

meson.override_dependency('xml_parser', xml_parser_library_dependency)

test(
  'xml_parser',
  executable(
    'xml_parser_test',
    build_by_default: false,
    dependencies: xml_parser_library_dependency,
    sources: 'testing/test_suite.cpp',
  )
)
