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
 * This is version 1.7 of the xml parser.
 **/

#include <iostream>

#include "xml_parser.h"

/**
 * Comment stages:
 * - 0  ->  not relevant for comment
 * - 1  ->  when '<' is read in the document scope (also for tags)
 * - 2  ->  when '<!' is read
 * - 3  ->  when '<!-' is read
 * - 4  ->  when '<!--' is read, this is a valid comment opener
 * - 5  ->  when '<!--' .. '-' is read
 * - 6  ->  when '<!--' .. '--' is read, comment closing, '>' expected next, which turns around to stage 0
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
	auto InEntity{false};
	auto InTag{false};
	auto InIdentifier{false};
	auto IsEndTag{false};
	auto IsEmptyElement{false};
	auto CommentStage{0u};
	
	while(_InputStream.get(Char))
	{
		switch(Char)
		{
		case '\n':
		case '\t':
		case ' ':
			{
				if(CommentStage == 0)
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
				else if(CommentStage == 4)
				{
					Buffer += Char;
				}
				else if(CommentStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					CommentStage = 4;
				}
				
				break;
			}
		case '=':
			{
				if(CommentStage == 0)
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
				else if(CommentStage == 4)
				{
					Buffer += Char;
				}
				else if(CommentStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					CommentStage = 4;
				}
				
				break;
			}
		case '"':
			{
				if(CommentStage == 0)
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
				else if(CommentStage == 4)
				{
					Buffer += Char;
				}
				else if(CommentStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					CommentStage = 4;
				}
				
				break;
			}
		case '<':
			{
				if(CommentStage == 0)
				{
					if(Buffer.empty() == false)
					{
						Text(Buffer);
						Buffer.erase();
					}
					CommentStage = 1;
				}
				else if(CommentStage == 4)
				{
					Buffer += Char;
				}
				else if(CommentStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					CommentStage = 4;
				}
				
				break;
			}
		case '>':
			{
				if(CommentStage == 0)
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
				else if(CommentStage == 4)
				{
					Buffer += Char;
				}
				else if(CommentStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					CommentStage = 4;
				}
				else if(CommentStage == 6)
				{
					CommentStage = 0;
					Comment(Buffer);
					Buffer.erase();
				}
				
				break;
			}
		case '/':
			{
				if(CommentStage == 0)
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
				else if(CommentStage == 1)
				{
					IsEndTag = true;
					CommentStage = 0;
				}
				else if(CommentStage == 4)
				{
					Buffer += Char;
				}
				else if(CommentStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					CommentStage = 4;
				}
				
				break;
			}
		case '&':
			{
				if(CommentStage == 0)
				{
					InEntity = true;
				}
				else if(CommentStage == 4)
				{
					Buffer += Char;
				}
				else if(CommentStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					CommentStage = 4;
				}
				
				break;
			}
		case ';':
			{
				if(CommentStage == 0)
				{
					if(InEntity == true)
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
						InEntity = false;
					}
					else
					{
						Buffer += Char;
					}
				}
				else if(CommentStage == 4)
				{
					Buffer += Char;
				}
				else if(CommentStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					CommentStage = 4;
				}
				
				break;
			}
		case '!':
			{
				if(CommentStage == 0)
				{
					Buffer += Char;
				}
				else if(CommentStage == 1)
				{
					CommentStage = 2;
				}
				else if(CommentStage == 4)
				{
					Buffer += Char;
				}
				else if(CommentStage == 5)
				{
					Buffer += '-';
					Buffer += Char;
					CommentStage = 4;
				}
				
				break;
			}
		case '-':
			{
				if(CommentStage == 0)
				{
					Buffer += Char;
				}
				else if(CommentStage == 2)
				{
					CommentStage = 3;
				}
				else if(CommentStage == 3)
				{
					CommentStage = 4;
				}
				else if(CommentStage == 4)
				{
					CommentStage = 5;
				}
				else if(CommentStage == 5)
				{
					CommentStage = 6;
				}
				
				break;
			}
		default:
			{
				if((CommentStage == 0) || (CommentStage == 1))
				{
					if(InAttributeValue == false)
					{
						CommentStage = 0;
						InTag = true;
						InIdentifier = true;
					}
					if(InEntity == true)
					{
						Entity += Char;
					}
					else
					{
						Buffer += Char;
					}
				}
				else if(CommentStage == 4)
				{
					Buffer += Char;
				}
				else if(CommentStage == 5)
				{
					Buffer += "-";
					Buffer += Char;
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
