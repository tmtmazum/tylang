#pragma once

#include "Generator.h"

namespace ty
{

class LLVM_IR_Generator : public FileGenerator
{
public:
	template <typename... Args>
	explicit LLVM_IR_Generator(Args&&... args) : FileGenerator(std::forward<Args>(args)...) {}

	void export_as(std::string const& name, Definition const& d) override;

	void export_as(std::string const& name, DataDefinition const& d) override;
};

} // namespace ty
