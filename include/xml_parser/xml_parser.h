/**
 * xml_parser
 * Copyright (C) 2021-2026  Hagen Möbius
 * SPDX-License-Identifier: MIT
**/

#ifndef XML_PARSER__XML_PARSER_H
#define XML_PARSER__XML_PARSER_H

#include <cstdint>
#include <expected>
#include <istream>
#include <map>

namespace XML
{
    class Location
    {
    public:
        Location(std::uint64_t CreateWithLine, std::uint64_t CreateWithColumn) :
            Column{CreateWithColumn},
            Line{CreateWithLine}
        {
        }
        
        std::uint64_t Column;
        std::uint64_t Line;
    };
    
    enum class ErrorCode
    {
        InvalidCharacter,
        InvalidClosingElementName
    };
    
    class Error
    {
    public:
        XML::ErrorCode const Code;
        XML::Location const Location;
    };
    
    class Parser
    {
    public:
        Parser(std::istream & InputStream);
        virtual ~Parser() = default;
        auto Parse() -> std::expected<void, XML::Error>;
        
    protected:
        virtual auto Comment(std::string const & Comment, XML::Location const & StartLocation) -> void;
        virtual auto ElementStart(std::string const & TagName, std::map<std::string, std::string> const & Attributes, XML::Location const & StartLocation) -> void;
        virtual auto ElementEnd(std::string const & TagName) -> void;
        virtual auto Text(std::string const & Text, XML::Location const & StartLocation) -> void;
        
    private:
        std::istream & m_InputStream;
    };
}

template<>
struct std::formatter<XML::ErrorCode> : std::formatter<std::string_view>
{
    auto format(XML::ErrorCode Code, std::format_context & Context) const -> std::format_context::iterator
    {
        auto Result = std::string_view{};
        
        switch(Code)
        {
        case XML::ErrorCode::InvalidCharacter:
            {
                Result = "InvalidCharacter";
                
                break;
            }
        case XML::ErrorCode::InvalidClosingElementName:
            {
                Result = "InvalidClosingElementName";
                
                break;
            }
        }
        
        return std::formatter<std::string_view>::format(Result, Context);
    }
};

#endif
