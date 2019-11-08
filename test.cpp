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
 * This is version 1.6 of the xml parser.
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
	virtual void ElementStart(const std::string & TagName, const std::map< std::string, std::string > & Attributes)
	{
		_Result += "[+" + TagName;
		for(std::map< std::string, std::string >::const_iterator Iterator = Attributes.begin(); Iterator != Attributes.end(); ++Iterator)
		{
			_Result += '|' + Iterator->first + '=' + Iterator->second;
		}
		_Result += ']';
	}
	
	virtual void ElementEnd(const std::string & TagName)
	{
		_Result += "[-" + TagName + ']';
	}
	
	virtual void Text(const std::string & Text)
	{
		_Result += '(' + Text + ')';
	}
	
	std::string _Result;
};

int main(int argc, char ** argv)
{
	{
		std::stringstream XMLStream{"<r><h a=\"v\">H<s t=\"f\"/> ! </h></r>"};
		TestParser TestParser{XMLStream};
		
		TestParser.Parse();
		assert(TestParser.GetResult() == "[+r][+h|a=v](H)[+s|t=f][-s]( ! )[-h][-r]");
	}
	{
		std::stringstream XMLStream{
"<call function=\"insert\">\
	<parameter name=\"identifier\">127.0.0.1</parameter>\
	<parameter name=\"title\">localhost</parameter>\
</call>"};
		TestParser TestParser{XMLStream};
		
		TestParser.Parse();
		assert(TestParser.GetResult() == "[+call|function=insert](\t)[+parameter|name=identifier](127.0.0.1)[-parameter](\t)[+parameter|name=title](localhost)[-parameter][-call]");
	}
	{
		std::stringstream XMLStream{"<root>hallo</root>"};
		TestParser TestParser{XMLStream};
		
		TestParser.Parse();
		assert(TestParser.GetResult() == "[+root](hallo)[-root]");
	}
	{
		std::stringstream XMLStream{"<root>&amp;</root>"};
		TestParser TestParser{XMLStream};
		
		TestParser.Parse();
		assert(TestParser.GetResult() == "[+root](&)[-root]");
	}
	{
		std::stringstream XMLStream{"<root>&lt;</root>"};
		TestParser TestParser{XMLStream};
		
		TestParser.Parse();
		assert(TestParser.GetResult() == "[+root](<)[-root]");
	}
	{
		std::stringstream XMLStream{"<root>&gt;</root>"};
		TestParser TestParser{XMLStream};
		
		TestParser.Parse();
		assert(TestParser.GetResult() == "[+root](>)[-root]");
	}
	{
		std::stringstream XMLStream{"<root att=\"h&amp;m\"/>"};
		TestParser TestParser{XMLStream};
		
		TestParser.Parse();
		assert(TestParser.GetResult() == "[+root|att=h&m][-root]");
	}
	{
		std::stringstream XMLStream{"<root att=\"h&quot;m\"/>"};
		TestParser TestParser{XMLStream};
		
		TestParser.Parse();
		assert(TestParser.GetResult() == "[+root|att=h\"m][-root]");
	}
	{
		std::stringstream XMLStream{"<root att=\"h&apos;m\"/>"};
		TestParser TestParser{XMLStream};
		
		TestParser.Parse();
		assert(TestParser.GetResult() == "[+root|att=h'm][-root]");
	}
	{
		std::stringstream XMLStream{"<root att=\"h&apos;&amp;m\"/>"};
		TestParser TestParser{XMLStream};
		
		TestParser.Parse();
		assert(TestParser.GetResult() == "[+root|att=h'&m][-root]");
	}
}
