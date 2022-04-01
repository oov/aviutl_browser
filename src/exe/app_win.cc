#include "app.h"

#include <shellapi.h>

#include "report_win.h"
#include "assets_win.h"
#include "path_win.h"

static bool to_fullpath(LPCWSTR path, std::wstring& o) {
    DWORD sz = GetCurrentDirectory(0, NULL);
    if (sz == 0) {
        report(HRESULT_FROM_WIN32(GetLastError()), L"GetCurrentDirectory failed");
        return false;
    }
    std::wstring cur;
    cur.resize(sz);
    sz = GetCurrentDirectory(static_cast<DWORD>(cur.size()), &cur[0]);
    if (sz == 0) {
        report(HRESULT_FROM_WIN32(GetLastError()), L"GetCurrentDirectory failed");
        return false;
    }
    std::wstring module;
    if (!get_module_file_name(NULL, module)) {
        return false;
    }
    for (auto i = module.size() - 1; i > 0; --i) {
        if ((module[i] == L'/' || module[i] == L'\\')) {
            module.resize(i + 1);
            module += L"..\\contents";
            break;
        }
    }
    if (!SetCurrentDirectory(module.c_str())) {
        report(HRESULT_FROM_WIN32(GetLastError()), L"SetCurrentDirectory failed");
        return false;
    }

    bool r = get_full_path_name(path, o);
    if (!SetCurrentDirectory(&cur[0])) {
        report(HRESULT_FROM_WIN32(GetLastError()), L"SetCurrentDirectory failed");
        return false;
    }
    return r;
}

static CefRefPtr<Assets> createErrorAssets() {
    return new StaticErrorAssets(500, L"Failed to load the content.<br>コンテンツの読み込みに失敗しました。");
}

CefRefPtr<Assets> App::InitAssets(const std::wstring& path, const bool use_dir) {
    std::wstring o;
    if (use_dir) {
        if (!to_fullpath(path.c_str(), o)) {
            return createErrorAssets();
        }
        CefRefPtr<AssetsFromLocalFile> a = new AssetsFromLocalFile();
        if (!a->SetBasePath(o)) {
            return createErrorAssets();
        }
        return a;
    }
    if (!to_fullpath(path.c_str(), o)) {
        return createErrorAssets();
    }
    CefRefPtr<AssetsFromZip> a = new AssetsFromZip();
    if (!a->SetSourceZip(o)) {
        return createErrorAssets();
    }
    return a;
}
