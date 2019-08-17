#include "client.h"

#include <windows.h>
#include <string>

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_closure_task.h"

#include "resource.h"
#include "report_win.h"
#include "version.h"

void Client::InitializeDevTools(CefRefPtr<CefBrowser> browser) {
	auto c = new PopupClient();
	c->after_created = [&](CefRefPtr<CefBrowser> browser) {
		HWND window = browser->GetHost()->GetWindowHandle();
		HANDLE icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAINICON));
		if (icon) {
			SendMessage(window, WM_SETICON, ICON_BIG, (LPARAM)icon);
			SendMessage(window, WM_SETICON, ICON_SMALL, (LPARAM)icon);
		}
		else {
			report(HRESULT_FROM_WIN32(GetLastError()), L"LoadIcon failed");
		}
		HMENU menu = GetSystemMenu(window, FALSE);
		EnableMenuItem(menu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		// It seems SW_SHOWDEFAULT is used inside CEF.
		// Since bridge.dll passes SW_HIDE in CreateProcess, so the window remains invisible as a result.
		ShowWindow(window, SW_SHOW);

		browser_list_.push_back(browser);
	};
	c->before_close = [&](CefRefPtr<CefBrowser> browser) {
		for (BrowserList::iterator bit = browser_list_.begin(); bit != browser_list_.end(); ++bit) {
			if ((*bit)->IsSame(browser)) {
				browser_list_.erase(bit);
				break;
			}
		}
	};
	c->do_not_close = true;
	CefWindowInfo wi = {};
	std::wstring title(L"AviUtlBrowser DevTools v");
	title += version;
	wi.SetAsPopup(NULL, title.c_str());
	wi.style |= WS_VISIBLE;
	wi.ex_style |= WS_EX_APPWINDOW;
	CefBrowserSettings bs = {};
	CefString utf8(L"UTF-8");
	cef_string_utf16_set(utf8.c_str(), utf8.length(), &bs.default_encoding, false);
	CefPoint pt = {};
	browser->GetHost()->ShowDevTools(wi, c, bs, pt);
	devtools_ = c;
}

CefRefPtr<CefResourceHandler> Client::GetUserFile() {
	LARGE_INTEGER sz = {};
	{
		WIN32_FIND_DATA fd = {};
		HANDLE h = FindFirstFile(userfile_.c_str(), &fd);
		if (h == INVALID_HANDLE_VALUE) {
			DWORD err = GetLastError();
			if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) {
				return StreamResourceHandler::CreateError(404, L"userfile is not found");
			}
			report(HRESULT_FROM_WIN32(err), L"FindFirstFile failed");
			return StreamResourceHandler::CreateError(500, L"cannot get file size");
		}
		if (!FindClose(h)) {
			report(HRESULT_FROM_WIN32(GetLastError()), L"FindClose failed");
		}
		sz.LowPart = fd.nFileSizeLow;
		sz.HighPart = fd.nFileSizeHigh;
	}
	CefString mime;
	assets_->GetMime(userfile_, mime);
	CefResponse::HeaderMap hm;
	hm.insert(std::make_pair(L"Content-Length", CefString(std::to_wstring(sz.QuadPart))));
	return StreamResourceHandler::CreateFromFile(200, mime, hm, userfile_);
}

bool Client::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
	cef_log_severity_t level,
	const CefString& message,
	const CefString& source,
	int line) {
#ifdef _DEBUG
	if (!dev_mode_) {
		std::wostringstream o;
		o << L"[aviutl_browser ";
		if (!source.empty()) {
			o << source.c_str();
			o << L"(";
			o << line;
			o << L") ";
		}
		switch (level) {
		case LOGSEVERITY_DEBUG:
			o << L"DEBUG";
			break;
		case LOGSEVERITY_INFO:
			o << L"INFO";
			break;
		case LOGSEVERITY_WARNING:
			o << L"WARN";
			break;
		case LOGSEVERITY_ERROR:
			o << L"ERROR";
			break;
		}
		o << L"] ";
		o << message.c_str();
		OutputDebugStringW(o.str().c_str());
	}
#endif
	return true;
}