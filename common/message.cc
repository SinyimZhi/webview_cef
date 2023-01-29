#include "message.h"
#include "flutter/encodable_value.h"
#include <iostream>

namespace
{
    const std::string keyID = "_id_";
    const std::string keyResult = "_result_";
    const std::string keyError = "_error_";

    bool GetInt(const flutter::EncodableMap* m, const std::string key, int* v) {
        auto it = m->find(key);
        if (it != m->end()) {
            *v = std::get<std::int32_t>(it->second);
            return true;
        }

        return false;
    }

    // Returns 0 if id not found or invalid.
    int GetMessageID(const flutter::EncodableMap* m) {
        int id = 0;
        if (GetInt(m, keyID, &id)) {
            return id;
        }

        return 0;
    }

    CefString GetCefString(const flutter::EncodableMap* m, const std::string key) {
        CefString ret;

        auto it = m->find(key);
        if (it != m->end()) {
            auto s = std::get<std::string>(it->second);
            ret = CefString(s);
        }

        return ret;
    }
}

namespace async_channel_message
{

CefRefPtr<CefProcessMessage> EvaluateJavaScript::CreateCefProcessMessage(const flutter::EncodableValue* v) {
    const flutter::EncodableMap* m =
        std::get_if<flutter::EncodableMap>(v);
    if (!m) return nullptr;

    auto message_id = GetMessageID(m);
    if (message_id == 0) return nullptr;

    auto code = GetCefString(m, "code");
    if (code.empty()) return nullptr;

    auto msg = CefProcessMessage::Create(ipc::EvaluateJavaScriptRequest);
    auto args = msg->GetArgumentList();
    args->SetInt(0, message_id);
    args->SetString(1, code);

    return msg;
}

flutter::EncodableValue EvaluateJavaScript::CreateFlutterChannelMessage(
    CefRefPtr<CefProcessMessage> cpm) {

    auto args = cpm->GetArgumentList();
    auto message_id = args->GetInt(ipc::indexID);

    auto m = flutter::EncodableMap{
        {flutter::EncodableValue(keyID), flutter::EncodableValue(message_id)},
    };

    auto success = args->GetBool(ipc::indexSuccessFlag);
    const auto err_or_result_flag = ipc::indexCustom;
    if (success) {
        m.insert({
            flutter::EncodableValue(keyResult),
            flutter::EncodableValue(args->GetString(err_or_result_flag).ToString()),
        });
    } else {
        auto error_msg = args->GetString(err_or_result_flag);

        m.insert({
            flutter::EncodableValue(keyError),
            flutter::EncodableValue(error_msg.ToString()),
        });
        if (error_msg == EvaluateErrorMessage) {
            m.insert({
                flutter::EncodableValue("message"),
                flutter::EncodableValue(args->GetString(indexEvalError).ToString()),
            });
            m.insert({
                flutter::EncodableValue("file"),
                flutter::EncodableValue(args->GetString(indexScriptResourceName).ToString()),
            });
            m.insert({
                flutter::EncodableValue("sourceLine"),
                flutter::EncodableValue(args->GetString(indexSourceLine).ToString()),
            });
            m.insert({
                flutter::EncodableValue("line"),
                flutter::EncodableValue(args->GetInt(indexLineNumber)),
            });
            m.insert({
                flutter::EncodableValue("column"),
                flutter::EncodableValue(args->GetInt(indexStartColumn)),
            });
        }
    }

    return flutter::EncodableValue(m);
}

} // namespace async_channel_message
