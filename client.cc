#include "client.h"

#include <sstream>
#include <string>
#include <chrono>

#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/cef_app.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_parser.h"

#include "depremultipliedalpha_table.h"

Client::Client(CefRefPtr<Assets> assets)
	: buf_(nullptr),
	resized_(true),
	loaded_(false),
	rendered_(true),
	capture_state_(0),
	cur_width_(1),
	cur_height_(1),
	callback_sequence_(0),
	waiting_sequence_(0),
	loading_(0),
	received_result_ok_(false),
	received_result_param_(""),
	received_result_ack_(0),
	assets_(assets)
{
}

Client::~Client() {
}

bool Client::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefProcessId source_process,
	CefRefPtr<CefProcessMessage> message) {
	const std::string& message_name = message->GetName();
	if (message_name == "rendered") {
		{
			std::lock_guard<std::mutex> lk(mtx_);
			auto a = message->GetArgumentList();
			if (a->GetInt(0) == waiting_sequence_) {
				rendered_ = true;
				received_result_ack_ = a->GetInt(1);
				received_result_ok_ = a->GetBool(2);
				received_result_param_ = a->GetString(3);
				cv_.notify_all();
			}
		}
		return true;
	}
	return false;
}

bool Client::OnBeforePopup(CefRefPtr<CefBrowser> browser,
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
	bool* no_javascript_access) {
	return true;
}

void Client::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();
	browser_list_.push_back(browser);
}

bool Client::DoClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();
	return false;
}

void Client::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();
	for (BrowserList::iterator bit = browser_list_.begin(); bit != browser_list_.end(); ++bit) {
		if ((*bit)->IsSame(browser)) {
			browser_list_.erase(bit);
			break;
		}
	}
}

void Client::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
	bool isLoading,
	bool canGoBack,
	bool canGoForward) {
	if (isLoading) {
		InterlockedIncrement(&loading_);
	}
	else {
		if (InterlockedDecrement(&loading_) == 0) {
			std::lock_guard<std::mutex> lk(mtx_);
			loaded_ = true;
			cv_.notify_all();
		}
	}
}

std::string GetDataURI(const std::string& data, const std::string& mime_type) {
	return "data:" + mime_type + ";base64," + CefURIEncode(CefBase64Encode(data.data(), data.size()), false).ToString();
}

void Client::OnLoadError(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	ErrorCode errorCode,
	const CefString& errorText,
	const CefString& failedUrl) {
	CEF_REQUIRE_UI_THREAD();
	if (errorCode == ERR_ABORTED) {
		return;
	}
	std::stringstream ss;
	ss << "<html><body bgcolor=\"white\">"
		"<h2>Failed to load URL "
		<< std::string(failedUrl) << " with error " << std::string(errorText)
		<< " (" << errorCode << ").</h2></body></html>";
	frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

void Client::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
	std::lock_guard<std::mutex> lk(mtx_);
	rect.Set(0, 0, cur_width_, cur_height_);
	resized_ = true;
	cv_.notify_all();
}

void Client::OnPaint(CefRefPtr<CefBrowser> browser,
	PaintElementType type,
	const RectList& dirtyRects,
	const void* buffer,
	int width,
	int height) {
	if (type != PET_VIEW || width == 0 || height == 0 || cur_width_ != width || cur_height_ != height) {
		return;
	}

	std::lock_guard<std::mutex> lk(mtx_);
	if (capture_state_ != 0) {
		return;
	}
	if (!buf_) {
		capture_state_ = 3;
		cv_.notify_all();
		return;
	}
	uint32_t ack = received_result_ack_, pixel = *((uint32_t*)buffer);
	if (ack) {
		if (
			(pixel & 0xff000000) != 0xff000000 ||
			(
				(ack & 0x00ff0000 && ((pixel >> 16) & 0xff) <= 0x7f) ||
				(!(ack & 0x00ff0000) && ((pixel >> 16) & 0xff) > 0x7f)
				) ||
			(
				(ack & 0x0000ff00 && ((pixel >> 8) & 0xff) <= 0x7f) ||
				(!(ack & 0x0000ff00) && ((pixel >> 8) & 0xff) > 0x7f)
				) ||
			(
				(ack & 0x000000ff && (pixel & 0xff) <= 0x7f) ||
				(!(ack & 0x000000ff) && (pixel & 0xff) > 0x7f)
			)
		) {
			capture_state_ = 2;
			cv_.notify_all();
			return;
		}
	}

	int sw = width * 4, sh = height;
	const uint8_t* sl = (const uint8_t*)buffer;
	int dw = cur_width_ * 4, dh = cur_height_;
	uint8_t* dl = (uint8_t*)buf_;
	int w = std::min(sw, dw), h = std::min(sh, dh);
	for (int y = 0; y < h; ++y, sl += sw, dl += dw) {
		for (int x = 0; x < w; x += 4) {
//			const uint_fast16_t a = sl[x + 3];
			const uint_fast16_t a = (uint_fast16_t)(sl[x + 3]) << 8;
			if (a > 0) {
				dl[x + 0] = dePremultipliedAlphaTable[a + (uint_fast16_t)(sl[x + 0])];
				dl[x + 1] = dePremultipliedAlphaTable[a + (uint_fast16_t)(sl[x + 1])];
				dl[x + 2] = dePremultipliedAlphaTable[a + (uint_fast16_t)(sl[x + 2])];
/*
				dl[x + 0] = (uint8_t)((uint_fast16_t)(sl[x + 0]) * 255 / a);
				dl[x + 1] = (uint8_t)((uint_fast16_t)(sl[x + 1]) * 255 / a);
				dl[x + 2] = (uint8_t)((uint_fast16_t)(sl[x + 2]) * 255 / a);
*/
			}
			dl[x + 3] = sl[x + 3];
		}
	}
	dl = (uint8_t*)buf_;
	dl[0] = 0;
	dl[1] = 0;
	dl[2] = 0;
	dl[3] = 0;
	capture_state_ = 1;
	cv_.notify_all();
}

