#pragma once

#include "token/TokenList.h"
#include "common/CompileInfo.h"
#include <utility>
#include <memory>
#include <vector>

namespace ty
{

using ParseIndex = TokenList::const_iterator;

template <typename T>
using Parsed = std::pair<std::unique_ptr<T>, ParseIndex>;

template <typename T>
using ParsedList = std::pair<std::vector<std::unique_ptr<T>>, ParseIndex>;

template <typename T, typename... Args>
auto make_parsed(ParseIndex next, Args&&... args)
{
    return std::make_pair(std::make_unique<T>(std::forward<Args>(args)...), next);
}

template <typename T, typename... Args>
auto make_parsed_list(ParseIndex next, Args&&... args)
{
    return std::pair<std::vector<std::unique_ptr<T>>, ParseIndex>{std::forward<Args>(args)..., next};
}


class ParseException : public std::exception {
public:
    ParseException(ParseIndex position, CompileError error, std::string msg)
        : m_position {position}
        , m_error_id{error}
        , m_message{std::move(msg)}
    {
    }

    virtual char const* what() const { return (std::string("Parse Exception : ") + m_message).c_str(); }

    ParseIndex      m_position;
    CompileError    m_error_id;
    std::string	    m_message;
};

#define PARSE_EXCEPTION(position, error_enum) \
    ParseException(position, error_enum, std::string{#error_enum} + __FUNCTION__ + ":" + std::to_string(__LINE__))

inline auto make_unexpected_end_of_file(ParseIndex it, std::string operation)
{
    return ParseException{ it, CompileError::unexpected_end_of_file,
        std::string{"Encountered premature end-of-file while parsing \'"} + operation + "\'" };
}

}