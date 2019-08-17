#include "app.h"

#include <string>

#include "include/base/cef_bind.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_closure_task.h"

#include "callback.h"

App::App() : initialized_(false) {
}

void App::WaitInitialize() {
	std::unique_lock<std::mutex> lk(mtx_);
	cv_.wait(lk, [&] {
		return initialized_ == true;
	});
}

bool App::RenderAndCapture(
    const std::wstring& path,
    const bool use_dir,
    const bool use_devtools,
    const std::wstring& userfile,
    const std::wstring& inbuf,
    std::wstring& outbuf,
    void* image, const int width, const int height) {
	std::lock_guard<std::mutex> lk(mtx_);
    CefRefPtr<Client> c;
    const auto key = client_key{ path, use_dir };
    const auto it = clients_.find(key);
    if (it != clients_.cend()) {
        c = it->second;
    } else {
        c = new Client(InitAssets(path, use_dir));
        clients_[key] = c;
        CefWindowInfo window_info;
        window_info.SetAsWindowless(nullptr);
        CefBrowserSettings browser_settings;
        browser_settings.background_color = 0;
        CefString utf8(L"UTF-8");
        cef_string_utf16_set(utf8.c_str(), utf8.length(), &browser_settings.default_encoding, false);
        browser_settings.javascript_access_clipboard = STATE_DISABLED;
        browser_settings.javascript_dom_paste = STATE_DISABLED;
        CefBrowserHost::CreateBrowser(
            window_info,
            c,
            L"https://assets.example/",
            browser_settings,
            nullptr,
            nullptr
        );
        c->Wait();
    }
    c->SetUseDevTools(use_devtools);
    c->SetUserFile(userfile);
    return c->RenderAndCapture(inbuf, outbuf, image, width, height);
}

void App::CloseAllBrowsers() {
	std::lock_guard<std::mutex> lk(mtx_);
    for (auto &c : clients_) {
        c.second->CloseAllBrowsers(true);
    }
    clients_.clear();
	CefPostTask(TID_UI, base::Bind(&CefQuitMessageLoop));
}

void App::OnContextInitialized() {
	CEF_REQUIRE_UI_THREAD();
    initialized_ = true;
    cv_.notify_all();
}

void App::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) {
    CefRefPtr<CefV8Value> ret;
    CefRefPtr<CefV8Exception> err;
    context->Eval(LR"JS(window.AviUtlBrowser = AviUtlBrowserExtension)JS", frame->GetURL(), 0, ret, err);
}

void App::OnWebKitInitialized() {
    CefString extensionCode(LR"JS(
const AviUtlBrowserExtension = (()=>{
  "use strict";
  native function rendered();
  let renderer = null;
  let ackElem = null;
  let curAck = 0;
  function updateAck() {
    if (document.body && !ackElem) {
      ackElem = document.createElement('div');
      ackElem.style.display = 'block';
      ackElem.style.position = 'fixed';
      ackElem.style.zIndex = '2147483647';
      ackElem.style.left = '0';
      ackElem.style.top = '0';
      ackElem.style.width = '1px';
      ackElem.style.height = '1px';
    }
    if (!ackElem) {
      return 0;
    }
    if (!ackElem.parentNode) {
      document.body.appendChild(ackElem);
    }
    curAck >>= 8;
    if (!curAck) {
      curAck = 0xff0000;
    }
    ackElem.style.backgroundColor = `rgb(${(curAck >>> 16) & 0xff}, ${(curAck >>> 8) & 0xff}, ${(curAck >>> 0) & 0xff})`;
    return curAck;
  }
  return {
    render(idx, param, userfile) {
      if (renderer) {
        renderer({param, userfile}).then(param => {
          rendered(idx, updateAck(), true, param);
        }).catch(err => {
          rendered(idx, updateAck(), false, err);
        });
      } else {
        updateAck();
        rendered(idx, updateAck(), false, "renderer is not registered");
      }
    },
    registerRenderer(fn) {
      renderer = fn;
    },
  };
})();
)JS");
	CefRefPtr<CefV8Handler> h(new CallbackHandler());
	CefRegisterExtension(L"v8/AviUtlBrowserExtension", extensionCode, h);
}
