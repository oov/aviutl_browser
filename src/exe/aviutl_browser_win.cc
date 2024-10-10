#include <thread>
#include <Windows.h>

#include "include/cef_parser.h"
#include "include/cef_sandbox_win.h"

#include "app.h"
#include "client.h"
#include "unicode_win.h"
#include "path_win.h"
#include "report_win.h"
#include "version.h"

#define CEF_USE_SANDBOX 1

#if defined(CEF_USE_SANDBOX)
  #pragma comment(lib, "cef_sandbox.lib")
#endif

struct share_mem_header {
	uint32_t header_size;
	uint32_t body_size;
	uint32_t version;
	uint32_t width;
	uint32_t height;
};

struct header {
	uint8_t version;
	uint8_t flags;
	uint16_t path_len;
	uint16_t userfile_len;
	uint16_t tabid_len;
};

inline static BOOL read(HANDLE h, void* buf, DWORD sz)
{
	char* b = (char*)buf;
	for (DWORD read(0); sz > 0; b += read, sz -= read)
	{
		if (!ReadFile(h, b, sz, &read, NULL))
		{
			return FALSE;
		}
	}
	return TRUE;
}

inline static BOOL write(HANDLE h, const void* buf, DWORD sz)
{
	const char* b = (char*)buf;
	for (DWORD written(0); sz > 0; b += written, sz -= written) {
		if (!WriteFile(h, b, sz, &written, NULL))
		{
			return FALSE;
		}
	}
	return TRUE;
}

static int ipc_worker(CefRefPtr<App> app) {
	HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
	if (in == INVALID_HANDLE_VALUE) {
		report(HRESULT_FROM_WIN32(GetLastError()), L"GetStdHandle(STD_INPUT_HANDLE) failed");
		goto finish;
	}
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	if (in == INVALID_HANDLE_VALUE) {
		report(HRESULT_FROM_WIN32(GetLastError()), L"GetStdHandle(STD_OUTPUT_HANDLE) failed");
		goto finish;
	}
	char fmo_name[32] = {};
	{
		if (GetEnvironmentVariableA("BRIDGE_FMO", fmo_name, 32) == 0) {
			DWORD err = GetLastError();
			if (err == ERROR_ENVVAR_NOT_FOUND) {
				OutputDebugString(L"environment variable \"BRIDGE_FMO\" is not found");
				goto finish;
			}
			report(HRESULT_FROM_WIN32(err), L"GetEnvironmentVariable failed");
		}
	}
	HANDLE fmo = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, fmo_name);
	if (!fmo) {
		report(HRESULT_FROM_WIN32(GetLastError()), L"OpenFileMapping failed");
		goto finish;
	}
	struct share_mem_header* view = (struct share_mem_header*)MapViewOfFile(fmo, FILE_MAP_WRITE, 0, 0, 0);
	if (!view) {
		report(HRESULT_FROM_WIN32(GetLastError()), L"MapViewOfFile failed");
		goto close_fmo;
	}
	app->WaitInitialize();

	{
		std::string buf;
		std::wstring path, userfile, tabid, win, wout;
		int32_t len(0);
		while (1) {
			if (!read(in, &len, sizeof(len))) {
				goto unmap_view;
			}
			buf.resize(len);
			if (!read(in, &buf[0], len)) {
				goto unmap_view;
			}
			size_t pos = 0;
			struct header* h = reinterpret_cast<struct header*>(&buf[pos]);
			if (h->version != 2) {
				OutputDebugString(L"Unsupported header version");
				goto unmap_view;
			}
			pos += sizeof(struct header);
			from_sjis(&buf[pos], static_cast<const int>(h->path_len), path);
			pos += h->path_len;
			from_sjis(&buf[pos], static_cast<const int>(h->userfile_len), userfile);
			pos += h->userfile_len;
			from_sjis(&buf[pos], static_cast<const int>(h->tabid_len), tabid);
			pos += h->tabid_len;
			from_sjis(&buf[pos], static_cast<const int>(buf.size() - pos), win);
			const bool r = app->RenderAndCapture(
				path,
				tabid,
				(h->flags & 1) != 0,
				(h->flags & 2) != 0,
				userfile,
				win,
				wout,
				(void*)(((char*)view) + view->header_size), view->width, view->height);
			to_sjis(&wout[0], static_cast<const int>(wout.size()), buf);
			if (!FlushViewOfFile(view, 0)) {
				report(HRESULT_FROM_WIN32(GetLastError()), L"FlushViewOfFile failed");
				goto unmap_view;
			}
			len = 1 + (int32_t)buf.size();
			if (!write(out, &len, sizeof(len))) {
				goto unmap_view;
			}
			const char b = r ? 1 : 0;
			if (!write(out, &b, sizeof(b))) {
				goto unmap_view;
			}
			if (!buf.empty()) {
				if (!write(out, &buf[0], static_cast<DWORD>(buf.size()))) {
					goto unmap_view;
				}
			}
		}
	}

