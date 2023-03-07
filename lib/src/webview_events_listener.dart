part of webview;

typedef TitleChangeCb = void Function(String title);
typedef UrlChangeCb = void Function(String url);
typedef CefQueryCallback = void Function({
  String? request,
});
typedef ScrollOffsetChangedCallback = void Function(double x, double y);

class WebviewEventsListener {
  TitleChangeCb? onTitleChanged;
  UrlChangeCb? onUrlChanged;
  ScrollOffsetChangedCallback? onScrollOffsetChanged;
  CefQueryCallback? onCefQuery;

  WebviewEventsListener({
    this.onTitleChanged,
    this.onUrlChanged,
    this.onScrollOffsetChanged,
    this.onCefQuery,
  });
}
