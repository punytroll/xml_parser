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

#include <iostream>

#include <xml_parser/xml_parser.h>

auto Forward(std::string && From, std::string & To) -> void
{
    To += From;
    From.erase();
}

auto ForwardEntityTo(std::string & Entity, std::string & To) -> void
{
    if(Entity == "amp")
    {
        Forward("&", To);
    }
    else if(Entity == "gt")
    {
        Forward(">", To);
    }
    else if(Entity == "lt")
    {
        Forward("<", To);
    }
    else if(Entity == "apos")
    {
        Forward("'", To);
    }
    else if(Entity == "quot")
    {
        Forward("\"", To);
    }
    else
    {
        Forward("&" + Entity + ";", To);
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
 * - 16 ->  when a whitespace is read after an attribute identifier
 * - 17 ->  when '=' is read after an attribute identifier
 * - 18 ->  when a whitespace is read after a '=' for an attribute
 * - 19 ->  when inside an attribute value
 **/

XMLParser::XMLParser(std::istream & InputStream) :
    m_InputStream(InputStream)
{
}

auto XMLParser::Parse() -> void
{
    auto Attributes = std::map<std::string, std::string>{};
    auto AttributeName = std::string{};
    auto AttributeValue = std::string{};
    auto Comment = std::string{};
    auto TagName = std::string{};
    auto Text = std::string{};
    auto Entity = std::string{};
    auto Char = '\0';
    auto ParsingStage = 0u;
    
    while(m_InputStream.get(Char))
    {
        //~ std::cout << std::boolalpha << "In stage " << ParsingStage << " got '"  << Char << "'. (Comment=\"" << Comment << "\"; TagName=\"" << TagName << "\"; Text=\"" << Text << "\"; AttributeName=\"" << AttributeName << "\"; AttributeValue=\"" << AttributeValue << "\"; Entity=\"" << Entity << "\")" << std::endl;
        switch(Char)
        {
        case '\n':
        case '\t':
        case ' ':
            {
                if(ParsingStage == 0)
                {
                    Text += Char;
                }
                else if(ParsingStage == 4)
                {
                    Comment += Char;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Char;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 6)
                {
                    Comment += "--";
                    Comment += Char;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 12)
                {
                    ParsingStage = 15;
                }
                else if(ParsingStage == 13)
                {
                    ParsingStage = 16;
                }
                else if(ParsingStage == 17)
                {
                    ParsingStage = 18;
                }
                else if(ParsingStage == 19)
                {
                    AttributeValue += Char;
                }
                
                break;
            }
        case '=':
            {
                if(ParsingStage == 0)
                {
                    Text += Char;
                }
                else if(ParsingStage == 4)
                {
                    Comment += Char;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Char;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 13)
                {
                    ParsingStage = 17;
                }
                else if(ParsingStage == 16)
                {
                    ParsingStage = 17;
                }
                
                break;
            }
        case '"':
            {
                if(ParsingStage == 0)
                {
                    Text += Char;
                }
                else if(ParsingStage == 4)
                {
                    Comment += Char;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Char;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 17)
                {
                    ParsingStage = 19;
                }
                else if(ParsingStage == 18)
                {
                    ParsingStage = 19;
                }
                else if(ParsingStage == 19)
                {
                    Attributes[AttributeName] = AttributeValue;
                    AttributeName.erase();
                    AttributeValue.erase();
                    ParsingStage = 8;
                }
                
                break;
            }
        case '<':
            {
                if(ParsingStage == 0)
                {
                    if(Text.empty() == false)
                    {
                        this->Text(Text);
                        Text.erase();
                    }
                    ParsingStage = 1;
                }
                else if(ParsingStage == 4)
                {
                    Comment += Char;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Char;
                    ParsingStage = 4;
                }
                
                break;
            }
        case '>':
            {
                if(ParsingStage == 0)
                {
                    Text += Char;
                }
                else if(ParsingStage == 4)
                {
                    Comment += Char;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Char;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 6)
                {
                    ParsingStage = 0;
                    this->Comment(Comment);
                    Comment.erase();
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
                    Text += Char;
                }
                else if(ParsingStage == 1)
                {
                    ParsingStage = 11;
                }
                else if(ParsingStage == 4)
                {
                    Comment += Char;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Char;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 8)
                {
                    ParsingStage = 10;
                }
                else if(ParsingStage == 12)
                {
                    ParsingStage = 10;
                }
                else if(ParsingStage == 15)
                {
                    ParsingStage = 10;
                }
                else if(ParsingStage == 19)
                {
                    AttributeValue += Char;
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
                    Comment += Char;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Char;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 19)
                {
                    ParsingStage = 9;
                }
                
                break;
            }
        case ';':
            {
                if(ParsingStage == 0)
                {
                    Text += Char;
                }
                else if(ParsingStage == 4)
                {
                    Comment += Char;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Char;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 7)
                {
                    ForwardEntityTo(Entity, Text);
                    ParsingStage = 0;
                }
                else if(ParsingStage == 9)
                {
                    ForwardEntityTo(Entity, AttributeValue);
                    ParsingStage = 19;
                }
                
                break;
            }
        case '!':
            {
                if(ParsingStage == 0)
                {
                    Text += Char;
                }
                else if(ParsingStage == 1)
                {
                    ParsingStage = 2;
                }
                else if(ParsingStage == 4)
                {
                    Comment += Char;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Char;
                    ParsingStage = 4;
                }
                
                break;
            }
        case '-':
            {
                if(ParsingStage == 0)
                {
                    Text += Char;
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
                else if(ParsingStage == 6)
                {
                    Comment += '-';
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
                else if(ParsingStage == 19)
                {
                    AttributeValue += Char;
                }
                
                break;
            }
        default:
            {
                if(ParsingStage == 0)
                {
                    Text += Char;
                }
                else if(ParsingStage == 1)
                {
                    TagName += Char;
                    ParsingStage = 12;
                }
                else if(ParsingStage == 4)
                {
                    Comment += Char;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Char;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 6)
                {
                    Comment += "--";
                    Comment += Char;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 7)
                {
                    Entity += Char;
                }
                else if(ParsingStage == 8)
                {
                    AttributeName += Char;
                    ParsingStage = 13;
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
                else if(ParsingStage == 19)
                {
                    AttributeValue += Char;
                }
                
                break;
            }
        }
    }
}

auto XMLParser::Comment(std::string const &) -> void
{
}

auto XMLParser::ElementStart(std::string const &, std::map<std::string, std::string> const &) -> void
{
}

auto XMLParser::ElementEnd(std::string const &) -> void
{
}

auto XMLParser::Text(std::string const &) -> void
{
}
