test: test.o xml_parser.o
	$(CXX) $(CXXFLAGS) $^ -o $@
