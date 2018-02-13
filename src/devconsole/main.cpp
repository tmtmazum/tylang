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

enum ReturnCode { SUCCESS, BAD_PARAMETERS = -1, COMPILE_ERROR = 1 };

int emit_llvm_from_file(cct::unique_file in)
{
    using namespace ty;

    std::string text;
    for (char c = in.getc(); c != EOF && !in.eof(); c = in.getc())
    {
        text += c;
    }
    auto list = tokenize(text);
    auto ast = parse(list);
    ty::LLVM_IR_Generator g{ cct::unique_file{stdout} };
    for (auto const& export_id : ast.export_list)
    {
        if (auto const* expr = ast.symbols.expr_at(export_id))
        {
            expr->generate(g);
        }
        else
        {
            fprintf(stderr, "Cannot find symbol '%s' for export", export_id.c_str());
            return COMPILE_ERROR;
        }
    }
    return SUCCESS;
}

int emit_llvm_from_file(cct::path path_in)
{
    auto in = cct::make_unique_file(path_in, "r");
    if (in.error_code())
    {
        fprintf(stderr, "failed to open input file: '%s', '%s'", path_in.c_str(), in.error_code().message().c_str());
        __debugbreak();
        return BAD_PARAMETERS;
    }

    return emit_llvm_from_file(std::move(in.value()));
}

int main(int argc, char** argv) try
{
    cct::scoped_failure_handler{ [](char const* op)
    {
        fprintf(stderr, "Operation '%s' failed\n", op);
    } };

    TY_ASSERTF(argc >= 2, "argc: %d", argc);

    if (std::string("tokenize") == argv[1])
    {
        auto list = ty::tokenize(argv[2]);
        cct::println("Tokenizing '%s'", argv[2]);
        for (auto const& token : list)
        {
            cct::println("%s", token.as_string().c_str());
        }
        return 0;
    }
    if (std::string("parse") == argv[1])
    {
        auto const ast = ty::parse(ty::tokenize(argv[2]));
        ast.print(cct::unique_file{ stdout });
        return 0;
    }
    if (std::string("emit_llvm") == argv[1])
    {
        ty::LLVM_IR_Generator g{ cct::unique_file{stdout} };
        auto const ast = ty::parse(ty::tokenize(argv[2]));
        for (auto const& export_id : ast.export_list)
        {
            if (auto const* expr = ast.symbols.expr_at(export_id))
            {
                expr->generate(g);
            }
            else
            {
                fprintf(stderr, "Cannot find symbol '%s' for export", export_id.c_str());
            }
        }
        return 0;
    }
    if (std::string("-i") == argv[1])
    {
        return emit_llvm_from_file(argv[2]);
    }
    if (std::string("--stdin") == argv[1])
    {
        return emit_llvm_from_file(cct::unique_file{ stdin });
    }

    return emit_llvm_from_file(argv[1]);
}
catch (ty::TokenException const&)
{
    return 1;
}
catch (ty::ParseException const&)
{
    return 1;
}
