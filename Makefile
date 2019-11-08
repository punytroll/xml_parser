CXXFLAGS = -Wall -Werror -O0 -g -std=c++17

check: test
	./test

clean:
	$(RM) test
	$(RM) test.o
	$(RM) xml_parser.o

test: test.o xml_parser.o
	$(CXX) $(LDFLAGS) $^ -o $@

test.o: test.cpp xml_parser.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

xml_parser.o: xml_parser.cpp xml_parser.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
