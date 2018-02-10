#pragma once

#include "token/TokenList.h"
#include "parse/Expr.h"
#include "SymbolTable.h"
#include <string>
#include <utility>

namespace ty
{

using ParseIndex = TokenList::const_iterator;

template <typename T>
using Parsed = std::pair<std::unique_ptr<T>, ParseIndex>;

template <typename T>
using ParsedList = std::pair<std::vector<std::unique_ptr<T>>, ParseIndex>;

template <typename T, typename... Args>
auto MakeParsed(ParseIndex next, Args&&... args)
{
    return std::make_pair(std::make_unique<T>(std::forward<Args>(args)...), next);
}

template <typename T, typename... Args>
auto MakeParsedList(ParseIndex next, Args&&... args)
{
    return std::pair<std::vector<std::unique_ptr<T>>, ParseIndex>{std::forward<Args>(args)..., next};
}


class ParseException : public std::exception {
public:
    ParseException(ParseIndex position, std::string msg)
        : m_position {position}
        , m_message{std::move(msg)}
    {
    }

    virtual char const* what() const { return ""; }

    ParseIndex m_position;
    std::string	m_message;
};

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
            throw ParseException(it, "Expected , or ) after function argument declaration");
        }
    }
    return MakeParsedList<FunctionArgDeclExpr>(it, std::move(arg_decl));
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
                throw ParseException(it, "Expected }");
            }
            return MakeParsed<ReturnExpr>(it + 1, std::move(s));
        }
    }
    return MakeParsed<ReturnExpr>(it_begin, nullptr);
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
                    throw ParseException(prev, "Expected ID before '=' token");
                }
                auto expr = ctx.parse_definition(std::string{ prev->begin, prev->end }, it + 1);
                ctx.symbols.add_expr(expr.first->id(), expr.first.get());
                ctx.exprs.emplace_back(std::move(expr.first));
                it = expr.second;
            }
            else
            {
                it++;
            }
        }
        ctx.end = it;
        return ctx;
    }

    template <typename ExitPredicate>
    ParseContext parse_statements_local(ParseIndex it_begin, ExitPredicate finished)
    {
        auto ctx = ParseContext::parse_statements(it_begin, std::move(finished));
        ctx.symbols.set_parent(&symbols);
        return ctx;
    }

    inline Parsed<Expr> parse_function(ParseIndex it_begin)
    {
        std::vector<std::unique_ptr<FunctionArgDeclExpr>> argument_decls;
        std::unique_ptr<ParseContext> body;

        // ! Non-owning pointers to return expressions inside of m_body
        std::vector<ReturnExpr*>                  returns;

        bool single_item = false;
        bool done = false;

        auto it = it_begin;
        while(it->type != LexItem::Type::eof)
        {
            if (it->type == LexItem::Type::param)
            {
                if (!argument_decls.empty())
                {
                    throw ParseException(it, "Unexpected character @");
                }
                it++;
            }
            else if (it->type == LexItem::Type::PAREN_OPEN)
            {
                if (!argument_decls.empty())
                {
                    throw ParseException(it, "Unexpected character (");
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
                }
                else
                {
                    auto stmts = it;
                    while(1)
                    {
                        body = std::make_unique<ParseContext>(parse_statements_local(it, [](ParseIndex i) { return i->type != LexItem::Type::BRACE_CLOSE; }));
                        stmts = body->end;
                        if (stmts->type == LexItem::Type::BRACE_CLOSE)
                        {
                            it = stmts + 1;
                            break;
                        }
                    }
                }
            }
        }
        return MakeParsed<FunctionDefnExpr>(it, std::move(argument_decls), std::move(body), std::move(returns));
    }

    inline Parsed<Expr> parse_definition(std::string name, ParseIndex it)
    {
        if (it->type == LexItem::Type::param)
        {
            return parse_function(it);
        }
        throw ParseException(it, "Expected function definition");
    }

};

inline auto parse(TokenList const& tlist)
{
    try
    {
        return ParseContext::parse_statements(tlist.begin(), [&](ParseIndex i) { return i == tlist.end() || i->type == LexItem::Type::eof; });
    }
    catch (ParseException const& e)
    {
        fprintf(stderr, "Fatal error during parse\n");
        fprintf(stderr, "'%s'\n", tlist.buffer().c_str());
        fputc(' ', stderr);
        for (auto const& c : tlist.buffer())
        {
            if (&c >= e.m_position->begin && &c < e.m_position->end)
            {
                fputc('^', stderr);
            }
            else
            {
                fputc(' ', stderr);
            }
        }
        if (e.m_position->type == LexItem::Type::eof)
        {
            fputc('^', stderr);
        }
        fprintf(stderr, "\n%s", e.m_message.c_str());
        return ParseContext{};
    }
}

}; // namespace ty
