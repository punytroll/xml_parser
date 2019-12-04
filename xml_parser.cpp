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
 * This is version 1.7.1 of the xml parser.
 **/

#include <iostream>

#include "xml_parser.h"

void _ForwardEntityToBuffer(std::string & Entity, std::string & Buffer)
{
	if(Entity == "amp")
	{
		Buffer += '&';
	}
	else if(Entity == "gt")
	{
		Buffer += '>';
	}
	else if(Entity == "lt")
	{
		Buffer += '<';
	}
	else if(Entity == "apos")
	{
		Buffer += '\'';
	}
	else if(Entity == "quot")
	{
		Buffer += '"';
	}
	else
	{
		Buffer += '&';
		Buffer += Entity;
		Buffer += ';';
	}
	Entity.erase();
}

/**
 * Parsing stages:
 * - 0  ->  document scope
 * - 1  ->  when '<' is read in the document scope
 * - 2  ->  when '<!' is read, preparing to open a comment
 * - 3  ->  when '<!-' is read, preparing to open a comment
 * - 4  ->  when '<!--' is read, comment opened
 * - 5  ->  when '<!--' .. '-' is read, preparing to close a comment
 * - 6  ->  when '<!--' .. '--' is read, comment closing, '>' expected next, which turns around to stage 0
 * - 7  ->  when '&' is read in the document scope
 * - 8  ->  when '<' was read but not continued by '!', this is an opening or self-closing tag
 * - 9  ->  when '&' is read in an attribute value
 * - 10 ->  when '/' is read in a tag but not after the opening '<', a self-closing tag
 * - 11 ->  when '</' is read, a closing tag
 * - 12 ->  when inside an opening or self-closing tag identifier
 * - 13 ->  when inside an attribute identifier in an opening or self-closing tag
 * - 14 ->  when inside a closing tag identifier
 * - 15 ->  when the tag identifier of an opening or self-closing tag is done
 **/

