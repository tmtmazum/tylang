#pragma once

#include "TyObject.h"

namespace ty
{
    class CompileNotice
    {

    };

    enum class CompileWarning
    {

    };

    //! Fatal compile errors 
    enum class CompileError
    {
        unexpected_token,    // unexpected token in this context
        unexpected_end_of_file,
        deduced_type_mismatch,
        unsupported_feature
    };
}; // namespace ty
