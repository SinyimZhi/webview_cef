﻿#include "webview_cef_plugin.h"


#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <thread>

#include "webview_app.h"
#include "texture_handler.h"

namespace webview_cef {
	bool init = false;

	std::unique_ptr<OsrImeHandlerWin> ime_handler = nullptr;

	void OnIMEStartComposition() {
		if (ime_handler) {
			ime_handler->CreateImeWindow();
			ime_handler->MoveImeWindow();
			ime_handler->ResetComposition();
		}
	}

	void OnIMEComposition(UINT message,
                                    WPARAM wParam,
                                    LPARAM lParam) {
		auto browser = WebviewHandler::CurrentFocusedBrowser();
		if (browser && ime_handler) {
			CefString cTextStr;
			if (ime_handler->GetResult(lParam, cTextStr)) {
				// Send the text to the browser. The |replacement_range| and
				// |relative_cursor_pos| params are not used on Windows, so provide
				// default invalid values.
				browser->GetHost()->ImeCommitText(cTextStr,
													CefRange(UINT32_MAX, UINT32_MAX), 0);
				ime_handler->ResetComposition();
				// Continue reading the composition string - Japanese IMEs send both
				// GCS_RESULTSTR and GCS_COMPSTR.
			}

			std::vector<CefCompositionUnderline> underlines;
			int composition_start = 0;

			if (ime_handler->GetComposition(lParam, cTextStr, underlines,
											composition_start)) {
				// Send the composition string to the browser. The |replacement_range|
				// param is not used on Windows, so provide a default invalid value.
				browser->GetHost()->ImeSetComposition(
					cTextStr, underlines, CefRange(UINT32_MAX, UINT32_MAX),
					CefRange(composition_start,
							static_cast<int>(composition_start + cTextStr.length())));

				// Update the Candidate Window position. The cursor is at the end so
				// subtract 1. This is safe because IMM32 does not support non-zero-width
				// in a composition. Also,  negative values are safely ignored in
				// MoveImeWindow
				ime_handler->UpdateCaretPosition(composition_start - 1);
			} else {
				OnIMECancelCompositionEvent();
			}
		}
	}

	void OnIMECancelCompositionEvent() {
		auto browser = WebviewHandler::CurrentFocusedBrowser();
		if (browser && ime_handler) {
			browser->GetHost()->ImeCancelComposition();
			ime_handler->ResetComposition();
			ime_handler->DestroyImeWindow();
		}
	}

	flutter::TextureRegistrar* texture_registrar;
	flutter::BinaryMessenger* messenger;

	CefRefPtr<WebviewApp> app;
	CefMainArgs mainArgs;

	void startCEF() {
		CefWindowInfo window_info;
		CefBrowserSettings settings;
		window_info.SetAsWindowless(nullptr);

		CefSettings cefs;
		cefs.windowless_rendering_enabled = true;
		CefInitialize(mainArgs, cefs, app.get(), nullptr);
		CefRunMessageLoop();
		CefShutdown();
	}

	template <typename T>
	std::optional<T> GetOptionalValue(const flutter::EncodableMap& map,
		const std::string& key) {
		const auto it = map.find(flutter::EncodableValue(key));
		if (it != map.end()) {
			const auto val = std::get_if<T>(&it->second);
			if (val) {
				return *val;
			}
		}
		return std::nullopt;
	}

	// static
	void WebviewCefPlugin::RegisterWithRegistrar(
		flutter::PluginRegistrarWindows* registrar) {
		texture_registrar = registrar->texture_registrar();
		messenger = registrar->messenger();
		auto plugin_channel =
			std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
				messenger, "webview_cef", &flutter::StandardMethodCodec::GetInstance());

		auto plugin = std::make_unique<WebviewCefPlugin>();
		plugin_channel->SetMethodCallHandler(
			[plugin_pointer = plugin.get()](const auto& call, auto result) {
				plugin_pointer->HandleMethodCall(call, std::move(result));
			});
		app = new WebviewApp(std::move(plugin_channel));
		registrar->AddPlugin(std::move(plugin));
	}

	void WebviewCefPlugin::sendKeyEvent(CefKeyEvent ev)
	{
		auto broswer = WebviewHandler::CurrentFocusedBrowser();
		if (broswer) {
			broswer->GetHost()->SendKeyEvent(ev);
		}
	}

	WebviewCefPlugin::WebviewCefPlugin() {}

	WebviewCefPlugin::~WebviewCefPlugin() {}

	void WebviewCefPlugin::HandleMethodCall(
		const flutter::MethodCall<flutter::EncodableValue>& method_call,
		std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
		if (method_call.method_name().compare("startCEF") == 0) {
			if (!init) {
				new std::thread(startCEF);
				init = true;
			}
			result->Success();
		} else if (method_call.method_name().compare("createBrowser") == 0) {
			auto const browser_id = *std::get_if<int>(method_call.arguments());
			auto handler = new WebviewHandler(messenger, browser_id);
			auto texture_handler = new TextureHandler(texture_registrar);
			handler->onPaintCallback = [texture_handler](const void* buffer, int32_t width, int32_t height) {
				texture_handler->onPaintCallback(buffer, width, height);
			};
			handler->onBrowserClose = [texture_handler] () mutable {
				delete texture_handler;
			};
			handler->onImeCompositionRangeChangedCallback = [] (CefRefPtr<CefBrowser> browser,
														const CefRange& selection_range,
														const CefRenderHandler::RectList& character_bounds) {
				if (ime_handler)
					ime_handler->ChangeCompositionRange(selection_range, character_bounds);
			};

			app->CreateBrowser(handler);
			result->Success(flutter::EncodableValue(texture_handler->texture_id()));
		}
		else {
			result->NotImplemented();
		}
	}

}  // namespace webview_cef
