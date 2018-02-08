#include "LLVM_IR_Generator.h"
#include "parse/Parse.h"

namespace ty
{
void LLVM_IR_Generator::generate(FunctionDefnExpr const& expr)
{
    CCT_CHECK(is_exportable_name(expr.id()));

    m_file.printf("define %s @%s() {");
    begin_function();
    for (auto const& a : expr.m_body->exprs)
    {
        a->generate(*this);
    }
    end_function();
}

void LLVM_IR_Generator::generate(Int32LiteralExpr const& expr)
{

}

void LLVM_IR_Generator::generate(ReturnExpr const& expr)
{
}

} // namespace ty
