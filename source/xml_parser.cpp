/**
 * xml_parser
 * Copyright (C) 2021-2026  Hagen Möbius
 * SPDX-License-Identifier: MIT
**/

#include <cassert>
#include <iostream>
#include <optional>
#include <vector>

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
 * - 9  ->  when the closing tag identifier has been finalised by a whitespace
 * - 10 ->  when '/' is read in a tag but not after the opening '<', a self-closing tag
 * - 11 ->  when '</' is read, a closing tag
 * - 12 ->  when inside an opening or self-closing tag identifier
 * - 13 ->  when inside an attribute identifier in an opening or self-closing tag
 * - 14 ->  when inside a closing tag identifier
 * - 15 ->  when the tag identifier of an opening or self-closing tag is has been finalised by a whitespace
 * - 16 ->  when a whitespace is read after an attribute identifier
 * - 17 ->  when '=' is read after an attribute identifier
 * - 18 ->  when a whitespace is read after a '=' for an attribute
 * - 19 ->  when inside an attribute value with double quotes
 * - 20 ->  when '&' is read in an attribute value with double quotes
 * - 21 ->  when inside an attribute value with single quotes
 * - 22 ->  when '&' is read in an attribute value with single quotes
 **/

XML::Parser::Parser(std::istream & InputStream) :
    m_InputStream(InputStream)
{
}

