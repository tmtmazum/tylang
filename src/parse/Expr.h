#pragma once

#include <string>
#include <memory>
#include <parse/Type.h>
#include <cgen/Generator.h>
#include "common/CompileInfo.h"
#include "ParseException.h"
#include <cppcoretools/print.h>

namespace ty
{

class LogicalException : public std::exception {
public:

    virtual char const* what() const { return ""; }

    CompileError    m_error_id;
    std::string	    m_message;

    LogicalException(CompileError error, std::string msg)
        : m_error_id{error}
        , m_message{std::move(msg)}
    {}
};

class Expr
{
public:
    virtual bool can_evaluate_at_compiletime() const noexcept { return false; }

    //! generates the associated code for the expression and returns a variable that holds the result
    virtual std::string generate(Generator&) const = 0;

    virtual void print(cct::unique_file& log_file, int level = 1) const = 0;

    //! Infers a type from the expression, if possible
    //! Returns null otherwise
    virtual Type const* inferred_type() const noexcept { return m_inferred_type.get(); }

    virtual Type const* specified_type() const noexcept { return m_specified_type.get(); }

    virtual Type const* resolved_type() const
    {
        auto inferred = inferred_type();
        auto specified = specified_type();
        if (inferred && specified && *inferred != *specified)
        {
            throw LogicalException(CompileError::deduced_type_mismatch, "");
        }
        return specified ? specified : inferred;
    }

protected:
    
    std::string m_id;

    std::unique_ptr<Type>   m_specified_type;

    std::unique_ptr<Type>   m_inferred_type;
};

using ExprList = std::vector<std::unique_ptr<Expr>>;

class Int32LiteralExpr : public Expr
{
public:
    explicit Int32LiteralExpr(std::string expr_str)
        : Expr{}
        , m_expr {std::move(expr_str)} 
    {
        m_inferred_type = std::make_unique<Int32Type>();
    }

    std::string generate(Generator& g) const override { return g.generate(*this); }

    void print(cct::unique_file& log_file, int level) const override
    {
        log_file.printf("%*c Int32LiteralExpr(%s) \n", level, '-', m_expr.c_str());
    }

    auto const& value_as_string() const { return m_expr; }

private:
    std::string		m_expr;
};

//! An expression that evaluates to a single value
class SingleExpr : public Expr
{
    auto const* child() const { return m_sub_expr.get(); }

private:
    std::unique_ptr<Expr>   m_sub_expr;

};

class ReturnExpr : public SingleExpr
{
public:
    explicit ReturnExpr(std::unique_ptr<Expr> expr)
        : m_sub_expr{ std::move(expr) } {}

    std::string generate(Generator& g) const override { return g.generate(*this); }

    Type const* inferred_type() const noexcept override { return m_sub_expr->inferred_type(); }

    void print(cct::unique_file& log_file, int level) const override
    {
        log_file.printf("%*c ReturnExpr \n", level, '-');
        m_sub_expr->print(log_file, level + 1);
    }

    auto const* child() const { return m_sub_expr.get(); }

    auto * child() { return m_sub_expr.get(); }

private:
    std::unique_ptr<Expr>   m_sub_expr;
};

class SymbolExpr : public Expr
{
    void print(cct::unique_file& log_file, int level) const override
    {
        log_file.printf("%*c ReturnExpr \n", level, '-');
    }
};

class BinaryOpExpr : public Expr
{
public:
    BinaryOpExpr(std::unique_ptr<Expr> exprL, std::unique_ptr<Expr> exprR)
        : m_left{ std::move(exprL) }, m_right{ std::move(exprR) }
    {}

    bool can_evaluate_at_compiletime() const noexcept 
    { 
        return m_left->can_evaluate_at_compiletime()
            && m_right->can_evaluate_at_compiletime(); 
    }

    std::string generate(Generator& g) const override { return g.generate(*this); }

    Type const* inferred_type() const noexcept override 
    {
        auto l_type = m_left->inferred_type();
        if (*l_type != *(m_right->inferred_type()))
        {
            return nullptr; // not valid
        }
        return l_type;
    }

    void print(cct::unique_file& log_file, int level) const override
    {
        log_file.printf("%*c BinaryOpExpr \n", level, '-');
        m_left->print(log_file, level + 1);
        m_right->print(log_file, level + 1);
    }

private:
    std::unique_ptr<Expr> m_left;
    std::unique_ptr<Expr> m_right;
};

class FunctionArgDeclExpr : public Expr
{
public:
    FunctionArgDeclExpr() = default;

    std::string generate(Generator& g) const override { return g.generate(*this); }
    
    void print(cct::unique_file& log_file, int level) const override
    {
        log_file.printf("%*c FunctionArgDeclExpr() \n", level, '-');
    }
};

class FunctionCallExpr : public Expr
{
public:
    std::string                         m_function_symbol;

    std::vector<std::unique_ptr<Expr>>	m_arguments;

    FunctionCallExpr(std::string func, decltype(m_arguments) arg)
        : m_function_symbol{ std::move(func) }, m_arguments{ std::move(arg) }
    {}

    std::string generate(Generator& g) const override { return g.generate(*this); }

    void print(cct::unique_file& log_file, int level) const override
    {
        log_file.printf("%*c FunctionCallExpr(%s) \n", level, '-', m_function_symbol.c_str());
        for (auto const& arg : m_arguments)
        {
            arg->print(log_file, level + 1);
        }
    }
};

class IdExpr : public Expr
{
private:
    std::string         m_symbol;

public:
    explicit IdExpr(std::string sym) : m_symbol{ std::move(sym) } {}

    std::string generate(Generator& g) const override { return g.generate(*this); }

    void print(cct::unique_file& log_file, int level) const override
    {
        log_file.printf("%*c IdExpr(%s) \n", level, '-', m_symbol.c_str());
    }
};

struct ParseContext;

class DefnExpr : public Expr
{
    std::string m_id;
public:
    explicit DefnExpr(std::string id) : m_id{ std::move(id) } {}

    std::string const& id() const { return m_id; }
};

class FunctionDefnExpr : public DefnExpr
{
public:
    std::vector<std::unique_ptr<FunctionArgDeclExpr>>	m_arguments;
    std::unique_ptr<ParseContext> m_body;

    // ! Non-owning pointers to return expressions inside of m_body
    std::vector<ReturnExpr*>                  m_returns;

    FunctionDefnExpr(std::string name, decltype(m_arguments) args, decltype(m_body) body, decltype(m_returns) returns)
        : DefnExpr{std::move(name)}
        , m_arguments { std::move(args)}
        , m_body{ std::move(body) }
        , m_returns{ std::move(returns) }
    {}

    Type const* inferred_type() const noexcept override { 
    // CHECK HERE THAT ALL RETURN TYPES ARE EQUIVALENT
        return m_returns.front()->inferred_type(); 
    }
    
    void print(cct::unique_file& log_file, int level) const override;

    std::string generate(Generator& g) const override { return g.generate(*this); }
};


class MemberFunctionCallExpr : public Expr
{
private:
    std::vector<std::unique_ptr<Expr>>	m_arguments;
};

class AddExpr : public BinaryOpExpr
{

};

} // namespace ty
