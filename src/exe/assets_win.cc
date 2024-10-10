#include "assets_win.h"

#include <shlwapi.h>

#include "unicode_win.h"
#include "report_win.h"

CefRefPtr<CefResourceHandler> AssetsFromLocalFile::Get(const CefString& path) {
	std::wstring tmp(base_path_.ToWString());
	tmp += path;
	WCHAR file[MAX_PATH] = {};
	DWORD len = MAX_PATH;
	{
		const HRESULT hr = PathCreateFromUrlW(tmp.c_str(), file, &len, 0);
		if (FAILED(hr)) {
			report(hr, L"PathCreateFromUrlW failed");
			return StreamResourceHandler::CreateError(500, L"cannot convert url to local path");
		}
	}
	file[len] = L'\0';
	LARGE_INTEGER sz = {};
	{
		WIN32_FIND_DATA fd = {};
		HANDLE h = FindFirstFile(file, &fd);
		if (h == INVALID_HANDLE_VALUE) {
			DWORD err = GetLastError();
			if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) {
				std::wstring ws(path);
				ws += L" is not found";
				return StreamResourceHandler::CreateError(404, ws.c_str());
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
	GetMime(path, mime);
	CefResponse::HeaderMap hm;
	hm.insert(std::make_pair(L"Content-Length", CefString(std::to_wstring(sz.QuadPart))));
	SetDefaultHeaders(hm);
	return StreamResourceHandler::CreateFromFile(200, mime, hm, file);
}

bool AssetsFromLocalFile::SetBasePath(const CefString& base_path) {
	WCHAR tmp[MAX_PATH + 16] = {};
	DWORD len = MAX_PATH + 16;
	HRESULT hr = UrlCreateFromPathW(reinterpret_cast<PCWSTR>(base_path.c_str()), tmp, &len, 0);
	if (FAILED(hr)) {
		report(hr, L"UrlCreateFromPathW failed");
		return false;
	}
	// drop last slash
	if (tmp[len - 1] == L'/') {
		tmp[len - 1] = L'\0';
	}
	base_path_ = tmp;
	return true;
}

size_t AssetsFromZip::read(void* pOpaque, mz_uint64 file_ofs, void* pBuf, size_t n) {
	HANDLE h = pOpaque;
	LARGE_INTEGER l = {};
	l.QuadPart = file_ofs;
	DWORD r = SetFilePointer(h, l.LowPart, &l.HighPart, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER) {
		DWORD err = GetLastError();
		if (err != NO_ERROR) {
			report(HRESULT_FROM_WIN32(err), L"SetFilePointer failed");
			return 0;
		}
	}
	DWORD read = 0;
	if (!ReadFile(h, pBuf, static_cast<DWORD>(n), &read, NULL)) {
		report(HRESULT_FROM_WIN32(GetLastError()), L"ReadFile failed");
		return 0;
	}
	return static_cast<size_t>(read);
}

bool AssetsFromZip::SetSourceZip(const CefString& zip_path) {
	LARGE_INTEGER sz = {};
	file_ = CreateFileW(reinterpret_cast<PCWSTR>(zip_path.c_str()), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file_ == INVALID_HANDLE_VALUE) {
		report(HRESULT_FROM_WIN32(GetLastError()), L"CreateFile failed");
		return false;
	}
	if (!GetFileSizeEx(file_, &sz)) {
		report(HRESULT_FROM_WIN32(GetLastError()), L"GetFileSizeEx failed");
		goto close_file;
	}
	archive_.m_pRead = &AssetsFromZip::read;
	archive_.m_pIO_opaque = file_;
	if (!mz_zip_reader_init(&archive_, sz.QuadPart, 0)) {
		goto close_file;
	}
	return true;

close_file:
	CloseHandle(file_);
	file_ = INVALID_HANDLE_VALUE;
	return false;
}

AssetsFromZip::~AssetsFromZip() {
	if (archive_.m_zip_mode == MZ_ZIP_MODE_READING) {
		mz_zip_reader_end(&archive_);
	}
	if (file_ != INVALID_HANDLE_VALUE) {
		CloseHandle(file_);
		file_ = INVALID_HANDLE_VALUE;
	}
}

CefRefPtr<CefResourceHandler> AssetsFromZip::Get(const CefString& path) {
	if (archive_.m_zip_mode != MZ_ZIP_MODE_READING) {
		return StreamResourceHandler::CreateError(500, L"target zip file is not ready");
	}
	std::string u8path;
	{
		const HRESULT hr = to_u8(reinterpret_cast<PCWSTR>(path.c_str()), static_cast<const int>(path.size()), u8path);
		if (FAILED(hr)) {
			return StreamResourceHandler::CreateError(500, L"failed to encode filename to utf-8");
		}
	}
	// trim first slash
	u8path = u8path.substr(1);
	const int r = mz_zip_reader_locate_file(&archive_, u8path.c_str(), nullptr, MZ_ZIP_FLAG_CASE_SENSITIVE);
	if (r == -1) {
		return StreamResourceHandler::CreateError(404, L"file not found from zip");
	}
	mz_zip_archive_file_stat fs = {};
	if (!mz_zip_reader_file_stat(&archive_, r, &fs)) {
		return StreamResourceHandler::CreateError(500, L"failed to get file size from zip");
	}
	std::string content;
	content.resize(fs.m_uncomp_size);
	if (!mz_zip_reader_extract_to_mem(&archive_, r, &content[0], content.size(), 0)) {
		return StreamResourceHandler::CreateError(500, L"failed to extract file from zip");
	}
	CefString mime;
	GetMime(path, mime);
	CefResponse::HeaderMap hm;
	hm.insert(std::make_pair(L"Content-Length", CefString(std::to_wstring(content.size()))));
	SetDefaultHeaders(hm);
	return StreamResourceHandler::CreateFromString(200, mime, hm, content);
}