// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "include/cef_parser.h"
#include "include/base/cef_logging.h"
#include "client_app_renderer.h"
#include "message.h"

using namespace async_channel_message;

namespace
{

CefString CefV8ValueToJSON(CefRefPtr<CefV8Context> v8_context, CefRefPtr<CefV8Value> value) {
    auto json_val = v8_context
        ->GetGlobal()
        ->GetValue("JSON")
        ->GetValue("stringify")
        ->ExecuteFunction(nullptr, CefV8ValueList{value});
    return json_val->GetStringValue();
}

}

ClientAppRenderer::ClientAppRenderer() {}

void ClientAppRenderer::OnWebKitInitialized() {
	// Create the renderer-side router for query handling.
	CefMessageRouterConfig config;
	message_router_ = CefMessageRouterRendererSide::Create(config);
}

// void ClientAppRenderer::OnBrowserCreated(CefRefPtr<CefBrowser> browser,
//     									 CefRefPtr<CefDictionaryValue> extra_info) {}

// void ClientAppRenderer::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) {}

// CefRefPtr<CefLoadHandler> ClientAppRenderer::GetLoadHandler() {}

void ClientAppRenderer::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefRefPtr<CefV8Context> context) {
	message_router_->OnContextCreated(browser, frame, context);
}

void ClientAppRenderer::OnContextReleased(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefV8Context> context) {
	message_router_->OnContextReleased(browser, frame, context);
}

// void ClientAppRenderer::OnUncaughtException(CefRefPtr<CefBrowser> browser,
// 											CefRefPtr<CefFrame> frame,
// 											CefRefPtr<CefV8Context> context,
// 											CefRefPtr<CefV8Exception> exception,
// 											CefRefPtr<CefV8StackTrace> stackTrace) {}

// void ClientAppRenderer::OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
//                                              CefRefPtr<CefFrame> frame,
//                                              CefRefPtr<CefDOMNode> node) {}

bool ClientAppRenderer::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
												 CefRefPtr<CefFrame> frame,
												 CefProcessId source_process,
												 CefRefPtr<CefProcessMessage> message) {
	DCHECK_EQ(source_process, PID_BROWSER);

	const auto message_name = message->GetName();
    if (message_name == ipc::EvaluateJavaScriptRequest) {
		this->evaluateJavaScript(browser, frame, source_process, message);
        return true;
    }

	return this->message_router_->OnProcessMessageReceived(browser, frame,
            source_process, message);
}


void ClientAppRenderer::evaluateJavaScript(CefRefPtr<CefBrowser> browser,
										   CefRefPtr<CefFrame> frame,
										   CefProcessId source_process,
										   CefRefPtr<CefProcessMessage> message) {

    auto const v8_context = frame->GetV8Context();
	auto response_msg = CefProcessMessage::Create(ipc::EvaluateJavaScriptResponse);
	auto response_args = response_msg->GetArgumentList();
	auto args = message->GetArgumentList();
	response_args->SetInt(ipc::indexID, args->GetInt(ipc::indexID));
	response_args->SetBool(ipc::indexSuccessFlag, false);

	constexpr auto err_or_result_flag = ipc::indexCustom + 0;
    if (!v8_context) {
		response_args->SetString(err_or_result_flag, "Unable to get v8 context");
		browser->GetMainFrame()->SendProcessMessage(PID_BROWSER, response_msg);
        return;
    }

    if (!v8_context->Enter()) {
		response_args->SetString(err_or_result_flag, "Unable to enter v8 context");
		browser->GetMainFrame()->SendProcessMessage(PID_BROWSER, response_msg);
        return;
    }

    CefRefPtr<CefV8Exception> exception;
    CefRefPtr<CefV8Value> retval;
	auto code = args->GetString(1);
    auto success = v8_context->Eval(code, frame->GetURL(), 0, retval, exception);
    if (success) {
		response_args->SetBool(ipc::indexSuccessFlag, true);
        if (retval->IsValid()) {
            response_args->SetString(err_or_result_flag, CefV8ValueToJSON(v8_context, retval));
        }
    } else {
		response_args->SetString(err_or_result_flag, EvaluateJavaScript::EvaluateErrorMessage);
		response_args->SetString(EvaluateJavaScript::indexEvalError, exception->GetMessageW());
		response_args->SetString(EvaluateJavaScript::indexScriptResourceName, exception->GetScriptResourceName());
		response_args->SetString(EvaluateJavaScript::indexSourceLine, exception->GetSourceLine());
		response_args->SetInt(EvaluateJavaScript::indexLineNumber, exception->GetLineNumber());
		response_args->SetInt(EvaluateJavaScript::indexStartColumn, exception->GetStartColumn());
    }

    v8_context->Exit();
	browser->GetMainFrame()->SendProcessMessage(PID_BROWSER, response_msg);
}
