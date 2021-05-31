#pragma once

#include <map>
#include <mutex>
#include <condition_variable>

#include "include/cef_app.h"

#include "client.h"

struct client_key {
	std::wstring path;
	std::wstring tabid;
	bool is_dir;
	bool operator<(const client_key& value) const
	{
		return std::tie(path, is_dir, value.tabid) < std::tie(value.path, value.is_dir, value.tabid);
	}
};

class App : public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler {
public:
	explicit App();

	// CefApp methods:
	virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
		return this;
	}
	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
		return this;
	}

	// CefBrowserProcessHandler methods:
	virtual void OnContextInitialized() override;

	// CefRenderProcessHandler methods:
	virtual void OnWebKitInitialized() override;
	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;

	void WaitInitialize();
	bool RenderAndCapture(
		const std::wstring& path,
		const std::wstring& tabid,
		const bool use_dir,
		const bool use_devtools,
		const std::wstring& userfile,
		const std::wstring& inbuf,
		std::wstring& outbuf,
		void* image, const int width, const int height
	);
	void CloseAllBrowsers();
private:
	std::mutex mtx_;
	std::condition_variable cv_;
	bool initialized_;

	std::map<client_key, CefRefPtr<Client>> clients_;

	CefRefPtr<Assets> InitAssets(const std::wstring& path, const bool use_dir);

	IMPLEMENT_REFCOUNTING(App);
};
