/**
 * Copyright (C) 2007  Hagen Möbius
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

/**
 * This is version 1.7.1 of the xml parser.
 **/

#include <assert.h>

#include <iostream>
#include <sstream>

#include "xml_parser.h"

class TestParser : public XMLParser
{
public:
	TestParser(std::istream & InputStream) :
		XMLParser(InputStream)
	{
	}
	
	const std::string & GetResult(void) const
	{
		return _Result;
	}
private:
	virtual void Comment(const std::string & Comment) override
	{
		_Result += "{" + Comment + "}";
	}
	
	virtual void ElementStart(const std::string & TagName, const std::map< std::string, std::string > & Attributes) override
	{
		_Result += "[+" + TagName;
		for(std::map< std::string, std::string >::const_iterator Iterator = Attributes.begin(); Iterator != Attributes.end(); ++Iterator)
		{
			_Result += '|' + Iterator->first + '=' + Iterator->second;
		}
		_Result += ']';
	}
	
	virtual void ElementEnd(const std::string & TagName) override
	{
		_Result += "[-" + TagName + ']';
	}
	
	virtual void Text(const std::string & Text) override
	{
		_Result += '(' + Text + ')';
	}
	
	std::string _Result;
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
	Test("<root attribute=\"value\"></root>", "[+root|attribute=value][-root]");
	Test("<root attribute=\"value\">text</root>", "[+root|attribute=value](text)[-root]");
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
