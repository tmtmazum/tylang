#include "LLVM_IR_Generator.h"
#include "common/CompileInfo.h"
#include "parse/Parse.h"

namespace ty
{

class CGenException : public std::exception {
public:
    CGenException(CompileError error, std::string msg)
        : m_error_id{error}
        , m_message{std::move(msg)}
    {
    }

    virtual char const* what() const { return (std::string("CGen Exception : ") + m_message).c_str(); }

    CompileError    m_error_id;
    std::string	    m_message;
};

#define CGEN_EXCEPTION_MSG(error_enum, msg) \
    CGenException(error_enum, std::string{#error_enum} + msg + "(" __FUNCTION__ + ":" + std::to_string(__LINE__) + ")")


void LLVM_IR_Generator::generate(ParseContext& ast)
{
    m_contexts.emplace_back(&ast);

    for (auto const& export_id : ast.export_list)
    {
        if (auto const* expr = ast.symbols.expr_at(export_id))
        {
            expr->generate(*this);
        }
        else
        {
            fprintf(stderr, "Cannot find symbol '%s' for export", export_id.c_str());
            throw CGEN_EXCEPTION_MSG(CompileError::cannot_generate_undefined_symbol, export_id);
        }
    }

    m_internal_pass = true;
    for (auto const& internal_id : ast.internal_list)
    {
        if (auto const* expr = ast.symbols.expr_at(internal_id))
        {
            expr->generate(*this);
        }
        else
        {
            fprintf(stderr, "Cannot find symbol '%s' for internal use", internal_id.c_str());
            throw CGEN_EXCEPTION_MSG(CompileError::cannot_generate_undefined_symbol, internal_id);
        }
    }
}

std::string LLVM_IR_Generator::generate(FunctionDefnExpr const& expr)
{
    TY_ASSERTF(is_exportable_name(expr.id()), "expr.id:%s", expr.id().c_str());

    auto type_s = to_string(dynamic_cast<SystemType const&>(*expr.resolved_type()).native_type());

    m_file.printf("define %s @%s() {\n", type_s, expr.id().c_str());
    begin_function();
    for (auto const& a : expr.m_body->exprs)
    {
        a->generate(*this);
    }
    end_function();
    m_file.printf("}\n");
    return "";
}

std::string LLVM_IR_Generator::generate(Int32LiteralExpr const& expr)
{
    TY_ASSERTF(is_inside_function(), "is_inside_function:%d", is_inside_function());
    return expr.value_as_string();
}

std::string LLVM_IR_Generator::generate(ReturnExpr const& expr)
{
    TY_ASSERTF(is_inside_function(), "is_inside_function:%d", is_inside_function());
    auto type_s = to_string(dynamic_cast<SystemType const&>(*expr.resolved_type()).native_type());
    m_file.printf("ret %s %s\n", type_s, expr.child()->generate(*this).c_str());
    return "??";
}
    
std::string LLVM_IR_Generator::generate(BinaryOpExpr const& expr)
{
    TY_ASSERTF(is_inside_function(), "is_inside_function:%d", is_inside_function());
    if (expr.m_op == "+")
    {
        auto type_s = to_string(dynamic_cast<SystemType const&>(*expr.m_left->resolved_type()).native_type());
        auto left = expr.m_left->generate(*this);
        auto right = expr.m_right->generate(*this);
        auto var_no = m_temp_no++;
        m_file.printf("%%%d = add nsw %s %s, %s\n", var_no, type_s, left.c_str(), right.c_str());
        return std::string("%") + std::to_string(var_no);
    }
    else
    {
        throw NOT_YET_IMPLEMENTED();
    }
}
    
std::string LLVM_IR_Generator::generate(FunctionCallExpr const& expr)
{
    TY_ASSERTF(is_inside_function(), "is_inside_function:%d", is_inside_function());

    throw NOT_YET_IMPLEMENTED();

}

} // namespace ty
