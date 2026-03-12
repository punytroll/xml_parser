/**
 * Copyright 2021-2022 Hagen Möbius
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

#ifndef XML_PARSER__PARSER_H
#define XML_PARSER__PARSER_H

#include <cstdint>
#include <istream>
#include <map>

namespace XML
{
    class Location
    {
    public:
        std::uint64_t Column;
        std::uint64_t Line;
    };
    
    class Parser
    {
    public:
        Parser(std::istream & InputStream);
        virtual ~Parser() = default;
        auto Parse() -> void;
    protected:
        virtual auto Comment(std::string const & Comment, XML::Location const & StartLocation) -> void;
        virtual auto ElementStart(std::string const & TagName, std::map<std::string, std::string> const & Attributes, XML::Location const & StartLocation) -> void;
        virtual auto ElementEnd(std::string const & TagName) -> void;
        virtual auto Text(std::string const & Text, XML::Location const & StartLocation) -> void;
    private:
        std::istream & m_InputStream;
    };
}

#endif
