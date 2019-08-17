#pragma once

#include <string>
#include <Windows.h>

static inline HRESULT to_u16(LPCSTR src, const int srclen, std::wstring& dest) {
	if (srclen == 0) {
		dest.resize(0);
		return S_OK;
	}
	const int destlen = MultiByteToWideChar(CP_UTF8, 0, src, srclen, nullptr, 0);
	if (destlen == 0) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
	dest.resize(destlen);
	if (MultiByteToWideChar(CP_UTF8, 0, src, srclen, &dest[0], destlen) == 0) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
	if (srclen == -1) {
		dest.resize(static_cast<size_t>(destlen) - 1);
	}
	return S_OK;
}

static inline HRESULT to_u8(LPCWSTR src, const int srclen, std::string& dest) {
	if (srclen == 0) {
		dest.resize(0);
		return S_OK;
	}
	const int destlen = WideCharToMultiByte(CP_UTF8, 0, src, srclen, nullptr, 0, nullptr, nullptr);
	if (destlen == 0) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
	dest.resize(destlen);
	if (WideCharToMultiByte(CP_UTF8, 0, src, srclen, &dest[0], destlen, nullptr, nullptr) == 0) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
	if (srclen == -1) {
		dest.resize(static_cast<size_t>(destlen) - 1);
	}
	return S_OK;
}

static inline HRESULT to_sjis(LPCWSTR src, const int srclen, std::string& dest) {
	if (srclen == 0) {
		dest.resize(0);
		return S_OK;
	}
	const int destlen = WideCharToMultiByte(932, 0, src, srclen, nullptr, 0, nullptr, nullptr);
	if (destlen == 0) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
	dest.resize(destlen);
	if (WideCharToMultiByte(932, 0, src, srclen, &dest[0], destlen, nullptr, nullptr) == 0) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
	if (srclen == -1) {
		dest.resize(static_cast<size_t>(destlen) - 1);
	}
	return S_OK;
}

static inline HRESULT from_sjis(LPCSTR src, const int srclen, std::wstring& dest) {
	if (srclen == 0) {
		dest.resize(0);
		return S_OK;
	}
	const int destlen = MultiByteToWideChar(932, 0, src, srclen, nullptr, 0);
	if (destlen == 0) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
	dest.resize(destlen);
	if (MultiByteToWideChar(932, 0, src, srclen, &dest[0], destlen) == 0) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
	if (srclen == -1) {
		dest.resize(static_cast<size_t>(destlen) - 1);
	}
	return S_OK;
}
