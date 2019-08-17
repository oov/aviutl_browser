#pragma once

#include <string>
#include <Windows.h>

bool get_module_file_name(const HMODULE h, std::wstring& o);
bool get_full_path_name(LPCWSTR path, std::wstring& o);
