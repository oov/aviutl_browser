#pragma once

#include <list>
#include <mutex>
#include <condition_variable>

#include "include/wrapper/cef_stream_resource_handler.h"
#include "include/cef_client.h"

#include "assets.h"

class PopupClient : public CefClient, public CefLifeSpanHandler {
public:
	std::function<void(CefRefPtr<CefBrowser> browser)> after_created;
	std::function<void(CefRefPtr<CefBrowser> browser)> before_close;
	std::function<void(CefRefPtr<CefBrowser> browser)> close;
	bool do_not_close;
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override { after_created(browser); }
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override { before_close(browser); }
	virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& target_url,
		const CefString& target_frame_name,
		WindowOpenDisposition target_disposition,
		bool user_gesture,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings,
		CefRefPtr<CefDictionaryValue>& extra_info,
		bool* no_javascript_access) override {
		return true;
	}
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) { return do_not_close; }
	IMPLEMENT_REFCOUNTING(PopupClient);
};

class Client : public CefClient,
	public CefDisplayHandler,
	public CefLifeSpanHandler,
	public CefLoadHandler,
	public CefRenderHandler,
	public CefRequestHandler,
	public CefResourceRequestHandler {
public:
	explicit Client(CefRefPtr<Assets> assets);
	~Client();

	// CefClient methods:
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefProcessId source_process,
		CefRefPtr<CefProcessMessage> message) override;

	// CefDisplayHandler methods:
	virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
		cef_log_severity_t level,
		const CefString& message,
		const CefString& source,
		int line) override;

	// CefLifeSpanHandler methods:
	virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& target_url,
		const CefString& target_frame_name,
		CefLifeSpanHandler::WindowOpenDisposition target_disposition,
		bool user_gesture,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings,
		CefRefPtr<CefDictionaryValue>& extra_info,
		bool* no_javascript_access) override;
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

	// CefLoadHandler methods:
	virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		ErrorCode errorCode,
		const CefString& errorText,
		const CefString& failedUrl) override;
	virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
		bool isLoading,
		bool canGoBack,
		bool canGoForward) override;

	// CefRenderHandler methods:
	virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
	virtual void OnPaint(CefRefPtr<CefBrowser> browser,
		PaintElementType type,
		const RectList& dirtyRects,
		const void* buffer,
		int width,
		int height) override;

	// CefRequestHandler methods:
	virtual CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		bool is_navigation,
		bool is_download,
		const CefString& request_initiator,
		bool& disable_default_handling) override {
		return this;
	}

	// CefResourceRequestHandler methods:
	virtual CefRefPtr<CefResourceHandler> GetResourceHandler(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request) override;

	void CloseAllBrowsers(bool force_close);

	void Wait();
	void SetUserFile(const std::wstring& userfile);
	void SetUseDevTools(const bool use_devtools);
	bool RenderAndCapture(const std::wstring& inbuf, std::wstring& outbuf, void* image, const int width, const int height);
private:
	void Resize(const int width, const int height);
	bool Render(const std::wstring& inbuf, std::wstring& outbuf, int timeout);
	void Capture(void* buf, const int width, const int height);
	void InitializeDevTools(CefRefPtr<CefBrowser> browser);
	void GetUserFileHash(std::wstring& hash);
	CefRefPtr<CefResourceHandler> GetUserFile();

	typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
	BrowserList browser_list_;

	std::wstring userfile_, userfile_hash_;
	CefRefPtr<PopupClient> devtools_;
	CefRefPtr<Assets> assets_;

	bool received_result_ok_;
	CefString received_result_param_;
	uint32_t received_result_ack_;

	bool resized_, rendered_, loaded_;
	int capture_state_;
	int callback_sequence_, waiting_sequence_;
	void* buf_;
	int cur_width_, cur_height_;
	LONG loading_;

	std::mutex mtx_;
	std::condition_variable cv_;

	IMPLEMENT_REFCOUNTING(Client);
};
