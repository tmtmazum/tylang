#pragma once

#include "token/TokenList.h"
#include "parse/Expr.h"
#include "SymbolTable.h"
#include "common/CompileInfo.h"
#include "ParseException.h"
#include <string>
#include <utility>
#include <iterator>

namespace ty
{

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

struct ParseContext
{
    ParseIndex                          begin;
    ParseIndex                          end;
    SymbolTable                         symbols;
    ExprList                            exprs;
    std::vector<std::string>            export_list;

    void print(cct::unique_file& out) const
    {
        for (auto const& expr : exprs)
        {
            expr->print(out);
        }

        out.printf("exported symbols: ");
        for (auto const& exp : export_list)
        {
            out.printf("%s, ", exp.c_str());
        }
    }

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
                if (prev->type == LexItem::Type::ID)
                {
                    auto expr = ctx.parse_function_call(std::string{ prev->begin, prev->end }, it + 1);
                    ctx.exprs.emplace_back(std::move(expr.first));
                    it = expr.second;
                }
                else
                {
                    throw ParseException(prev, CompileError::unexpected_token, "Expected ID before '(' token");
                }
            }
            else if(it->type == LexItem::Type::ID)
            {
                it++;
            }
            else if (it->type == LexItem::Type::EXPORT)
            {
                auto next = it + 1;
                if (next->type != LexItem::Type::PAREN_OPEN)
                {
                    throw ParseException(next, CompileError::unexpected_token, "Expected ( after export");
                }
                it = ctx.parse_exports(next + 1, std::back_inserter(ctx.export_list));
            }
            else
            {
                throw ParseException(it, CompileError::unexpected_token, "Unexpected token while parsing statements");
            }
        }
        ctx.end = it + 1;
        return ctx;
    }

    template <typename StringInserter>
    auto parse_exports(ParseIndex it_begin, StringInserter insert)
    {
        auto it = it_begin;
            
        while (1)
        {
            if (it->type == LexItem::Type::eof)
            {
                throw make_unexpected_end_of_file(it, "function call");
            }
            else if (it->type == LexItem::Type::ID)
            {
                insert = std::string{ it->begin, it->end };
                it = it + 1;
                if (it->type == LexItem::Type::COMMA)
                {
                    it = it + 1;
                }
                else if (it->type == LexItem::Type::PAREN_CLOSE)
                {
                    return (it = it + 1);
                }
                else
                {
                    throw ParseException(it, CompileError::unexpected_token, "Unexpected token during parsing exports");
                }
            }
            else
            {
                throw ParseException(it, CompileError::unexpected_token, "Unexpected token during parsing exports");
            }
        }
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
                    if (auto func_expr = symbols.expr_at(function_symbol))
                    {
                        return make_parsed<FunctionCallExpr>(it + 2, function_symbol, func_expr, std::move(arguments));
                    }
                    else
                    {
                        throw ParseException(it, CompileError::undefined_function_being_called, "Call to undefined function");
                    }
                default:
                    throw ParseException(it, CompileError::unexpected_token, "Unexpected token during parse_function_call");
                }
            }
            else if(it->type == LexItem::Type::NUM)
            {
                switch ((it + 1)->type)
                {
                case LexItem::Type::COMMA:
                    arguments.emplace_back(std::make_unique<Int32LiteralExpr>(it->as_lexeme()));
                    it = it + 2;
                    break;
                case LexItem::Type::PAREN_CLOSE:
                    arguments.emplace_back(std::make_unique<Int32LiteralExpr>(it->as_lexeme()));
                    if (auto func_expr = symbols.expr_at(function_symbol))
                    {
                        return make_parsed<FunctionCallExpr>(it + 2, function_symbol, func_expr, std::move(arguments));
                    }
                    else
                    {
                        throw ParseException(it, CompileError::undefined_function_being_called, "Call to undefined function");
                    }
                default:
                    throw ParseException(it, CompileError::unexpected_token, "Unexpected token during parse_function_call");
                }
            }
            else if(it->type == LexItem::Type::PAREN_CLOSE)
            {
                if (auto func_expr = symbols.expr_at(function_symbol))
                {
                    return make_parsed<FunctionCallExpr>(it + 1, function_symbol, func_expr, std::move(arguments));
                }
                else
                {
                    throw ParseException(it, CompileError::undefined_function_being_called, "Call to undefined function");
                }
            }
            else
            {
                throw ParseException(it, CompileError::unexpected_token, "Unexpected token while parsing function call");
            }
        }
        throw ParseException(it, CompileError::unexpected_token, "Unexpected token while parsing function call");
    }

    //  30          OK - Int32LiteralExpr
    //  foo(30)     OK - FunctionCallExpr
    //  foo(2) + foo(2) OK - 
    inline Parsed<Expr> parse_until_brace(ParseIndex it_begin)
    {
        ParsedList<Expr> list;
        auto it = it_begin;
        while (!it->is(LexItem::Type::eof))
        {
            if(it->is(LexItem::Type::NUM))
            {
                std::tie(std::back_inserter(list.first), it) = make_parsed<Int32LiteralExpr>(it + 1, it->as_lexeme());
            }
            else if (it->is(LexItem::Type::ID) && (it + 1)->is(LexItem::Type::PAREN_OPEN))
            {
                std::tie(std::back_inserter(list.first), it) = parse_function_call(it->as_lexeme(), it + 2);
            }
            else if (it->is(LexItem::Type::PLUS))
            {
                if (list.first.empty())
                {
                    throw ParseException(it, CompileError::unexpected_token, "Expected operands before + operator");
                }
                else if (list.first.size() != 1)
                {
                    throw ParseException(it - 1, CompileError::unexpected_token, "Expected exactly one operand before + operator");
                }
                auto rhs = parse_until_brace(it + 1);
                return make_parsed<BinaryOpExpr>(rhs.second, std::move(list.first.front()), it->as_lexeme(), std::move(rhs.first));
            }
            else if (it->is(LexItem::Type::BRACE_CLOSE))
            {
                it = it + 1;
                break;
            }
            else
            {
                throw PARSE_EXCEPTION(it, CompileError::unexpected_token);
            }
        }
        if (list.first.size() == 1)
        {
            return Parsed<Expr>{std::move(list.first.front()), it};
        }
        throw PARSE_EXCEPTION(it, CompileError::unexpected_token);
    }

    inline Parsed<FunctionDefnExpr> parse_function(std::string name, ParseIndex it_begin)
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

                    auto r = parse_until_brace(it + 1);
                    if (auto Ret = std::make_unique<ReturnExpr>(std::move(r.first)))
                    {
                        returns.emplace_back(Ret.get());
                        body->exprs.emplace_back(std::move(Ret));
                    }
                    it = r.second;
                    return make_parsed<FunctionDefnExpr>(it, std::move(name), std::move(argument_decls), std::move(body), std::move(returns));
                }
                else
                {
                    auto stmts = it;
                    while(1)
                    {
                        body = std::make_unique<ParseContext>(parse_statements_local(it, [](ParseIndex i) { return i->type != LexItem::Type::BRACE_CLOSE; }));
                        stmts = body->end;
                        return make_parsed<FunctionDefnExpr>(it, std::move(name), std::move(argument_decls), std::move(body), std::move(returns));
                    }
                }
            }
            else if (it->type == LexItem::Type::eof)
            {
                throw make_unexpected_end_of_file(it, "function definition");
            }
        }
    }

    inline Parsed<DefnExpr> parse_definition(std::string name, ParseIndex it)
    {
        if (it->type == LexItem::Type::param)
        {
            return parse_function(std::move(name), it);
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
        //fprintf(stderr, "'%s'\n", tlist.buffer().c_str());
        fputc(' ', stderr);
        bool found = false;
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