unmap_view:
	if (!UnmapViewOfFile(view)) {
		report(HRESULT_FROM_WIN32(GetLastError()), L"UnmapViewOfFile failed");
	}
close_fmo:
	if (!CloseHandle(fmo)) {
		report(HRESULT_FROM_WIN32(GetLastError()), L"CloseHandle failed");
	}
finish:
	app->CloseAllBrowsers();
	return 1;
}

static bool create_cache_dir(std::wstring& o) {
	std::wstring s;
	if (!get_module_file_name(NULL, s)) {
		return false;
	}
	for (auto i = s.size() - 1; i > 0; --i) {
		if ((s[i] == L'/' || s[i] == L'\\')) {
			s.resize(i + 1);
			s += L"cache";
			break;
		}
	}
	if (!CreateDirectoryW(s.c_str(), NULL)) {
		const DWORD err = GetLastError();
		if (err != ERROR_ALREADY_EXISTS) {
			report(HRESULT_FROM_WIN32(GetLastError()), L"CreateDirectory failed");
			return false;
		}
	}
	o = s;
	return true;
}

int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine,
	int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	// CefEnableHighDPISupport();

	std::thread t1;
	{
		void* sandbox_info = NULL;
#if defined(CEF_USE_SANDBOX)
		CefScopedSandboxInfo scoped_sandbox;
		sandbox_info = scoped_sandbox.sandbox_info();
#endif
		CefMainArgs main_args(hInstance);
		CefRefPtr<App> app(new App);

		int exit_code = CefExecuteProcess(main_args, app, sandbox_info);
		if (exit_code >= 0) {
			return exit_code;
		}

		CefSettings settings;
		settings.windowless_rendering_enabled = true;

		std::wstring cache;
		if (!create_cache_dir(cache)) {
			return 1;
		}
		CefString(&settings.root_cache_path).FromWString(cache);
		CefString(&settings.cache_path).FromWString(cache);

		std::wstring ver(L"AviUtlBrowser/");
		ver += version;
		CefString(&settings.user_agent_product).FromWString(ver);
		
#if !defined(CEF_USE_SANDBOX)
		settings.no_sandbox = true;
#endif

		LANGID langId = GetUserDefaultLangID();
		wchar_t lang[32] = {}, ctry[32] = {};
		if (!GetLocaleInfo(langId, LOCALE_SISO639LANGNAME, lang, 32)) {
			report(HRESULT_FROM_WIN32(GetLastError()), L"GetLocaleInfo failed");
			return 1;
		}
		if (!GetLocaleInfo(langId, LOCALE_SISO3166CTRYNAME, ctry, 32)) {
			report(HRESULT_FROM_WIN32(GetLastError()), L"GetLocaleInfo failed");
			return 1;
		}
		std::wstring locale(lang);
		locale += L"-";
		locale += std::wstring(ctry);
		CefString(&settings.locale).FromWString(locale);
		CefString(&settings.accept_language_list).FromWString(locale);

		CefInitialize(main_args, settings, app.get(), sandbox_info);
		t1 = std::thread(ipc_worker, app);
	}

	CefRunMessageLoop();
	t1.join();
	CefShutdown();
	return 0;
}
