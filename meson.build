project(
  'xml_parser',
  'cpp',
  version: '2.0.1',
  meson_version: '>=0.63.0',
  default_options : ['cpp_std=c++20', 'warning_level=3', 'werror=true']
)

xml_parser_library = library(
  'xml_parser',
  sources: ['source/xml_parser.cpp'],
  include_directories: include_directories('include')
)

xml_parser_library_dependency = declare_dependency(
  include_directories: include_directories('include'),
  link_with: xml_parser_library
)

test(
  'xml_parser',
  executable(
    'xml_parser_test',
    sources: ['testing/test_suite.cpp'],
    dependencies: [xml_parser_library_dependency]
  )
)
