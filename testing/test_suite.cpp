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

#include <cassert>
#include <format>
#include <iostream>
#include <sstream>

#include <xml_parser/parser.h>

class ContentParser : public XML::Parser
{
public:
    ContentParser(std::istream & InputStream) :
        XML::Parser{InputStream}
    {
    }
    
    std::string const & GetResult() const
    {
        return m_Result;
    }
private:
    auto Comment(std::string const & Comment, [[maybe_unused]] XML::Location const & StartLocation) -> void override
    {
        m_Result += "{" + Comment + "}";
    }
    
    auto ElementStart(std::string const & TagName, std::map<std::string, std::string> const & Attributes, [[maybe_unused]] XML::Location const & StartLocation) -> void override
    {
        m_Result += "[+" + TagName;
        for(auto Iterator = Attributes.begin(); Iterator != Attributes.end(); ++Iterator)
        {
            m_Result += '|' + Iterator->first + '=' + Iterator->second;
        }
        m_Result += ']';
    }
    
    auto ElementEnd(std::string const & TagName) -> void override
    {
        m_Result += "[-" + TagName + ']';
    }
    
    auto Text(std::string const & Text, [[maybe_unused]] XML::Location const & StartLocation) -> void override
    {
        m_Result += '(' + Text + ')';
    }
    
    std::string m_Result;
};

class PositionParser : public XML::Parser
{
public:
    PositionParser(std::istream & InputStream) :
        XML::Parser{InputStream}
    {
    }
    
    std::string const & GetResult() const
    {
        return m_Result;
    }
private:
    auto Comment(std::string const & Comment, XML::Location const & StartLocation) -> void override
    {
        m_Result += std::format("{}:{}#{}", StartLocation.Line, StartLocation.Column, Comment);
    }
    
    auto ElementStart(std::string const & TagName, [[maybe_unused]] std::map<std::string, std::string> const & Attributes, XML::Location const & StartLocation) -> void override
    {
        m_Result += std::format("{}:{}+{}", StartLocation.Line, StartLocation.Column, TagName);
    }
    
    auto ElementEnd([[maybe_unused]] std::string const & TagName) -> void override
    {
    }
    
    auto Text(std::string const & Text, XML::Location const & StartLocation) -> void override
    {
        m_Result += std::format("{}:{}={}", StartLocation.Line, StartLocation.Column, Text);
    }
    
    std::string m_Result;
};

auto TestContent(std::string const & XMLString, std::string const & TestString) -> void
{
    //~ std::cout << ">>>> parsing \"" << XMLString << '"' << std::endl;
    
    auto XMLStream = std::stringstream{XMLString};
    auto Parser = ContentParser{XMLStream};
    
    Parser.Parse();
    
    auto ResultString = Parser.GetResult();
    
    if(ResultString != TestString)
    {
        throw std::runtime_error{std::format("The XML string \"{}\" did not evaluate to the content test string:\n        Expected result: \"{}\"\n          Actual result: \"{}\"", XMLString, TestString, ResultString)};
    }
    //~ std::cout << "<<<<" << std::endl;
}

