#pragma once

#include <functional>
#include <map>
#include <mutex>

#include "include/wrapper/cef_stream_resource_handler.h"

typedef std::function<CefRefPtr<CefResourceHandler>(const CefString&)> AssetsFunct;

class Assets : public virtual CefBaseRefCounted {
	typedef std::map<CefString, CefString> cefstrmap;
	cefstrmap mime_map_;
	bool mime_map_initialized;
	std::mutex mtx_;
	void InitializeMimeMap();
protected:
	virtual CefRefPtr<CefResourceHandler> Get(const CefString& path) = 0;
	virtual void GetMimeJson(CefString& json);
public:
	explicit Assets();
	void GetMime(const CefString& path, CefString& mime);
	void SetDefaultHeaders(CefResponse::HeaderMap& header_map);
	CefRefPtr<CefResourceHandler> operator()(const CefString& path);
	IMPLEMENT_REFCOUNTING(Assets);
};

class StreamResourceHandler : public CefStreamResourceHandler
{
private:
	std::string content;
	explicit StreamResourceHandler(
		int status_code,
		const CefString& status_text,
		const CefString& mime_type,
		CefResponse::HeaderMap header_map,
		CefRefPtr<CefStreamReader> stream) : CefStreamResourceHandler(status_code, status_text, mime_type, header_map, stream) {};
public:
	static CefRefPtr<CefResourceHandler> CreateFromString(
		int status_code,
		const CefString& mime_type,
		CefResponse::HeaderMap header_map,
		std::string& content);
	static CefRefPtr<CefResourceHandler> CreateFromFile(
		int status_code,
		const CefString& mime_type,
		CefResponse::HeaderMap header_map,
		const CefString& filepath);
	static CefRefPtr<CefResourceHandler> CreateError(
		int status_code,
		const CefString& message);
};


class StaticErrorAssets : public Assets {
private:
	const int status_code_;
	const CefString message_;
protected:
	virtual CefRefPtr<CefResourceHandler> Get(const CefString& path) {
		return StreamResourceHandler::CreateError(status_code_, message_);
	}
public:
	explicit StaticErrorAssets(int status_code, const CefString& message) : status_code_(status_code), message_(message) {}
};