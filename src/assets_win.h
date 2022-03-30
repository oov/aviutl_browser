#pragma once

#include <Windows.h>

#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"
#undef MINIZ_HEADER_FILE_ONLY

#include "assets.h"

class AssetsFromLocalFile : public virtual Assets {
	CefString base_path_;
protected:
	virtual CefRefPtr<CefResourceHandler> Get(const CefString& path) override;
public:
	bool SetBasePath(const CefString& base_path);
};

class AssetsFromZip : public virtual Assets {
	HANDLE file_;
	mz_zip_archive archive_;
	static size_t read(void* pOpaque, mz_uint64 file_ofs, void* pBuf, size_t n);
protected:
	virtual CefRefPtr<CefResourceHandler> Get(const CefString& path) override;
public:
	explicit AssetsFromZip() : file_(INVALID_HANDLE_VALUE), archive_({}) {}
	virtual ~AssetsFromZip() override;
	bool SetSourceZip(const CefString& zip_path);
};
