#include <cppcoretools\print.h>
#include "parse/SymbolTable.h"
#include "parse/Parse.h"
#include "cgen/LLVM_IR_Generator.h"
#include "token/TokenList.h"

class UndefinedSymbolException : public std::exception
{
public:
    char const* what() const override { return "Undefined symbol during compilation"; }
};

void run_tests()
{
    using namespace ty;

    auto const ast = parse(tokenize("foo = @() -> {5125421}"));
    for (auto const& expr : ast.exprs)
    {
        expr->print(cct::unique_file{ stdout });
    }
}

int main(int argc, char** argv)
{
    cct::scoped_failure_handler{ [](char const* op)
    {
        fprintf(stderr, "Operation '%s' failed\n", op);
    } };

    if (argc == 1)
    {
        run_tests();
        return 1;
    }

    CCT_CHECK(argc >= 2);

    using namespace ty;

    cct::unique_file in{ argv[1], "r" };

    std::string text;
    for (char c = in.getc(); c != EOF; c = in.getc())
    {
        text += c;
    }
    auto list = tokenize(text);
    for (auto token : list)
    {
        cct::println("; %s", token.as_string().c_str());
    }

    return -1; // fail for now

    //	cct::println("@x = global i32 5, align 4");

    auto ast = parse(list);

    //Global<SymbolTable>().add<Int32LiteralExpr>("x", "5");
    //Global<ExportList>().emplace_back("x");

    LLVM_IR_Generator g{ cct::unique_file{stdout} };
    for (auto const& export_id : Global<ExportList>())
    {
        if (auto const* defn = Global<SymbolTable>().expr_at(export_id))
        {
            defn->generate(g);
        }
        else
        {
            throw UndefinedSymbolException{};
        }
    }
    cct::println(R"(define i32 @five() { ret i32 5 })");

    return 0;
}
