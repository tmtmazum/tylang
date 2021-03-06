#pragma once

#include <cppcoretools/print.h>

namespace ty
{

class Definition;
class DataDefinition;
class Expr;
class LiteralExpr;
class Int32LiteralExpr;
class ReturnExpr;
class SymbolExpr;
class NumExpr;
class BinaryOpExpr;
class FunctionCallExpr;
class FunctionDefnExpr;
class MemberFunctionCallExpr;
class AddExpr;


//! Abstract interface for generating code
class Generator
{
public:
	//! Returns whether or not 'name' satisfies the rules for a valid exportable name 
	//! in the given output langauge
	virtual bool is_exportable_name(std::string const& name) const { return true; }

	//! Export a symbol definition to the identifier 'name'
    //! If name is empty, an anonymous temporary will be created
    virtual void generate(FunctionDefnExpr const& expr) = 0;

    virtual void generate(Int32LiteralExpr const& expr) = 0;

    virtual void generate(ReturnExpr const& expr) = 0;

    // DELETE THIS LATER
    virtual void generate(Expr const& expr) { /* not yet impl */}
};

//! Generalization of a Generator that writes using a unique_file (wrapper around C file)
class FileGenerator : public Generator
{
public:
	explicit FileGenerator(cct::unique_file file)
		: m_file{ std::move(file) }
	{}

protected:
	cct::unique_file m_file;
};

} // namespace ty
