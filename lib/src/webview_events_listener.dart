part of webview;

typedef TitleChangeCb = void Function(String title);
typedef UrlChangeCb = void Function(String url);
typedef CefQueryCallback = void Function({
  String? request,
});

class WebviewEventsListener {
  TitleChangeCb? onTitleChanged;
  UrlChangeCb? onUrlChanged;
  CefQueryCallback? onCefQuery;

  WebviewEventsListener({
    this.onTitleChanged,
    this.onUrlChanged,
    this.onCefQuery,
  });
}
