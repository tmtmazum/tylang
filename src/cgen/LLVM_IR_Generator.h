#pragma once

#include "Generator.h"
#include "parse/Parse.h"

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
    
    void generate(ParseContext& context) override;

    std::string generate(FunctionDefnExpr const& expr) override;
    
    std::string generate(Int32LiteralExpr const& expr) override;

    std::string generate(ReturnExpr const& expr) override;

    std::string generate(BinaryOpExpr const& expr) override;

    std::string generate(FunctionCallExpr const& expr) override;

private:
    void begin_function() { m_temp_no = 1; }

    void end_function() { m_temp_no = 0; }

    bool is_inside_function() const { return m_temp_no >= 1; }

    std::vector<ParseContext*>  m_contexts;

    bool m_internal_pass = false;

    int m_temp_no = 0;

};

} // namespace ty
