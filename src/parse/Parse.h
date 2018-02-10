#pragma once

#include "token/TokenList.h"
#include "parse/Expr.h"
#include "SymbolTable.h"
#include "common/CompileInfo.h"
#include <string>
#include <utility>
#include <iterator>

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

    virtual char const* what() const { return ""; }

    ParseIndex      m_position;
    CompileError    m_error_id;
    std::string	    m_message;
};

inline auto make_unexpected_end_of_file(ParseIndex it, std::string operation)
{
    return ParseException{ it, CompileError::unexpected_end_of_file,
        std::string{"Encountered premature end-of-file while parsing \'"} + operation + "\'" };
}

inline ParsedList<FunctionArgDeclExpr> parse_argument_decls(ParseIndex it_begin)
{
    std::vector<std::unique_ptr<FunctionArgDeclExpr>> arg_decl;

    auto it = it_begin;
    while (1)
    {
        if (it->type == LexItem::Type::PAREN_CLOSE)
        {
            it++;
            break;
        }
        else
        {
            throw ParseException(it, CompileError::unexpected_token, "Expected , or ) after function argument declaration");
        }
    }
    return make_parsed_list<FunctionArgDeclExpr>(it, std::move(arg_decl));
}

inline Parsed<ReturnExpr> parse_return_expr(ParseIndex it_begin)
{

    for (auto it = it_begin; ; it++)
    {
        if (it->type == LexItem::Type::NUM)
        {
            auto s = std::make_unique<Int32LiteralExpr>(it->as_lexeme());
            it++;
            if (it->type != LexItem::Type::BRACE_CLOSE)
            {
                throw ParseException(it, CompileError::unexpected_token, "Expected }");
            }
            return make_parsed<ReturnExpr>(it + 1, std::move(s));
        }
    }
    return make_parsed<ReturnExpr>(it_begin, nullptr);
}

struct ParseContext
{
    ParseIndex                          begin;
    ParseIndex                          end;
    SymbolTable                         symbols;
    ExprList                            exprs;

    template <typename ExitPredicate>
    static ParseContext parse_statements(ParseIndex it_begin, ExitPredicate finished)
    {
        ParseContext ctx;

        ctx.begin = it_begin;

        auto it = it_begin;
        while(!finished(it))
        {
            if (it->type == LexItem::Type::DEFN)
            {
                auto const prev = it - 1;
                if (prev->type != LexItem::Type::ID)
                {
                    throw ParseException(prev, CompileError::unexpected_token, "Expected ID before '=' token");
                }
                auto expr = ctx.parse_definition(std::string{ prev->begin, prev->end }, it + 1);
                ctx.symbols.add_expr(expr.first->id(), expr.first.get());
                ctx.exprs.emplace_back(std::move(expr.first));
                it = expr.second;
            }
            else if (it->type == LexItem::Type::PAREN_OPEN)
            {
                auto const prev = it - 1;
                if (prev->type != LexItem::Type::ID)
                {
                    throw ParseException(prev, CompileError::unexpected_token, "Expected ID before '(' token");
                }
                auto expr = ctx.parse_function_call(std::string{ prev->begin, prev->end }, it + 1);
                ctx.exprs.emplace_back(std::move(expr.first));
                it = expr.second;
            }
            else if(it->type == LexItem::Type::ID)
            {
                it++;
            }
            else
            {
                throw ParseException(it, CompileError::unexpected_token, "Unexpected token while parsing statements");
            }
        }
        ctx.end = it + 1;
        return ctx;
    }

    template <typename ExitPredicate>
    ParseContext parse_statements_local(ParseIndex it_begin, ExitPredicate finished)
    {
        auto ctx = ParseContext::parse_statements(it_begin, std::move(finished));
        ctx.symbols.set_parent(&symbols);
        return ctx;
    }
    