CefRefPtr<CefResourceHandler> Client::GetResourceHandler(
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefRequest> request) {
	CefURLParts u;
	if (!CefParseURL(request->GetURL(), u)) {
		return NULL;
	}
	if (CefString(&u.scheme) == L"https" && CefString(&u.host) == L"assets.example") {
		auto path = CefString(&u.path);
		if (path == L"/userfile") {
			if (userfile_.empty()) {
				return StreamResourceHandler::CreateError(404, L"file is not provided");
			}
			return GetUserFile();
		}
		return (*assets_)(path);
	}
	return NULL;
}

void Client::Capture(void* b, const int width, const int height) {
	std::unique_lock<std::mutex> lk(mtx_);
	buf_ = b;
	do {
		capture_state_ = 0;
		cv_.wait(lk, [&] {
			return capture_state_ != 0;
		});
	} while (capture_state_ == 2);
}

void Client::Wait() {
	std::unique_lock<std::mutex> lk(mtx_);
	cv_.wait(lk, [&] {
		return loaded_ == true;
	});
}

void Client::Resize(const int width, const int height) {
	std::unique_lock<std::mutex> lk(mtx_);
	if (cur_width_ == width && cur_height_ == height) {
		return;
	}
	resized_ = false;
	capture_state_ = 0;
	cur_width_ = width;
	cur_height_ = height;
	buf_ = nullptr;

	CefPostTask(TID_UI, base::Bind(&CefBrowserHost::WasResized, browser_list_.front()->GetHost()));
	cv_.wait(lk, [&] {
		return resized_ == true && capture_state_ == 3;
	});
}

static uint64_t cyrb64(const uint32_t* src, const size_t len, const uint32_t seed)
{
	uint32_t h1 = 0x91eb9dc7 ^ seed, h2 = 0x41c6ce57 ^ seed;
	for (size_t i = 0; i < len; ++i)
	{
		h1 = (h1 ^ src[i]) * 2654435761;
		h2 = (h2 ^ src[i]) * 1597334677;
	}
	h1 = ((h1 ^ (h1 >> 16)) * 2246822507) ^ ((h2 ^ (h2 >> 13)) * 3266489909);
	h2 = ((h2 ^ (h2 >> 16)) * 2246822507) ^ ((h1 ^ (h1 >> 13)) * 3266489909);
	return (((uint64_t)h2) << 32) | ((uint64_t)h1);
}

static void to_hex(wchar_t* dst, uint64_t x)
{
	const char* chars = "0123456789abcdef";
	for (int i = 15; i >= 0; --i)
	{
		dst[i] = chars[x & 0xf];
		x >>= 4;
	}
}

void Client::SetUserFile(const std::wstring& userfile) {
	std::unique_lock<std::mutex> lk(mtx_);
	if (userfile_ == userfile) {
		return;
	}
	userfile_ = userfile;
	if (userfile_.empty()) {
		userfile_hash_.clear();
		return;
	}
	std::string buf;
	const size_t l = userfile_.size() * sizeof(wchar_t);
	buf.resize((l + 3) & ~3);
	memcpy(&buf[0], &userfile_[0], l);
	userfile_hash_.resize(16);
	to_hex(&userfile_hash_[0], cyrb64(reinterpret_cast<uint32_t*>(&buf[0]), buf.size() / sizeof(uint32_t), 0x15544521));
}

