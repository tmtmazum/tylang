if "%2"=="" goto invalid_usage
clang++ -emit-llvm -I"F:\\Microsoft Visual Studio 2014\\VC\\include" -I"C:\\Program Files (x86)\\Windows Kits\\10\Include\\10.0.14393.0\\ucrt" -S %1 -o %2
goto jobs_done

:invalid_usage
echo Usage: emit_llvm.bat <input_file> <output_file>

:jobs_done
