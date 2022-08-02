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
#include <iostream>
#include <sstream>

#include <xml_parser/xml_parser.h>

class TestParser : public XMLParser
{
public:
    TestParser(std::istream & InputStream) :
        XMLParser(InputStream)
    {
    }
    
    std::string const & GetResult() const
    {
        return m_Result;
    }
private:
    auto Comment(std::string const & Comment) -> void override
    {
        m_Result += "{" + Comment + "}";
    }
    
    auto ElementStart(std::string const & TagName, std::map<std::string, std::string> const & Attributes) -> void override
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
    
    auto Text(std::string const & Text) -> void override
    {
        m_Result += '(' + Text + ')';
    }
    
    std::string m_Result;
};

auto Test(std::string const & XMLString, std::string const & TestString) -> void
{
    //~ std::cout << ">>>>" << std::endl;
    
    auto XMLStream = std::stringstream{XMLString};
    auto Parser = TestParser{XMLStream};
    
    Parser.Parse();
    
    auto ResultString = Parser.GetResult();
    
    if(ResultString != TestString)
    {
        throw std::runtime_error{"The XML string \"" + XMLString + "\" did not evaluate to the test string:\n        Expected result: \"" + TestString + "\"\n            Real result: \"" + ResultString + "\""};
    }
    //~ std::cout << "<<<<" << std::endl;
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char * argv[]) -> int
{
    Test("<root/>", "[+root][-root]");
    Test("<   root/>", "[+root][-root]");
    Test("<root />", "[+root][-root]");
    Test("<root></root>", "[+root][-root]");
    Test("<root>text</root>", "[+root](text)[-root]");
    Test("<root>=</root>", "[+root](=)[-root]");
    Test("<root>/</root>", "[+root](/)[-root]");
    Test("<root>&amp;</root>", "[+root](&)[-root]");
    Test("<root>&lt;</root>", "[+root](<)[-root]");
    Test("<root>&gt;</root>", "[+root](>)[-root]");
    Test("<root>!</root>", "[+root](!)[-root]");
    Test("<root>-</root>", "[+root](-)[-root]");
    Test("<root>;</root>", "[+root](;)[-root]");
    Test("<root>--</root>", "[+root](--)[-root]");
    Test("<root>---</root>", "[+root](---)[-root]");
    Test("<root>!--</root>", "[+root](!--)[-root]");
    Test("<root>></root>", "[+root](>)[-root]");
    Test("<root>\"</root>", "[+root](\")[-root]");
    Test("<root attribute=\"value\"/>", "[+root|attribute=value][-root]");
    Test("<root attribute1=\"value1\" attribute2=\"value2\"/>", "[+root|attribute1=value1|attribute2=value2][-root]");
    Test("<root attribute=\"  value\"/>", "[+root|attribute=  value][-root]");
    Test("<root attribute=\"value  \"/>", "[+root|attribute=value  ][-root]");
    Test("<root attribute=\"va  lue\"/>", "[+root|attribute=va  lue][-root]");
    Test("<root attribute =\"value\"/>", "[+root|attribute=value][-root]");
    Test("<root attribute= \"value\"/>", "[+root|attribute=value][-root]");
    Test("<root attribute = \"value\"/>", "[+root|attribute=value][-root]");
    Test("<root attribute=\"value\"></root>", "[+root|attribute=value][-root]");
    Test("<root attribute=\"value\">text</root>", "[+root|attribute=value](text)[-root]");
    Test("<tag name=\"mime-type\">image/jpg</tag>", "[+tag|name=mime-type](image/jpg)[-tag]");
    Test("<root><!-- Comment --></root>", "[+root]{ Comment }[-root]");
    Test("<root><!-- - --></root>", "[+root]{ - }[-root]");
    Test("<root><!-- < --></root>", "[+root]{ < }[-root]");
    Test("<root><!-- > --></root>", "[+root]{ > }[-root]");
    Test("<root><!-- ; --></root>", "[+root]{ ; }[-root]");
    Test("<root><!-- ! --></root>", "[+root]{ ! }[-root]");
    Test("<root><!-- = --></root>", "[+root]{ = }[-root]");
    Test("<root><!-- & --></root>", "[+root]{ & }[-root]");
    Test("<root><!-- \" --></root>", "[+root]{ \" }[-root]");
    Test("<root><!-- / --></root>", "[+root]{ / }[-root]");
    Test("<root><!-- -< --></root>", "[+root]{ -< }[-root]");
    Test("<root><!-- -> --></root>", "[+root]{ -> }[-root]");
    Test("<root><!-- -! --></root>", "[+root]{ -! }[-root]");
    Test("<root><!-- -; --></root>", "[+root]{ -; }[-root]");
    Test("<root><!-- -& --></root>", "[+root]{ -& }[-root]");
    Test("<root><!-- -= --></root>", "[+root]{ -= }[-root]");
    Test("<root><!-- -\" --></root>", "[+root]{ -\" }[-root]");
    Test("<root><!-- -/ --></root>", "[+root]{ -/ }[-root]");
    Test("<root><!-- \"Hallo Test\" - This -! is -\" a -< conclusive -> test -= of <> special /= characters!</root>&amp;-; --></root>", "[+root]{ \"Hallo Test\" - This -! is -\" a -< conclusive -> test -= of <> special /= characters!</root>&amp;-; }[-root]");
    Test("<r><h a=\"v\">H<s t=\"f\"/> ! </h></r>", "[+r][+h|a=v](H)[+s|t=f][-s]( ! )[-h][-r]");
    Test("<call function=\"insert\">\
    <parameter name=\"identifier\">127.0.0.1</parameter>\
    <parameter name=\"title\">localhost</parameter>\
</call>", "[+call|function=insert](    )[+parameter|name=identifier](127.0.0.1)[-parameter](    )[+parameter|name=title](localhost)[-parameter][-call]");
    Test("<root att=\"h&amp;m\"/>", "[+root|att=h&m][-root]");
    Test("<root att=\"h&quot;m\"/>", "[+root|att=h\"m][-root]");
    Test("<root att=\"h&apos;m\"/>", "[+root|att=h'm][-root]");
    Test("<root att=\"h&apos;&amp;m\"/>", "[+root|att=h'&m][-root]");
    Test("<root-hyphen/>", "[+root-hyphen][-root-hyphen]");
    Test("<root-hyphen></root-hyphen>", "[+root-hyphen][-root-hyphen]");
    Test("<root attribute-hyphen=\"\"/>", "[+root|attribute-hyphen=][-root]");
    Test("<root><!-- - --></root>", "[+root]{ - }[-root]");
    Test("<root><!-- - - --></root>", "[+root]{ - - }[-root]");
    Test("<root><!-- - - > --></root>", "[+root]{ - - > }[-root]");
    Test("<root><!-- -- --></root>", "[+root]{ -- }[-root]");
    Test("<root><!-- --- --></root>", "[+root]{ --- }[-root]");
    Test("<root><!-- ---- --></root>", "[+root]{ ---- }[-root]");
    Test("<root><!-- <t-e-s-t></t-e-s-t> --></root>", "[+root]{ <t-e-s-t></t-e-s-t> }[-root]");
}
