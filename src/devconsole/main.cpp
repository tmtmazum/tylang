#include <cppcoretools\print.h>
#include "parse/SymbolTable.h"
#include "parse/Definition.h"
#include "cgen/LLVM_IR_Generator.h"

class UndefinedSymbolException : public std::exception
{
public:
	char const* what() const override { return "Undefined symbol during compilation"; }
};

int main(int argc, char** argv)
{
	cct::scoped_failure_handler{ [](char const* op)
	{
		fprintf(stderr, "Operation '%s' failed\n", op);
	} };
	CCT_CHECK(argc >= 2);

	using namespace ty;

//	cct::println("@x = global i32 5, align 4");

	Global<SymbolTable>().bind_new<Int32Definition>("x", 5);
	
	Global<ExportList>().emplace_back("x");

	LLVM_IR_Generator g{ cct::unique_file{stdout} };
	for (auto const& export_ : Global<ExportList>())
	{
		if (auto const* defn = Global<SymbolTable>().definition_at(export_))
		{
			g.export_as(export_, *defn);
		}
		else
		{
			throw UndefinedSymbolException{};
		}
	}

	return 0;
}
