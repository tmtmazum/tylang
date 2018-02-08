#pragma once
#include <vector>
#include <string>
#include <cctype>

/*! 
INPUT
---
id = @(a:int)->{a}
export(id)

---
OUTPUT(1)
---
ID alpha
DEFN
ID int
BRACE_OPEN
NUM 2
BRACE_CLOSE
ID export
PAREN_OPEN
ID alpha
PAREN_CLOSE

---
*/
namespace ty
{
	struct TokenException : public std::exception
	{

		char const* what() const override
		{
			return "Token exception ";
		}
	};

	struct LexItem
	{
		enum class Type { UNKNOWN, BRACE_OPEN, COMMA, BRACE_CLOSE, PAREN_OPEN, PAREN_CLOSE, ID, DEFN, DECL, NUM, param, PLUS, MINUS, ARROW, eof };

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
            case Type::eof: return "eof";
			default:
			case Type::UNKNOWN: return "unknown";
			}
		}

		auto as_string() const
		{
			return std::string{ as_string(type) } + " \'" + std::string{ begin, end } + "\'";
		}

        auto as_lexeme() const
        {
            return std::string{ begin, end };
        }
	};

	class TokenList : public std::vector<LexItem> 
    {
        std::string m_buffer;

    public:
        TokenList(std::string data) : 
            std::vector<LexItem>{}, m_buffer{ std::move(data) } 
        {}

        auto& buffer() { return m_buffer; }
        auto const& buffer() const { return m_buffer; }
    };

	inline TokenList tokenize(std::string s)
	{
        if (!::isspace(s.back()))
        {
            return tokenize(s + " ");
        }
        
        TokenList list{ std::move(s) };
		for (auto it = list.buffer().begin(); it != list.buffer().end(); )
		{
			switch (*it)
			{
			case '(': list.emplace_back(LexItem::Type::PAREN_OPEN, &*it, &*(it + 1)); it++;  continue;
			case ')': list.emplace_back(LexItem::Type::PAREN_CLOSE, &*it, &*(it + 1)); it++;  continue;
			case '{': list.emplace_back(LexItem::Type::BRACE_OPEN, &*it, &*(it + 1)); it++; continue;
			case '}': list.emplace_back(LexItem::Type::BRACE_CLOSE, &*it, &*(it + 1)); it++;  continue;
			case '=': list.emplace_back(LexItem::Type::DEFN, &*it, &*(it + 1)); it++; continue;
			case ':': list.emplace_back(LexItem::Type::DECL, &*it, &*(it + 1)); it++; continue;
			case '@': list.emplace_back(LexItem::Type::param, &*it, &*(it + 1)); it++; continue;
			case '+': list.emplace_back(LexItem::Type::PLUS, &*it, &*(it + 1)); it++; continue;
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
				while (::isalnum(*it)) { it++; }
				list.emplace_back(LexItem::Type::ID, &*b, &*it);
				continue;
			}
			else if (::isspace(*it)) { it++; }
			else throw TokenException{};
		}
        list.emplace_back(LexItem::Type::eof, "", "" + 1);
		return list;
	}
}
