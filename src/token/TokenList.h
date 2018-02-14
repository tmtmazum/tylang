#pragma once

#include <vector>
#include <string>
#include <cctype>
#include <memory>
/*! 
 *-- Example Input ---
 *   id = @(a:int)->{a}
 *   export(id)

 *-- Example OUTPUT(1) ---
 *   ID alpha
 *   DEFN
 *   ID int
 *   BRACE_OPEN
 *   NUM 2
 *   BRACE_CLOSE
 *   ID export
 *   PAREN_OPEN
 *   ID alpha
 *   PAREN_CLOSE
**/

namespace ty
{

    struct LexItem
    {
        enum class Type { UNKNOWN, BRACE_OPEN, COMMA, BRACE_CLOSE, PAREN_OPEN, PAREN_CLOSE, ID, DEFN, DECL, NUM, param, PLUS, MINUS, ARROW, EXPORT, eof };

        Type		type;
        char const* begin;
        char const* end;

        LexItem(Type t, char const* b, char const* e)
            : type{ t }, begin{ b }, end{ e } {}

        static auto as_string(Type t)
        {
            switch (t)
            {
            case Type::BRACE_OPEN: return "brace_open";
            case Type::BRACE_CLOSE: return "brace_close";
            case Type::PAREN_OPEN: return "paren_open";
            case Type::PAREN_CLOSE: return "paren_close";
            case Type::ID: return "id";
            case Type::DEFN: return "defn";
            case Type::DECL: return "decl";
            case Type::param: return "param";
            case Type::PLUS: return "plus";
            case Type::MINUS: return "minus";
            case Type::ARROW: return "arrow";
            case Type::NUM: return "num";
            case Type::COMMA: return "comma";
            case Type::EXPORT: return "export";
            case Type::eof: return "eof";
            default:
            case Type::UNKNOWN: return "unknown";
            }
        }

        bool operator==(Type t) const { return type == t; }

        bool is(Type t) const { return type == t; }
        
        bool is_either(Type t0, Type t1) const { return type == t0 || type == t1; }

        auto as_lexeme() const
        {
            return std::string{ begin, static_cast<size_t>(end - begin) };
        }

        auto as_string() const
        {
            return std::string{ as_string(type) } + " \'" + as_lexeme() + "\'";
        }
    };

    class TokenList : public std::vector<LexItem> 
    {
        // Wrapping a std::string with a std::unique_ptr ensures
        // that pointers into the string will never be invalidated
        // after the ptr is moved from
        std::unique_ptr<std::string> m_buffer;

    public:
        TokenList(decltype(m_buffer) data) : 
            std::vector<LexItem>{}, m_buffer{ std::move(data) } 
        {}

        auto& buffer() { return *m_buffer; }
        auto const& buffer() const { return *m_buffer; }
    };
    
    struct TokenException : public std::exception
    {
        TokenList   m_tokens;
        char const* m_location;
    public:
        TokenException(TokenList tl, char const* location)
            : m_tokens{ std::move(tl) }, m_location(location)
        {}

        char const* what() const override
        {
            return "Token exception ";
        }
    };

    inline auto special_id(char const* lex)
    {
        if (::strncmp("export", lex, std::size("export")-1) == 0)
        {
            return LexItem::Type::EXPORT;
        }
        return LexItem::Type::ID;
    }

    inline TokenList tokenize(std::string s)
    {
        try
        {
            if (!::isspace(s.back()))
            {
                return tokenize(s + " ");
            }

            TokenList list{ std::make_unique<std::string>(std::move(s)) };
            for (auto it = list.buffer().begin(); it != list.buffer().end(); )
            {
                auto const insert_single_char_token = [&](auto token, auto& it)
                {
                    list.emplace_back(token, &*it, &*(it + 1)); it++;
                };

                switch (*it)
                {
                case '(': insert_single_char_token(LexItem::Type::PAREN_OPEN, it); continue;
                case ')': list.emplace_back(LexItem::Type::PAREN_CLOSE, &*it, &*(it + 1)); it++;  continue;
                case '{': list.emplace_back(LexItem::Type::BRACE_OPEN, &*it, &*(it + 1)); it++; continue;
                case '}': list.emplace_back(LexItem::Type::BRACE_CLOSE, &*it, &*(it + 1)); it++;  continue;
                case '=': list.emplace_back(LexItem::Type::DEFN, &*it, &*(it + 1)); it++; continue;
                case ':': list.emplace_back(LexItem::Type::DECL, &*it, &*(it + 1)); it++; continue;
                case '@': list.emplace_back(LexItem::Type::param, &*it, &*(it + 1)); it++; continue;
                case '+': list.emplace_back(LexItem::Type::PLUS, &*it, &*(it + 1)); it++; continue;
                case ',': list.emplace_back(LexItem::Type::COMMA, &*it, &*(it + 1)); it++; continue;
                case '-':
                {
                    auto next = it + 1;
                    if (next != list.buffer().end() && *next == '>')
                    {
                        list.emplace_back(LexItem::Type::ARROW, &*it, &*(next + 1));
                        it = next + 1;
                    }
                    else
                    {
                        list.emplace_back(LexItem::Type::MINUS, &*it, &*(it + 1));
                        it++;
                    }
                }
                break;
                }
                if (::isdigit(*it))
                {
                    auto b = it;
                    while (::isdigit(*it)) { it++; }
                    list.emplace_back(LexItem::Type::NUM, &*b, &*it);
                    continue;
                }
                else if (::isalpha(*it))
                {
                    auto b = it;
                    while (::isalnum(*it) || *it == '_') { it++; }
                    list.emplace_back(special_id(&*b), &*b, &*it);
                    continue;
                }
                else if (::isspace(*it)) { it++; }
                else throw TokenException{std::move(list), &*it};
            }
            list.emplace_back(LexItem::Type::eof, "", "" + 1);
            return list;
        }
        catch (TokenException const& t)
        {
            for (auto const& c : t.m_tokens.buffer())
            {
                if (&c == t.m_location)
                {
                    fprintf(stderr, "%c <-- %s\n", c, "Unexpected token here -- compilation stopped");
                }
                else fputc(c, stderr);
            }
            throw;
        }
    }
}
