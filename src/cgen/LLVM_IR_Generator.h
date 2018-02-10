#pragma once

#include "Generator.h"

namespace ty
{

class Int32Type;
class Definition;

//! Generates code for the LLVM IR format
class LLVM_IR_Generator : public FileGenerator
{
public:
	template <typename... Args>
	explicit LLVM_IR_Generator(Args&&... args) : FileGenerator(std::forward<Args>(args)...) {}

    virtual void generate(FunctionDefnExpr const& expr) override;
    
    virtual void generate(Int32LiteralExpr const& expr) override;

    virtual void generate(ReturnExpr const& expr) override;

private:
    void begin_function() { m_temp_no = 1; }

    void end_function() { m_temp_no = 0; }

    bool is_inside_function() const { return m_temp_no >= 1; }

    int m_temp_no = 0;

};

} // namespace ty
