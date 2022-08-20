// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_handler.h"

#include <sstream>
#include <string>
#include <iostream>

#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

namespace {

SimpleHandler* g_instance = nullptr;

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type) {
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}

}  // namespace

SimpleHandler::SimpleHandler(bool use_views)
    : use_views_(use_views), is_closing_(false) {
  DCHECK(!g_instance);
  g_instance = this;
}

SimpleHandler::~SimpleHandler() {
  g_instance = nullptr;
}

// static
SimpleHandler* SimpleHandler::GetInstance() {
  return g_instance;
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  if (use_views_) {
    // Set the title of the window using the Views framework.
    CefRefPtr<CefBrowserView> browser_view =
        CefBrowserView::GetForBrowser(browser);
    if (browser_view) {
      CefRefPtr<CefWindow> window = browser_view->GetWindow();
      if (window)
        window->SetTitle(title);
    }
  } else if (!IsChromeRuntimeEnabled()) {
    // Set the title of the window using platform APIs.
    PlatformTitleChange(browser, title);
  }
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Add to the list of existing browsers.
  browser_list_.push_back(browser);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (browser_list_.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Allow Chrome to show the error page.
  if (IsChromeRuntimeEnabled())
    return;

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
    return;

  // Display a load error message using a data: URI.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL "
     << std::string(failedUrl) << " with error " << std::string(errorText)
     << " (" << errorCode << ").</h2></body></html>";

  frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
//    CefPostTask(TID_UI, base::BindOnce(&SimpleHandler::CloseAllBrowsers, this,
//                                       force_close));
    return;
  }

  if (browser_list_.empty())
    return;

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it)
    (*it)->GetHost()->CloseBrowser(force_close);
}

// static
bool SimpleHandler::IsChromeRuntimeEnabled() {
  static int value = -1;
  if (value == -1) {
    CefRefPtr<CefCommandLine> command_line =
        CefCommandLine::GetGlobalCommandLine();
    value = command_line->HasSwitch("enable-chrome-runtime") ? 1 : 0;
  }
  return value == 1;
}

void SimpleHandler::scrollUp()
{
    BrowserList::const_iterator it = browser_list_.begin();
    if (it != browser_list_.end()) {
        CefMouseEvent ev;
        ev.x = 500;
        ev.y = 500;
        (*it)->GetHost()->SendMouseWheelEvent(ev, 0, -100);
    }
}

void SimpleHandler::scrollDown()
{
    BrowserList::const_iterator it = browser_list_.begin();
    if (it != browser_list_.end()) {
        CefMouseEvent ev;
        ev.x = 500;
        ev.y = 500;
        (*it)->GetHost()->SendMouseWheelEvent(ev, 0, 100);
    }
}

void SimpleHandler::changeSize(int w, int h)
{
//    this->width = w;
//    this->height = h;
    BrowserList::const_iterator it = browser_list_.begin();
    if (it != browser_list_.end()) {
        (*it)->GetHost()->WasResized();
    }
}

void SimpleHandler::cursorClick(int x, int y, bool up)
{
    BrowserList::const_iterator it = browser_list_.begin();
    if (it != browser_list_.end()) {
        CefMouseEvent ev;
        ev.x = x;
        ev.y = y;
        (*it)->GetHost()->SendMouseClickEvent(ev, CefBrowserHost::MouseButtonType::MBT_LEFT, up, 1);
    }
}

void SimpleHandler::loadUrl(std::string url)
{
    BrowserList::const_iterator it = browser_list_.begin();
    if (it != browser_list_.end()) {
        (*it)->GetMainFrame()->LoadURL(url);
    }
}

void SimpleHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) {
    int a = 500;
    int b = 500;
    rect = CefRect(0, 0, width, height);
    return;
}

void SimpleHandler::OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type,
                            const CefRenderHandler::RectList &dirtyRects, const void *buffer, int w, int h) {
    onPaintCallback(buffer, w, h);
}

void SimpleHandler::PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                                        const CefString& title) {
}