auto TestPosition(std::string const & XMLString, std::string const & TestString) -> void
{
    //~ std::cout << ">>>> parsing \"" << XMLString << '"' << std::endl;
    
    auto XMLStream = std::stringstream{XMLString};
    auto Parser = PositionParser{XMLStream};
    
    Parser.Parse();
    
    auto ResultString = Parser.GetResult();
    
    if(ResultString != TestString)
    {
        throw std::runtime_error{std::format("The XML string \"{}\" did not evaluate to the position test string:\n        Expected result: \"{}\"\n          Actual result: \"{}\"", XMLString, TestString, ResultString)};
    }
    //~ std::cout << "<<<<" << std::endl;
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char * argv[]) -> int
{
    // testing positions
    TestPosition("<root/>", "0:0+root");
    TestPosition(" <root/>", "0:0= 0:1+root");
    TestPosition("<root>text</root>", "0:0+root0:6=text");
    TestPosition("<root>\ntext\n</root>", "0:0+root0:6=\ntext\n");
    TestPosition("<root>\n<child/>\n</root>", "0:0+root0:6=\n1:0+child1:8=\n");
    TestPosition("<root>\n\t<child/>\n</root>", "0:0+root0:6=\n\t1:1+child1:9=\n");
    TestPosition("<root>\n\t<child></child>\n</root>", "0:0+root0:6=\n\t1:1+child1:16=\n");
    TestPosition("<root>\n\t<child>text</child>\n</root>", "0:0+root0:6=\n\t1:1+child1:8=text1:20=\n");
    TestPosition("<root><!-- comment --></root>", "0:0+root0:6# comment ");
    TestPosition("<root><!-- comment -->text</root>", "0:0+root0:6# comment 0:22=text");
    TestPosition("<root>text<!-- comment --></root>", "0:0+root0:6=text0:10# comment ");
    // testing content
    TestContent("<root/>", "[+root][-root]");
    TestContent("<   root/>", "[+root][-root]");
    TestContent("<root />", "[+root][-root]");
    TestContent("<root></root>", "[+root][-root]");
    TestContent("<root>text</root>", "[+root](text)[-root]");
    TestContent("<root>=</root>", "[+root](=)[-root]");
    TestContent("<root>/</root>", "[+root](/)[-root]");
    TestContent("<root>&amp;</root>", "[+root](&)[-root]");
    TestContent("<root>&lt;</root>", "[+root](<)[-root]");
    TestContent("<root>&gt;</root>", "[+root](>)[-root]");
    TestContent("<root>!</root>", "[+root](!)[-root]");
    TestContent("<root>-</root>", "[+root](-)[-root]");
    TestContent("<root>;</root>", "[+root](;)[-root]");
    TestContent("<root>--</root>", "[+root](--)[-root]");
    TestContent("<root>---</root>", "[+root](---)[-root]");
    TestContent("<root>!--</root>", "[+root](!--)[-root]");
    TestContent("<root>></root>", "[+root](>)[-root]");
    TestContent("<root>\"</root>", "[+root](\")[-root]");
    TestContent("<root>'</root>", "[+root](')[-root]");
    TestContent("<root attribute=\"value\"/>", "[+root|attribute=value][-root]");
    TestContent("<root attribute=\"value'\"/>", "[+root|attribute=value'][-root]");
    TestContent("<root attribute='value'/>", "[+root|attribute=value][-root]");
    TestContent("<root attribute='value\"'/>", "[+root|attribute=value\"][-root]");
    TestContent("<root attribute1=\"value1\" attribute2=\"value2\"/>", "[+root|attribute1=value1|attribute2=value2][-root]");
    TestContent("<root attribute=\"  value\"/>", "[+root|attribute=  value][-root]");
    TestContent("<root attribute=\"value  \"/>", "[+root|attribute=value  ][-root]");
    TestContent("<root attribute=\"va  lue\"/>", "[+root|attribute=va  lue][-root]");
    TestContent("<root attribute =\"value\"/>", "[+root|attribute=value][-root]");
    TestContent("<root attribute= \"value\"/>", "[+root|attribute=value][-root]");
    TestContent("<root attribute = \"value\"/>", "[+root|attribute=value][-root]");
    TestContent("<root attribute=\"value\"></root>", "[+root|attribute=value][-root]");
    TestContent("<root attribute=\"value\">text</root>", "[+root|attribute=value](text)[-root]");
    TestContent("<tag name=\"mime-type\">image/jpg</tag>", "[+tag|name=mime-type](image/jpg)[-tag]");
    TestContent("<root><!-- Comment --></root>", "[+root]{ Comment }[-root]");
    TestContent("<root><!-- - --></root>", "[+root]{ - }[-root]");
    TestContent("<root><!-- < --></root>", "[+root]{ < }[-root]");
    TestContent("<root><!-- > --></root>", "[+root]{ > }[-root]");
    TestContent("<root><!-- ; --></root>", "[+root]{ ; }[-root]");
    TestContent("<root><!-- ! --></root>", "[+root]{ ! }[-root]");
    TestContent("<root><!-- = --></root>", "[+root]{ = }[-root]");
    TestContent("<root><!-- & --></root>", "[+root]{ & }[-root]");
    TestContent("<root><!-- \" --></root>", "[+root]{ \" }[-root]");
    TestContent("<root><!-- / --></root>", "[+root]{ / }[-root]");
    TestContent("<root><!-- -< --></root>", "[+root]{ -< }[-root]");
    TestContent("<root><!-- -> --></root>", "[+root]{ -> }[-root]");
    TestContent("<root><!-- -! --></root>", "[+root]{ -! }[-root]");
    TestContent("<root><!-- -; --></root>", "[+root]{ -; }[-root]");
    TestContent("<root><!-- -& --></root>", "[+root]{ -& }[-root]");
    TestContent("<root><!-- -= --></root>", "[+root]{ -= }[-root]");
    TestContent("<root><!-- -\" --></root>", "[+root]{ -\" }[-root]");
    TestContent("<root><!-- -/ --></root>", "[+root]{ -/ }[-root]");
    TestContent("<root><!-- \"Hallo Test\" - This -! is -\" a -< conclusive -> test -= of <> special /= characters!</root>&amp;-; --></root>", "[+root]{ \"Hallo Test\" - This -! is -\" a -< conclusive -> test -= of <> special /= characters!</root>&amp;-; }[-root]");
    TestContent("<r><h a=\"v\">H<s t=\"f\"/> ! </h></r>", "[+r][+h|a=v](H)[+s|t=f][-s]( ! )[-h][-r]");
    TestContent("<call function=\"insert\">\
    <parameter name=\"identifier\">127.0.0.1</parameter>\
    <parameter name=\"title\">localhost</parameter>\
</call>", "[+call|function=insert](    )[+parameter|name=identifier](127.0.0.1)[-parameter](    )[+parameter|name=title](localhost)[-parameter][-call]");
    TestContent("<root attribute=\"h&amp;m\"/>", "[+root|attribute=h&m][-root]");
    TestContent("<root attribute=\"h&quot;m\"/>", "[+root|attribute=h\"m][-root]");
    TestContent("<root attribute=\"h&apos;m\"/>", "[+root|attribute=h'm][-root]");
    TestContent("<root attribute=\"h&apos;&amp;m\"/>", "[+root|attribute=h'&m][-root]");
    TestContent("<root attribute=\"!\"/>", "[+root|attribute=!][-root]");
    TestContent("<root attribute=\"value!\"/>", "[+root|attribute=value!][-root]");
    TestContent("<root attribute=\"!value\"/>", "[+root|attribute=!value][-root]");
    TestContent("<root attribute=\"val!ue\"/>", "[+root|attribute=val!ue][-root]");
    TestContent("<root attribute='!'/>", "[+root|attribute=!][-root]");
    TestContent("<root attribute='value!'/>", "[+root|attribute=value!][-root]");
    TestContent("<root attribute='!value'/>", "[+root|attribute=!value][-root]");
    TestContent("<root attribute='val!ue'/>", "[+root|attribute=val!ue][-root]");
    TestContent("<root attribute=\"=\"/>", "[+root|attribute==][-root]");
    TestContent("<root attribute=\"value=\"/>", "[+root|attribute=value=][-root]");
    TestContent("<root attribute=\"=value\"/>", "[+root|attribute==value][-root]");
    TestContent("<root attribute=\"val=ue\"/>", "[+root|attribute=val=ue][-root]");
    TestContent("<root attribute='='/>", "[+root|attribute==][-root]");
    TestContent("<root attribute='value='/>", "[+root|attribute=value=][-root]");
    TestContent("<root attribute='=value'/>", "[+root|attribute==value][-root]");
    TestContent("<root attribute='val=ue'/>", "[+root|attribute=val=ue][-root]");
    TestContent("<root-hyphen/>", "[+root-hyphen][-root-hyphen]");
    TestContent("<root-hyphen></root-hyphen>", "[+root-hyphen][-root-hyphen]");
    TestContent("<root attribute-hyphen=\"\"/>", "[+root|attribute-hyphen=][-root]");
    TestContent("<root><!-- - --></root>", "[+root]{ - }[-root]");
    TestContent("<root><!-- - - --></root>", "[+root]{ - - }[-root]");
    TestContent("<root><!-- - - > --></root>", "[+root]{ - - > }[-root]");
    TestContent("<root><!-- -- --></root>", "[+root]{ -- }[-root]");
    TestContent("<root><!-- --- --></root>", "[+root]{ --- }[-root]");
    TestContent("<root><!-- ---- --></root>", "[+root]{ ---- }[-root]");
    TestContent("<root><!-- <t-e-s-t></t-e-s-t> --></root>", "[+root]{ <t-e-s-t></t-e-s-t> }[-root]");
}
