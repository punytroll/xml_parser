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
	
	const std::string & GetResult(void) const
	{
		return m_Result;
	}
private:
	virtual void Comment(const std::string & Comment) override
	{
		m_Result += "{" + Comment + "}";
	}
	
	virtual void ElementStart(const std::string & TagName, const std::map< std::string, std::string > & Attributes) override
	{
		m_Result += "[+" + TagName;
		for(std::map< std::string, std::string >::const_iterator Iterator = Attributes.begin(); Iterator != Attributes.end(); ++Iterator)
		{
			m_Result += '|' + Iterator->first + '=' + Iterator->second;
		}
		m_Result += ']';
	}
	
	virtual void ElementEnd(const std::string & TagName) override
	{
		m_Result += "[-" + TagName + ']';
	}
	
	virtual void Text(const std::string & Text) override
	{
		m_Result += '(' + Text + ')';
	}
	
	std::string m_Result;
};

std::string TestParse(const std::string & XMLString)
{
	std::stringstream XMLStream{XMLString};
	TestParser Parser{XMLStream};
	
	Parser.Parse();
	
	return Parser.GetResult();
}

void Test(const std::string XMLString, const std::string & TestString)
{
	//~ std::cout << ">>>>" << std::endl;
	
	auto ResultString{TestParse(XMLString)};
	
	if(ResultString != TestString)
	{
		throw std::runtime_error{"The XML string \"" + XMLString + "\" did not evaluate to the test string:\n        Expected result: \"" + TestString + "\"\n            Real result: \"" + ResultString + "\""};
	}
	//~ std::cout << "<<<<" << std::endl;
}

int main(int argc, char ** argv)
{
	Test("<root/>", "[+root][-root]");
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
</call>", "[+call|function=insert](\t)[+parameter|name=identifier](127.0.0.1)[-parameter](\t)[+parameter|name=title](localhost)[-parameter][-call]");
	Test("<root att=\"h&amp;m\"/>", "[+root|att=h&m][-root]");
	Test("<root att=\"h&quot;m\"/>", "[+root|att=h\"m][-root]");
	Test("<root att=\"h&apos;m\"/>", "[+root|att=h'm][-root]");
	Test("<root att=\"h&apos;&amp;m\"/>", "[+root|att=h'&m][-root]");
	Test("<root-hyphen/>", "[+root-hyphen][-root-hyphen]");
	Test("<root-hyphen></root-hyphen>", "[+root-hyphen][-root-hyphen]");
	Test("<root attribute-hyphen=\"\"/>", "[+root|attribute-hyphen=][-root]");
}