void Client::SetUseDevTools(const bool use_devtools) {
	std::unique_lock<std::mutex> lk(mtx_);
	if ((use_devtools && devtools_) || (!use_devtools && !devtools_)) {
		return;
	}
	if (!CefCurrentlyOn(TID_UI)) {
		CefPostTask(TID_UI, base::Bind(&Client::SetUseDevTools, this, use_devtools));
		return;
	}
	CefRefPtr<CefBrowser> browser = browser_list_.front();
	if (use_devtools) {
		InitializeDevTools(browser);
	}
	else {
		devtools_->do_not_close = false;
		browser->GetHost()->CloseDevTools();
		devtools_ = nullptr;
	}
}

bool Client::RenderAndCapture(const std::wstring& inbuf, std::wstring& outbuf, void* image, const int width, const int height) {
	Resize(width, height);
	const bool r = Render(inbuf, outbuf, 30000);
	Capture(image, width, height);
	return true;
}

std::wstring buildCallbackCallCode(const std::wstring& param, const std::wstring& userfile_hash, const int sequence) {
	std::wostringstream o;
	o << L"AviUtlBrowser.render(";
	o << sequence;
	o << L",\"";
	for (auto c = param.cbegin(); c != param.cend(); c++) {
		switch (*c) {
		case L'\x00': o << L"\\x00"; break;
		case L'\x01': o << L"\\x01"; break;
		case L'\x02': o << L"\\x02"; break;
		case L'\x03': o << L"\\x03"; break;
		case L'\x04': o << L"\\x04"; break;
		case L'\x05': o << L"\\x05"; break;
		case L'\x06': o << L"\\x06"; break;
		case L'\x07': o << L"\\x07"; break;
		case L'\x08': o << L"\\b"; break;
		case L'\x09': o << L"\\t"; break;
		case L'\x0a': o << L"\\n"; break;
		case L'\x0b': o << L"\\x0b"; break;
		case L'\x0c': o << L"\\f"; break;
		case L'\x0d': o << L"\\r"; break;
		case L'\x0e': o << L"\\x0e"; break;
		case L'\x0f': o << L"\\x0f"; break;
		case L'\x10': o << L"\\x10"; break;
		case L'\x11': o << L"\\x11"; break;
		case L'\x12': o << L"\\x12"; break;
		case L'\x13': o << L"\\x13"; break;
		case L'\x14': o << L"\\x14"; break;
		case L'\x15': o << L"\\x15"; break;
		case L'\x16': o << L"\\x16"; break;
		case L'\x17': o << L"\\x17"; break;
		case L'\x18': o << L"\\x18"; break;
		case L'\x19': o << L"\\x19"; break;
		case L'\x1a': o << L"\\x1a"; break;
		case L'\x1b': o << L"\\x1b"; break;
		case L'\x1c': o << L"\\x1c"; break;
		case L'\x1d': o << L"\\x1d"; break;
		case L'\x1e': o << L"\\x1e"; break;
		case L'\x1f': o << L"\\x1f"; break;
		case L'\x22': o << L"\\\""; break;
		case L'\x5c': o << L"\\\\"; break;
		default: o << *c;
		}
	}
	o << L"\",";
	if (userfile_hash.empty()) {
		o << L"null";
	}
	else {
		o << L"\"";
		o << userfile_hash;
		o << L"\"";
	}
	o << L")";
	return o.str();
}

bool Client::Render(const std::wstring& inbuf, std::wstring& outbuf, int timeout) {
	std::unique_lock<std::mutex> lk(mtx_);
	rendered_ = false;
	waiting_sequence_ = ++callback_sequence_;
	{
		auto f = browser_list_.front()->GetMainFrame();
		f->ExecuteJavaScript(buildCallbackCallCode(inbuf, userfile_hash_, waiting_sequence_), f->GetURL(), 0);
	}
	if (!cv_.wait_for(lk, std::chrono::milliseconds(timeout), [&] {
		return rendered_;
	})) {
		outbuf = L"timeout";
		return false;
	}
	outbuf = received_result_param_;
	return received_result_ok_;
}

void Client::CloseAllBrowsers(bool force_close) {
	if (!CefCurrentlyOn(TID_UI)) {
		CefPostTask(TID_UI, base::Bind(&Client::CloseAllBrowsers, this,
			force_close));
		return;
	}

	std::unique_lock<std::mutex> lk(mtx_);
	for (auto it = browser_list_.cbegin(); it != browser_list_.cend(); ++it) {
		(*it)->GetHost()->CloseBrowser(force_close);
	}
}
