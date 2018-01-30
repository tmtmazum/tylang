#pragma once

#include <cppcoretools/print.h>

namespace ty
{

class Definition;
class DataDefinition;

class Generator
{
public:
	static bool is_exportable_name(std::string const& name) { return true; }

	virtual void export_as(std::string const& name, Definition const&) = 0;

	virtual void export_as(std::string const& name, DataDefinition const&) = 0;
};

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

