/**
 * Copyright 2021 Hagen Möbius
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
**/

#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <istream>
#include <map>

class XMLParser
{
public:
    XMLParser(std::istream & InputStream);
    virtual ~XMLParser();
    auto Parse() -> void;
protected:
    virtual auto Comment(std::string const & Comment) -> void;
    virtual auto ElementStart(std::string const & TagName, std::map<std::string, std::string> const & Attributes) -> void;
    virtual auto ElementEnd(std::string const & TagName) -> void;
    virtual auto Text(std::string const & Text) -> void;
private:
    std::istream & m_InputStream;
};

#endif