XMLParser::XMLParser(std::istream & InputStream) :
	_InputStream(InputStream)
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
	std::string Entity;
	char Char;
	auto InAttributeValue{false};
	auto ParsingStage{0u};
	
	while(_InputStream.get(Char))
	{
		//~ std::cout << std::boolalpha << "Got '"  << Char << "' at " << ParsingStage << ". (InAttributeValue=" << InAttributeValue << "; TagName=\"" << TagName << "\"; AttributeName=\"" << AttributeName << "\"; Entity=\"" << Entity << "\"; Buffer=\"" << Buffer << "\")" << std::endl;
		switch(Char)
		{
		case '\n':
		case '\t':
		case ' ':
			{
				if(ParsingStage == 0)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 4)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					ParsingStage = 4;
				}
				else if(ParsingStage == 8)
				{
					if(InAttributeValue == true)
					{
						Buffer += Char;
					}
				}
				else if(ParsingStage == 11)
				{
					TagName = Buffer;
					Buffer.erase();
				}
				else if(ParsingStage == 12)
				{
					ParsingStage = 15;
				}
				else if(ParsingStage == 13)
				{
					AttributeName = Buffer;
					ParsingStage = 8;
				}
				
				break;
			}
		case '=':
			{
				if(ParsingStage == 0)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 4)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					ParsingStage = 4;
				}
				else if(ParsingStage == 8)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 13)
				{
					ParsingStage = 8;
				}
				
				break;
			}
		case '"':
			{
				if(ParsingStage == 0)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 4)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					ParsingStage = 4;
				}
				else if(ParsingStage == 8)
				{
					if(InAttributeValue == false)
					{
						InAttributeValue = true;
					}
					else
					{
						InAttributeValue = false;
						Attributes[AttributeName] = Buffer;
						AttributeName.erase();
						Buffer.erase();
					}
				}
				
				break;
			}
		case '<':
			{
				if(ParsingStage == 0)
				{
					if(Buffer.empty() == false)
					{
						Text(Buffer);
						Buffer.erase();
					}
					ParsingStage = 1;
				}
				else if(ParsingStage == 4)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					ParsingStage = 4;
				}
				
				break;
			}
		case '>':
			{
				if(ParsingStage == 4)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					ParsingStage = 4;
				}
				else if(ParsingStage == 6)
				{
					ParsingStage = 0;
					Comment(Buffer);
					Buffer.erase();
				}
				else if(ParsingStage == 8)
				{
					ElementStart(TagName, Attributes);
					TagName.erase();
					Attributes.clear();
					ParsingStage = 0;
				}
				else if(ParsingStage == 10)
				{
					ElementStart(TagName, Attributes);
					ElementEnd(TagName);
					TagName.erase();
					Attributes.clear();
					ParsingStage = 0;
				}
				else if(ParsingStage == 11)
				{
					ElementEnd(TagName);
					TagName.erase();
					ParsingStage = 0;
				}
				else if(ParsingStage == 12)
				{
					ElementStart(TagName, Attributes);
					TagName.erase();
					Attributes.clear();
					ParsingStage = 0;
				}
				else if(ParsingStage == 14)
				{
					ElementEnd(TagName);
					TagName.erase();
					ParsingStage = 0;
				}
				
				break;
			}
		case '/':
			{
				if(ParsingStage == 0)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 1)
				{
					ParsingStage = 11;
				}
				else if(ParsingStage == 4)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					ParsingStage = 4;
				}
				else if(ParsingStage == 8)
				{
					if(InAttributeValue == false)
					{
						ParsingStage = 10;
					}
					else
					{
						Buffer += Char;
					}
				}
				else if(ParsingStage == 12)
				{
					ParsingStage = 10;
				}
				else if(ParsingStage == 14)
				{
					TagName = Buffer;
					Buffer.erase();
				}
				else if(ParsingStage == 15)
				{
					ParsingStage = 10;
				}
				
				break;
			}
		case '&':
			{
				if(ParsingStage == 0)
				{
					ParsingStage = 7;
				}
				else if(ParsingStage == 4)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					ParsingStage = 4;
				}
				else if(ParsingStage == 8)
				{
					ParsingStage = 9;
				}
				
				break;
			}
		case ';':
			{
				if(ParsingStage == 0)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 4)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					ParsingStage = 4;
				}
				else if(ParsingStage == 7)
				{
					_ForwardEntityToBuffer(Entity, Buffer);
					ParsingStage = 0;
				}
				else if(ParsingStage == 9)
				{
					_ForwardEntityToBuffer(Entity, Buffer);
					ParsingStage = 8;
				}
				
				break;
			}
		case '!':
			{
				if(ParsingStage == 0)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 1)
				{
					ParsingStage = 2;
				}
				else if(ParsingStage == 4)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					ParsingStage = 4;
				}
				
				break;
			}
		case '-':
			{
				if(ParsingStage == 0)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 2)
				{
					ParsingStage = 3;
				}
				else if(ParsingStage == 3)
				{
					ParsingStage = 4;
				}
				else if(ParsingStage == 4)
				{
					ParsingStage = 5;
				}
				else if(ParsingStage == 5)
				{
					ParsingStage = 6;
				}
				else if(ParsingStage == 8)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 11)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 12)
				{
					TagName += Char;
				}
				else if(ParsingStage == 13)
				{
					AttributeName += Char;
				}
				else if(ParsingStage == 14)
				{
					TagName += Char;
				}
				
				break;
			}
		default:
			{
				if(ParsingStage == 0)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 1)
				{
					TagName += Char;
					ParsingStage = 12;
				}
				else if(ParsingStage == 4)
				{
					Buffer += Char;
				}
				else if(ParsingStage == 5)
				{
					Buffer += "-";
					Buffer += Char;
				}
				else if(ParsingStage == 7)
				{
					Entity += Char;
				}
				else if(ParsingStage == 8)
				{
					if(InAttributeValue == false)
					{
						ParsingStage = 12;
					}
					Buffer += Char;
				}
				else if(ParsingStage == 9)
				{
					Entity += Char;
				}
				else if(ParsingStage == 11)
				{
					TagName += Char;
					ParsingStage = 14;
				}
				else if(ParsingStage == 12)
				{
					TagName += Char;
				}
				else if(ParsingStage == 13)
				{
					AttributeName += Char;
				}
				else if(ParsingStage == 14)
				{
					TagName += Char;
				}
				else if(ParsingStage == 15)
				{
					AttributeName += Char;
					ParsingStage = 13;
				}
				
				break;
			}
		}
	}
}

void XMLParser::Comment(const std::string & Comment)
{
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
