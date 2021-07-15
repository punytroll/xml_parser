cmake_minimum_required(VERSION 3.10)

project(
	xml_parser
	LANGUAGES CXX
	VERSION 2.0
)

add_library(
	xml_parser STATIC
	source/xml_parser.cpp
)

target_include_directories(
	xml_parser
	PUBLIC include
)

add_executable(
	test_suite
	testing/test_suite.cpp
)

target_include_directories(
	test_suite
	PRIVATE include
)

target_link_libraries(
	test_suite
	PRIVATE xml_parser
)

enable_testing()

add_test(
	NAME TestSuite
	COMMAND test_suite
)
