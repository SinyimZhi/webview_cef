// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef COMMON_RENDERER_CLIENT_APP_RENDERER_H_
#define COMMON_RENDERER_CLIENT_APP_RENDERER_H_
#pragma once

#include <set>

#include "client_app.h"

// Client app implementation for the renderer process.
class ClientAppRenderer : public ClientApp, public CefRenderProcessHandler {
public:
    ClientAppRenderer();

private:
    // CefApp methods.
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return this;
    }

    // CefRenderProcessHandler methods.
    // void OnWebKitInitialized() override;
    // void OnBrowserCreated(CefRefPtr<CefBrowser> browser,
    //                       CefRefPtr<CefDictionaryValue> extra_info) override;
    // void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) override;
    // CefRefPtr<CefLoadHandler> GetLoadHandler() override;
    // void OnContextCreated(CefRefPtr<CefBrowser> browser,
    //                       CefRefPtr<CefFrame> frame,
    //                       CefRefPtr<CefV8Context> context) override;
    // void OnContextReleased(CefRefPtr<CefBrowser> browser,
    //                        CefRefPtr<CefFrame> frame,
    //                        CefRefPtr<CefV8Context> context) override;
    // void OnUncaughtException(CefRefPtr<CefBrowser> browser,
    //                          CefRefPtr<CefFrame> frame,
    //                          CefRefPtr<CefV8Context> context,
    //                          CefRefPtr<CefV8Exception> exception,
    //                          CefRefPtr<CefV8StackTrace> stackTrace) override;
    // void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
    //                           CefRefPtr<CefFrame> frame,
    //                           CefRefPtr<CefDOMNode> node) override;
    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefProcessId source_process,
                                  CefRefPtr<CefProcessMessage> message) override;

private:
    void evaluateJavaScript(CefRefPtr<CefBrowser> browser,
					   CefRefPtr<CefFrame> frame,
					   CefProcessId source_process,
					   CefRefPtr<CefProcessMessage> message);

    IMPLEMENT_REFCOUNTING(ClientAppRenderer);
    DISALLOW_COPY_AND_ASSIGN(ClientAppRenderer);
};

#endif  // COMMON_RENDERER_CLIENT_APP_RENDERER_H_
