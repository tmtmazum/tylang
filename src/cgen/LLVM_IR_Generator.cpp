#include "LLVM_IR_Generator.h"
#include "parse/Definition.h"

namespace ty
{

void LLVM_IR_Generator::export_as(std::string const& name, Definition const& d)
{
	if (auto const* data = dynamic_cast<DataDefinition const*>(&d))
	{
		export_as(name, *data);
	}
}

void LLVM_IR_Generator::export_as(std::string const& name, DataDefinition const& d)
{
	CCT_CHECK(is_exportable_name(name));

	m_file.printf("@%s = global %s %d, align %d\n", 
		name.c_str(), to_string(d.native_type()), d.value(), d.alignment());
}


}