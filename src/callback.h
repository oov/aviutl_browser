#pragma once

#include "include/cef_app.h"

class CallbackHandler : public CefV8Handler {
public:
	CallbackHandler() {}

	virtual bool Execute(const CefString& name,
		CefRefPtr<CefV8Value> object,
		const CefV8ValueList& arguments,
		CefRefPtr<CefV8Value>& retval,
		CefString& exception) override {
		if (name == "rendered") {
			CefRefPtr<CefProcessMessage> msg(CefProcessMessage::Create("rendered"));
			auto a = msg->GetArgumentList();
			a->SetInt(0, arguments[0]->GetIntValue());
			a->SetInt(1, arguments[1]->GetIntValue());
			a->SetBool(2, arguments[2]->GetBoolValue());
			a->SetString(3, arguments[3]->GetStringValue());
			CefV8Context::GetCurrentContext()->GetBrowser()->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);
			return true;
		}
		return false;
	}

	IMPLEMENT_REFCOUNTING(CallbackHandler);
};
