#include "Expr.h"
#include "Parse.h"

namespace ty
{

void FunctionDefnExpr::print(cct::unique_file& log_file, int level) const
{
    log_file.printf("%*c FunctionDefnExpr() \n", level, '-');
    for (auto const& a : m_arguments)
    {
        a->print(log_file, level + 1);
    }
    for (auto const& a : m_body->exprs)
    {
        a->print(log_file, level + 1);
    }
}

}