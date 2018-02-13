#include "LLVM_IR_Generator.h"
#include "parse/Parse.h"

namespace ty
{
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
    auto type_s = to_string(dynamic_cast<SystemType const&>(*expr.resolved_type()).native_type());
    m_file.printf("ret %s %s\n", type_s, expr.child()->generate(*this).c_str());
    return "??";
}

} // namespace ty