auto XML::Parser::Parse() -> std::expected<void, XML::Error>
{
    auto Attributes = std::map<std::string, std::string>{};
    auto AttributeName = std::string{};
    auto AttributeValue = std::string{};
    auto Comment = std::string{};
    auto ElementNameStack = std::vector<std::string>{};
    auto TagName = std::string{};
    auto Text = std::string{};
    auto Entity = std::string{};
    auto Character = '\0';
    auto ParsingStage = 0u;
    auto CurrentLocation = XML::Location{0, 0};
    auto StartLocation = std::optional<XML::Location>{};
    auto Error = std::optional<XML::Error>{};
    
    while((Error.has_value() == false) && (m_InputStream.get(Character)))
    {
        //~ std::cout << std::boolalpha << "In stage " << ParsingStage << " got '"  << Character << "'. (Comment=\"" << Comment << "\"; TagName=\"" << TagName << "\"; Text=\"" << Text << "\"; AttributeName=\"" << AttributeName << "\"; AttributeValue=\"" << AttributeValue << "\"; Entity=\"" << Entity << "\"; CurrentLocation.Line=\"" << CurrentLocation.Line << "\"; CurrentLocation.Column=\"" << CurrentLocation.Column << "\"; ";
        //~ if(StartLocation.has_value() == true)
        //~ {
            //~ std::cout << "StartLocation.Line=\"" << StartLocation->Line << "\"; StartLocation.Column=\"" << StartLocation->Column << "\"";
        //~ }
        //~ else
        //~ {
            //~ std::cout << "StartLocation=none";
        //~ }
        //~ std::cout << ')' << std::endl;
        switch(Character)
        {
        case '\n':
        case '\t':
        case ' ':
            {
                if(ParsingStage == 0)
                {
                    if(StartLocation.has_value() == false)
                    {
                        StartLocation = CurrentLocation;
                    }
                    Text += Character;
                }
                else if(ParsingStage == 1)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 4)
                {
                    Comment += Character;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Character;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 6)
                {
                    Comment += "--";
                    Comment += Character;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 10)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 11)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 12)
                {
                    ParsingStage = 15;
                }
                else if(ParsingStage == 13)
                {
                    ParsingStage = 16;
                }
                else if(ParsingStage == 14)
                {
                    ParsingStage = 9;
                }
                else if(ParsingStage == 17)
                {
                    ParsingStage = 18;
                }
                else if(ParsingStage == 19)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 20)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 21)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 22)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                
                break;
            }
        case '=':
            {
                if(ParsingStage == 0)
                {
                    if(StartLocation.has_value() == false)
                    {
                        StartLocation = CurrentLocation;
                    }
                    Text += Character;
                }
                else if(ParsingStage == 1)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 4)
                {
                    Comment += Character;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Character;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 9)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 10)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 11)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 12)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 13)
                {
                    ParsingStage = 17;
                }
                else if(ParsingStage == 14)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 15)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 16)
                {
                    ParsingStage = 17;
                }
                else if(ParsingStage == 17)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 19)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 20)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 21)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 22)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                
                break;
            }
        case '"':
            {
                if(ParsingStage == 0)
                {
                    if(StartLocation.has_value() == false)
                    {
                        StartLocation = CurrentLocation;
                    }
                    Text += Character;
                }
                else if(ParsingStage == 1)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 4)
                {
                    Comment += Character;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Character;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 9)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 10)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 11)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 12)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 13)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 14)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 15)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 16)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
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
                    ParsingStage = 15;
                }
                else if(ParsingStage == 20)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 21)
                {
                    AttributeValue += Character;
                }
                
                break;
            }
        case '\'':
            {
                if(ParsingStage == 0)
                {
                    if(StartLocation.has_value() == false)
                    {
                        StartLocation = CurrentLocation;
                    }
                    Text += Character;
                }
                else if(ParsingStage == 1)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 4)
                {
                    Comment += Character;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Character;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 9)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 10)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 11)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 12)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 13)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 14)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 15)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 16)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 17)
                {
                    ParsingStage = 21;
                }
                else if(ParsingStage == 18)
                {
                    ParsingStage = 21;
                }
                else if(ParsingStage == 19)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 20)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 21)
                {
                    Attributes[AttributeName] = AttributeValue;
                    AttributeName.erase();
                    AttributeValue.erase();
                    ParsingStage = 15;
                }
                else if(ParsingStage == 22)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                
                break;
            }
        case '<':
            {
                if(ParsingStage == 0)
                {
                    if(Text.empty() == false)
                    {
                        assert(StartLocation.has_value() == true);
                        this->Text(Text, StartLocation.value());
                        Text.erase();
                    }
                    StartLocation = CurrentLocation;
                    ParsingStage = 1;
                }
                else if(ParsingStage == 1)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 4)
                {
                    Comment += Character;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Character;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 9)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 10)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 11)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 12)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 13)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 14)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 15)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 16)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 17)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 18)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 19)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 20)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 21)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 22)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                
                break;
            }
        case '>':
            {
                if(ParsingStage == 0)
                {
                    if(StartLocation.has_value() == false)
                    {
                        StartLocation = CurrentLocation;
                    }
                    Text += Character;
                }
                else if(ParsingStage == 1)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 4)
                {
                    Comment += Character;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Character;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 6)
                {
                    assert(StartLocation.has_value() == true);
                    this->Comment(Comment, StartLocation.value());
                    Comment.erase();
                    StartLocation.reset();
                    ParsingStage = 0;
                }
                else if(ParsingStage == 9)
                {
                    if(ElementNameStack.empty() == false)
                    {
                        auto StartedElementName = ElementNameStack.back();
                        
                        if(StartedElementName == TagName)
                        {
                            ElementNameStack.pop_back();
                            ElementEnd(TagName);
                            TagName.erase();
                            StartLocation.reset();
                            ParsingStage = 0;
                        }
                        else
                        {
                            assert(StartLocation.has_value() == true);
                            Error.emplace(XML::ErrorCode::InvalidClosingElementName, StartLocation.value());
                        }
                    }
                    else
                    {
                        assert(StartLocation.has_value() == true);
                        Error.emplace(XML::ErrorCode::InvalidClosingElementName, StartLocation.value());
                    }
                }
                else if(ParsingStage == 10)
                {
                    assert(StartLocation.has_value() == true);
                    ElementStart(TagName, Attributes, StartLocation.value());
                    ElementEnd(TagName);
                    TagName.erase();
                    Attributes.clear();
                    StartLocation.reset();
                    ParsingStage = 0;
                }
                else if(ParsingStage == 11)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 12)
                {
                    assert(StartLocation.has_value() == true);
                    ElementNameStack.push_back(TagName);
                    ElementStart(TagName, Attributes, StartLocation.value());
                    TagName.erase();
                    Attributes.clear();
                    StartLocation.reset();
                    ParsingStage = 0;
                }
                else if(ParsingStage == 13)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 14)
                {
                    if(ElementNameStack.empty() == false)
                    {
                        auto StartedElementName = ElementNameStack.back();
                        
                        if(StartedElementName == TagName)
                        {
                            ElementNameStack.pop_back();
                            ElementEnd(TagName);
                            TagName.erase();
                            StartLocation.reset();
                            ParsingStage = 0;
                        }
                        else
                        {
                            assert(StartLocation.has_value() == true);
                            Error.emplace(XML::ErrorCode::InvalidClosingElementName, StartLocation.value());
                        }
                    }
                    else
                    {
                        assert(StartLocation.has_value() == true);
                        Error.emplace(XML::ErrorCode::InvalidClosingElementName, StartLocation.value());
                    }
                }
                else if(ParsingStage == 15)
                {
                    assert(StartLocation.has_value() == true);
                    ElementNameStack.push_back(TagName);
                    ElementStart(TagName, Attributes, StartLocation.value());
                    TagName.erase();
                    Attributes.clear();
                    StartLocation.reset();
                    ParsingStage = 0;
                }
                else if(ParsingStage == 16)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 17)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 18)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 19)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 20)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 21)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 22)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                
                break;
            }
        case '/':
            {
                if(ParsingStage == 0)
                {
                    if(StartLocation.has_value() == false)
                    {
                        StartLocation = CurrentLocation;
                    }
                    Text += Character;
                }
                else if(ParsingStage == 1)
                {
                    ParsingStage = 11;
                }
                else if(ParsingStage == 4)
                {
                    Comment += Character;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Character;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 9)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 10)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 11)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 12)
                {
                    ParsingStage = 10;
                }
                else if(ParsingStage == 13)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 14)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 15)
                {
                    ParsingStage = 10;
                }
                else if(ParsingStage == 16)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 16)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 17)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 18)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 19)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 21)
                {
                    AttributeValue += Character;
                }
                
                break;
            }
        case '&':
            {
                if(ParsingStage == 0)
                {
                    if(StartLocation.has_value() == false)
                    {
                        StartLocation = CurrentLocation;
                    }
                    ParsingStage = 7;
                }
                else if(ParsingStage == 1)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 4)
                {
                    Comment += Character;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Character;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 9)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 10)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 11)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 12)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 13)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 14)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 15)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 16)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 17)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 18)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 19)
                {
                    ParsingStage = 20;
                }
                else if(ParsingStage == 20)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 21)
                {
                    ParsingStage = 22;
                }
                else if(ParsingStage == 22)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                
                break;
            }
        case ';':
            {
                if(ParsingStage == 0)
                {
                    if(StartLocation.has_value() == false)
                    {
                        StartLocation = CurrentLocation;
                    }
                    Text += Character;
                }
                else if(ParsingStage == 1)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 4)
                {
                    Comment += Character;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Character;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 7)
                {
                    ForwardEntityTo(Entity, Text);
                    ParsingStage = 0;
                }
                else if(ParsingStage == 9)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 11)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 12)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 13)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 14)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 15)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 16)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 17)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 18)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 19)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 20)
                {
                    if(Entity.empty() == true)
                    {
                        Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                    }
                    else
                    {
                        ForwardEntityTo(Entity, AttributeValue);
                        ParsingStage = 19;
                    }
                }
                else if(ParsingStage == 21)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 22)
                {
                    if(Entity.empty() == true)
                    {
                        Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                    }
                    else
                    {
                        ForwardEntityTo(Entity, AttributeValue);
                        ParsingStage = 21;
                    }
                }
                
                break;
            }
        case '!':
            {
                if(ParsingStage == 0)
                {
                    if(StartLocation.has_value() == false)
                    {
                        StartLocation = CurrentLocation;
                    }
                    Text += Character;
                }
                else if(ParsingStage == 1)
                {
                    ParsingStage = 2;
                }
                else if(ParsingStage == 4)
                {
                    Comment += Character;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Character;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 9)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 10)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 11)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 12)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 13)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 14)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 15)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 16)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 17)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 18)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 19)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 20)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 21)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 22)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                
                break;
            }
        case '-':
            {
                if(ParsingStage == 0)
                {
                    if(StartLocation.has_value() == false)
                    {
                        StartLocation = CurrentLocation;
                    }
                    Text += Character;
                }
                else if(ParsingStage == 1)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
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
                else if(ParsingStage == 9)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 10)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 11)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 12)
                {
                    TagName += Character;
                }
                else if(ParsingStage == 13)
                {
                    AttributeName += Character;
                }
                else if(ParsingStage == 14)
                {
                    TagName += Character;
                }
                else if(ParsingStage == 15)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 19)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 20)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 21)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 22)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                
                break;
            }
        default:
            {
                if(ParsingStage == 0)
                {
                    if(StartLocation.has_value() == false)
                    {
                        StartLocation = CurrentLocation;
                    }
                    Text += Character;
                }
                else if(ParsingStage == 1)
                {
                    TagName += Character;
                    ParsingStage = 12;
                }
                else if(ParsingStage == 2)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 4)
                {
                    Comment += Character;
                }
                else if(ParsingStage == 5)
                {
                    Comment += '-';
                    Comment += Character;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 6)
                {
                    Comment += "--";
                    Comment += Character;
                    ParsingStage = 4;
                }
                else if(ParsingStage == 7)
                {
                    Entity += Character;
                }
                else if(ParsingStage == 9)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 11)
                {
                    TagName += Character;
                    ParsingStage = 14;
                }
                else if(ParsingStage == 12)
                {
                    TagName += Character;
                }
                else if(ParsingStage == 13)
                {
                    AttributeName += Character;
                }
                else if(ParsingStage == 14)
                {
                    TagName += Character;
                }
                else if(ParsingStage == 15)
                {
                    AttributeName += Character;
                    ParsingStage = 13;
                }
                else if(ParsingStage == 16)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 17)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 18)
                {
                    Error.emplace(XML::ErrorCode::InvalidCharacter, CurrentLocation);
                }
                else if(ParsingStage == 19)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 20)
                {
                    Entity += Character;
                }
                else if(ParsingStage == 21)
                {
                    AttributeValue += Character;
                }
                else if(ParsingStage == 22)
                {
                    Entity += Character;
                }
                
                break;
            }
        }
        if(Character == '\n')
        {
            CurrentLocation.Column = 0;
            CurrentLocation.Line += 1;
        }
        else
        {
            CurrentLocation.Column += 1;
        }
    }
    if(Error.has_value() == true)
    {
        return std::unexpected{Error.value()};
    }
    else
    {
        return {};
    }
}

auto XML::Parser::Comment(std::string const &, XML::Location const &) -> void
{
}

auto XML::Parser::ElementStart(std::string const &, std::map<std::string, std::string> const &, XML::Location const &) -> void
{
}

auto XML::Parser::ElementEnd(std::string const &) -> void
{
}

auto XML::Parser::Text(std::string const &, XML::Location const &) -> void
{
}
