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

/**
 * Parsing stages:
 * - 0  ->  not relevant for comments or entities
 * - 1  ->  when '<' is read in the document scope (also for tags)
 * - 2  ->  when '<!' is read
 * - 3  ->  when '<!-' is read
 * - 4  ->  when '<!--' is read, this is a valid comment opener
 * - 5  ->  when '<!--' .. '-' is read
 * - 6  ->  when '<!--' .. '--' is read, comment closing, '>' expected next, which turns around to stage 0
 * - 7  ->  when '&' is read in a place where entities are valid
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
	auto InTag{false};
	auto InIdentifier{false};
	auto IsEndTag{false};
	auto IsEmptyElement{false};
	auto ParsingStage{0u};
	
	while(_InputStream.get(Char))
	{
		switch(Char)
		{
		case '\n':
		case '\t':
		case ' ':
			{
				if(ParsingStage == 0)
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
		case '=':
			{
				if(ParsingStage == 0)
				{
					if(InIdentifier == true)
					{
						AttributeName = Buffer;
						InIdentifier = false;
						Buffer.erase();
					}
					else
					{
						Buffer += Char;
					}
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
		case '"':
			{
				if(ParsingStage == 0)
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
				if(ParsingStage == 0)
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
				else if(ParsingStage == 6)
				{
					ParsingStage = 0;
					Comment(Buffer);
					Buffer.erase();
				}
				
				break;
			}
		case '/':
			{
				if(ParsingStage == 0)
				{
					if((InTag == true) && (InAttributeValue == false))
					{
						if(InIdentifier == true)
						{
							TagName = Buffer;
							Buffer.erase();
							InIdentifier = false;
						}
						IsEmptyElement = true;
					}
					else
					{
						Buffer += Char;
					}
				}
				else if(ParsingStage == 1)
				{
					IsEndTag = true;
					ParsingStage = 0;
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
					Entity = "";
					ParsingStage = 0;
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
				
				break;
			}
		default:
			{
				if((ParsingStage == 0) || (ParsingStage == 1))
				{
					if(InAttributeValue == false)
					{
						ParsingStage = 0;
						InTag = true;
						InIdentifier = true;
					}
					Buffer += Char;
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
