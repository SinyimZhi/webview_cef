#ifndef COMMON_MESSAGE_H_
#define COMMON_MESSAGE_H_
#pragma once

#include "include/cef_base.h"
#include "include/cef_process_message.h"
#include <flutter/standard_method_codec.h>

namespace ipc
{
    const CefString EvaluateJavaScriptRequest = "EvaluateJavaScriptRequest";
    const CefString EvaluateJavaScriptResponse = "EvaluateJavaScriptResponse";

    // CefProcessMessage argument position
    const size_t indexID = 0; // message id
    const size_t indexSuccessFlag = 1;
    const size_t indexCustom = 2;
}

namespace async_channel_message
{

class EvaluateJavaScript {
public:
    static inline const CefString EvaluateErrorMessage = "Evaluate Error";
    static const size_t indexEvalError = ipc::indexCustom + 1;
    static const size_t indexScriptResourceName = ipc::indexCustom + 2;
	static const size_t indexSourceLine = ipc::indexCustom + 3;
	static const size_t indexLineNumber = ipc::indexCustom + 4;
	static const size_t indexStartColumn = ipc::indexCustom + 5;

    static CefRefPtr<CefProcessMessage> CreateCefProcessMessage(const flutter::EncodableValue* v);
    static flutter::EncodableValue CreateFlutterChannelMessage(CefRefPtr<CefProcessMessage> cpm);
};

} // namespace async_channel_message

#endif  // COMMON_MESSAGE_H_
