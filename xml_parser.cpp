/**
 * Copyright (C) 2006, 2007  Hagen Möbius
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
 * This is version 1.5.1 of the xml parser.
 **/

#include <iostream>

#include "xml_parser.h"

XMLParser::XMLParser(std::istream & InputStream) :
	m_InputStream(InputStream)
{
}

XMLParser::~XMLParser(void)
{
}

void XMLParser::Parse(void)
{
	std::map< std::string, std::string > Attributes;
	std::string TagName;
	std::string AttributeName;
	std::string Buffer;
	char Char;
	bool InAttributeValue(false);
	bool InTag(false);
	bool InIdentifier(false);
	bool IsEndTag(false);
	bool IsEmptyElement(false);
	
	while(m_InputStream.get(Char))
	{
		switch(Char)
		{
		case '\n':
		case '\t':
		case ' ':
			{
				if((InTag == false) || (InAttributeValue == true))
				{
					Buffer += Char;
				}
				else
				{
					if(InIdentifier == true)
					{
						InIdentifier = false;
						if(TagName.length() == 0)
						{
							TagName = Buffer;
						}
						else
						{
							AttributeName = Buffer;
						}
						Buffer.erase();
					}
				}
				
				break;
			}
		case '=':
			{
				if(InIdentifier == true)
				{
					AttributeName = Buffer;
					InIdentifier = false;
					Buffer.erase();
				}
				
				break;
			}
		case '"':
			{
				if(InAttributeValue == false)
				{
					InAttributeValue = true;
				}
				else
				{
					
					InAttributeValue = false;
					Attributes[AttributeName] = Buffer;
					Buffer.erase();
				}
				
				break;
			}
		case '<':
			{
				if(Buffer.empty() == false)
				{
					Text(Buffer);
					Buffer.erase();
				}
				InTag = true;
				
				break;
			}
		case '>':
			{
				if(InIdentifier == true)
				{
					TagName = Buffer;
					InIdentifier = false;
					Buffer.erase();
				}
				if(IsEndTag == true)
				{
					ElementEnd(TagName);
					TagName.erase();
					IsEndTag = false;
				}
				else
				{
					ElementStart(TagName, Attributes);
					if(IsEmptyElement == true)
					{
						ElementEnd(TagName);
						IsEmptyElement = false;
					}
					TagName.erase();
					Attributes.clear();
				}
				InTag = false;
				
				break;
			}
		case '/':
			{
				if((InTag == true) && (InAttributeValue == false))
				{
					if(InIdentifier == true)
					{
						TagName = Buffer;
						Buffer.erase();
						InIdentifier = false;
					}
					if(TagName.length() == 0)
					{
						IsEndTag = true;
					}
					else
					{
						IsEmptyElement = true;
					}
				}
				else
				{
					Buffer += '/';
				}
				
				break;
			}
		default:
			{
				if((InTag == true) && (InAttributeValue == false))
				{
					InIdentifier = true;
				}
				Buffer += Char;
				
				break;
			}
		}
	}
}

void XMLParser::ElementStart(const std::string & TagName, const std::map< std::string, std::string > & Attributes)
{
}

void XMLParser::ElementEnd(const std::string & TagName)
{
}

void XMLParser::Text(const std::string & Text)
{
}