    inline Parsed<FunctionCallExpr> parse_function_call(std::string function_symbol, ParseIndex it_begin)
    {
        std::vector<std::unique_ptr<Expr>>	arguments;

        auto it = it_begin;
        while (1)
        {
            if (it->type == LexItem::Type::eof)
            {
                throw make_unexpected_end_of_file(it, "function call");
            }
            else if (it->type == LexItem::Type::ID)
            {
                switch ((it + 1)->type)
                {
                case LexItem::Type::PAREN_OPEN:
                {
                    auto sub_call = parse_function_call(it->as_lexeme(), it + 2);
                    arguments.emplace_back(std::move(sub_call.first));
                    it = sub_call.second;
                }
                break;
                case LexItem::Type::COMMA:
                    arguments.emplace_back(std::make_unique<IdExpr>(it->as_lexeme()));
                    it = it + 2;
                    break;
                case LexItem::Type::PAREN_CLOSE:
                    arguments.emplace_back(std::make_unique<IdExpr>(it->as_lexeme()));
                    return make_parsed<FunctionCallExpr>(it + 2, function_symbol, std::move(arguments));
                default:
                    throw ParseException(it, CompileError::unexpected_token, "Unexpected token during parse_function_call");
                }
            }
            else if(it->type == LexItem::Type::NUM)
            {
                switch ((it + 1)->type)
                {
                case LexItem::Type::COMMA:
                    arguments.emplace_back(std::make_unique<Int32LiteralExpr>((it + 1)->as_lexeme()));
                    it = it + 2;
                    break;
                case LexItem::Type::PAREN_CLOSE:
                    arguments.emplace_back(std::make_unique<Int32LiteralExpr>((it + 1)->as_lexeme()));
                    return make_parsed<FunctionCallExpr>(it + 2, function_symbol, std::move(arguments));
                default:
                    throw ParseException(it, CompileError::unexpected_token, "Unexpected token during parse_function_call");
                }
            }
            else if(it->type == LexItem::Type::PAREN_CLOSE)
            {
                return make_parsed<FunctionCallExpr>(it + 1, function_symbol, std::move(arguments));
            }
            else
            {
                throw ParseException(it, CompileError::unexpected_token, "Unexpected token while parsing function call");
            }
        }
        throw ParseException(it, CompileError::unexpected_token, "Unexpected token while parsing function call");
    }

    inline Parsed<Expr> parse_function(ParseIndex it_begin)
    {
        std::vector<std::unique_ptr<FunctionArgDeclExpr>> argument_decls;
        std::unique_ptr<ParseContext> body;

        // ! Non-owning pointers to return expressions inside of m_body
        std::vector<ReturnExpr*>                  returns;

        bool single_item = false;

        auto it = it_begin;
        while(1)
        {
            if (it->type == LexItem::Type::param)
            {
                if (!argument_decls.empty())
                {
                    throw ParseException(it, CompileError::unexpected_token, "Unexpected character @");
                }
                it++;
            }
            else if (it->type == LexItem::Type::PAREN_OPEN)
            {
                if (!argument_decls.empty())
                {
                    throw ParseException(it, CompileError::unexpected_token, "Unexpected character (");
                }
                auto arg_decls = parse_argument_decls(it + 1);
                argument_decls = std::move(arg_decls.first);
                it = arg_decls.second;
            }
            else if (it->type == LexItem::Type::ARROW)
            {
                single_item = true;
                it++;
            }
            else if (it->type == LexItem::Type::BRACE_OPEN)
            {
                if (single_item)
                {
                    body = std::make_unique<ParseContext>();

                    auto r = parse_return_expr(it + 1);
                    returns.emplace_back(r.first.get());
                    body->exprs.emplace_back(std::move(r.first));
                    it = r.second;
                    return make_parsed<FunctionDefnExpr>(it, std::move(argument_decls), std::move(body), std::move(returns));
                }
                else
                {
                    auto stmts = it;
                    while(1)
                    {
                        body = std::make_unique<ParseContext>(parse_statements_local(it, [](ParseIndex i) { return i->type != LexItem::Type::BRACE_CLOSE; }));
                        stmts = body->end;
                        return make_parsed<FunctionDefnExpr>(it, std::move(argument_decls), std::move(body), std::move(returns));
                    }
                }
            }
            else if (it->type == LexItem::Type::eof)
            {
                throw make_unexpected_end_of_file(it, "function definition");
            }
        }
    }

    inline Parsed<Expr> parse_definition(std::string name, ParseIndex it)
    {
        if (it->type == LexItem::Type::param)
        {
            return parse_function(it);
        }
        throw ParseException(it, CompileError::unsupported_feature, "Expected function definition");
    }

};

inline auto parse(TokenList const& tlist)
{
    if (tlist.empty())
    {
        return ParseContext{};
    }

    try
    {
        return ParseContext::parse_statements(tlist.begin(), [&](ParseIndex i) { return i == tlist.end() || i->type == LexItem::Type::eof; });
    }
    catch (ParseException const& e)
    {
        fprintf(stderr, "Fatal error during parse\n");
        fprintf(stderr, "'%s'\n", tlist.buffer().c_str());
        fputc(' ', stderr);
        bool found = false;
        auto* begin = &tlist.buffer().front();
        for (auto const& c : tlist.buffer())
        {
            if (&c >= e.m_position->begin && &c < e.m_position->end)
            {
                fprintf(stderr, "%c <-- %s\n", c, e.m_message.c_str());
            }
            else fputc(c, stderr);
        }
        //fprintf(stderr, "\n%s", e.m_message.c_str());
        throw;
    }
}

}; // namespace ty
