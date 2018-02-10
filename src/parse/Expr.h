#pragma once

#include <string>
#include <memory>
#include <parse/Type.h>
#include <cgen/Generator.h>
#include <cppcoretools/print.h>

namespace ty
{

class Expr
{
public:
    explicit Expr(std::string id = "") : m_id{ std::move(id) } {}

    virtual bool can_evaluate_at_compiletime() const noexcept { return false; }

    //! generates the associated code for the expression and returns a variable that holds the result
    virtual void generate(Generator&) const = 0;

    virtual void print(cct::unique_file& log_file, int level = 1) const = 0;

    //! Infers a type from the expression, if possible
    //! Returns null otherwise
    virtual Type const* inferred_type() const noexcept { return m_inferred_type.get(); }

    virtual Type const* specified_type() const noexcept { return m_specified_type.get(); }

    auto const& id() const noexcept { return m_id; }


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

    void generate(Generator& g) const override { return g.generate(*this); }

    void print(cct::unique_file& log_file, int level) const override
    {
        log_file.printf("%*c Int32LiteralExpr(%s) \n", level, '-', m_expr.c_str());
    }

private:
    std::string		m_expr;
};

class ReturnExpr : public Expr
{
public:
    explicit ReturnExpr(std::unique_ptr<Expr> expr)
        : m_sub_expr{ std::move(expr) } {}

    void generate(Generator& g) const override { return g.generate(*this); }

    void print(cct::unique_file& log_file, int level) const override
    {
        log_file.printf("%*c ReturnExpr \n", level, '-');
        m_sub_expr->print(log_file, level + 1);
    }
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
    bool can_evaluate_at_compiletime() const noexcept 
    { 
        return m_left->can_evaluate_at_compiletime()
            && m_right->can_evaluate_at_compiletime(); 
    }

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

    void generate(Generator& g) const override { return g.generate(*this); }
    
    void print(cct::unique_file& log_file, int level) const override
    {
        log_file.printf("%*c FunctionArgDeclExpr() \n", level, '-');
    }
};

class FunctionCallExpr : public Expr
{
private:
    std::string                         m_function_symbol;

    std::vector<std::unique_ptr<Expr>>	m_arguments;

public:
    FunctionCallExpr(std::string func, decltype(m_arguments) arg)
        : m_function_symbol{ std::move(func) }, m_arguments{ std::move(arg) }
    {}

    void generate(Generator& g) const override { return g.generate(*this); }

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

    void generate(Generator& g) const override { return g.generate(*this); }

    void print(cct::unique_file& log_file, int level) const override
    {
        log_file.printf("%*c IdExpr(%s) \n", level, '-', m_symbol.c_str());
    }
};

struct ParseContext;

class FunctionDefnExpr : public Expr
{
public:

    std::vector<std::unique_ptr<FunctionArgDeclExpr>>	m_arguments;
    std::unique_ptr<ParseContext> m_body;

    // ! Non-owning pointers to return expressions inside of m_body
    std::vector<ReturnExpr*>                  m_returns;

    FunctionDefnExpr(decltype(m_arguments) args, decltype(m_body) body, decltype(m_returns) returns)
        : m_arguments{std::move(args)}, m_body{std::move(body)}, m_returns{std::move(returns)}
    {}

    void print(cct::unique_file& log_file, int level) const override;

    void generate(Generator& g) const override { return g.generate(*this); }
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
