#pragma once

#include <cppcoretools/print.h>

namespace ty
{

class Definition;
class DataDefinition;

//! Abstract interface for generating code
class Generator
{
public:
	//! Returns whether or not 'name' satisfies the rules for a valid exportable name 
	//! in the given output langauge
	virtual bool is_exportable_name(std::string const& name) const { return true; }

	//! Export a symbol definition to the identifier 'name'
	virtual void export_as(std::string const& name, Definition const&) = 0;

	//! Export a symbol DataDefinition to the identifier 'name'
	virtual void export_as(std::string const& name, DataDefinition const&) = 0;
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
