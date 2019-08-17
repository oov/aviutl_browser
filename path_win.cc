#include "path_win.h"

#include "report_win.h"

bool get_module_file_name(const HMODULE h, std::wstring& o) {
    o.resize(0);
    while (o.size() < 4096) {
        o.resize(o.size() + MAX_PATH);
        DWORD r = GetModuleFileName(h, &o[0], static_cast<DWORD>(o.size()));
        if (r == 0) {
            report(HRESULT_FROM_WIN32(GetLastError()), L"GetModuleFileName failed");
            return false;
        }
        if (r < o.size()) {
            o.resize(r);
            return true;
        }
    }
    return false;
}

bool get_full_path_name(LPCWSTR path, std::wstring& o) {
    DWORD r = GetFullPathName(path, 0, NULL, NULL);
    if (r == 0) {
        report(HRESULT_FROM_WIN32(GetLastError()), L"GetFullPathName failed");
        return false;
    }
    o.resize(r); // including the terminating null character
    r = GetFullPathName(path, static_cast<DWORD>(o.size()), &o[0], NULL);
    if (r == 0) {
        report(HRESULT_FROM_WIN32(GetLastError()), L"GetFullPathName failed");
        return false;
    }
    o.resize(r); // not including the terminating null character
    return true;
}
