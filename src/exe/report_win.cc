#include "report_win.h"

#include <string>

bool report_(HRESULT hr, LPCWSTR message) {
	if (SUCCEEDED(hr)) {
		return false;
	}
	std::wstring m(message);
	m += L": ";
	{
		LPWSTR t = NULL;
		FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			hr,
			LANG_USER_DEFAULT,
			(LPWSTR)&t,
			0,
			NULL);
		m += t;
		LocalFree(t);
	}
	m += L"\n";
	OutputDebugStringW(m.c_str());
	return true;
}
