project('xml_parser', 'cpp', version: '2.0.0', default_options : ['warning_level=3', 'cpp_std=c++2a'])

libxml_parser = library('xml_parser', ['source/xml_parser.cpp'], include_directories: 'include')

test_suite = executable('test_suite', ['testing/test_suite.cpp'], include_directories: 'include', link_with: libxml_parser)

test('test suite', test_suite)
