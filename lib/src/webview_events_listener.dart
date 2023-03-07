part of webview;

typedef TitleChangeCb = void Function(String title);
typedef UrlChangeCb = void Function(String url);
typedef CefQueryCallback = void Function({
  String? request,
});
typedef ScrollOffsetChangedCallback = void Function(double x, double y);
typedef LoadingProgressChangedCallback = void Function(double v);
typedef LoadingStateChangedCallback = void Function(bool isLoading);
typedef LoadStartCallback = void Function(String url);
typedef LoadEndCallback = void Function(int statusCode);
typedef LoadErrorCallback = void Function(int code, String text, String url);

class WebviewEventsListener {
  TitleChangeCb? onTitleChanged;
  UrlChangeCb? onUrlChanged;
  ScrollOffsetChangedCallback? onScrollOffsetChanged;

  /// Called when the overall page loading progress has changed.
  /// progress ranges from 0.0 to 1.0.
  LoadingProgressChangedCallback? onLoadingProgressChanged;

  /// Called when the loading state has changed. This callback will be executed
  /// twice -- once when loading is initiated either programmatically or by user
  /// action, and once when loading is terminated due to completion, cancellation
  /// of failure. It will be called before any calls to OnLoadStart and after all
  /// calls to OnLoadError and/or OnLoadEnd.
  LoadingStateChangedCallback? onLoadingStateChanged;

  LoadStartCallback? onLoadStart;
  LoadEndCallback? onLoadEnd;
  LoadErrorCallback? onLoadError;
  CefQueryCallback? onCefQuery;

  WebviewEventsListener({
    this.onTitleChanged,
    this.onUrlChanged,
    this.onScrollOffsetChanged,
    this.onLoadingProgressChanged,
    this.onLoadingStateChanged,
    this.onLoadStart,
    this.onLoadEnd,
    this.onLoadError,
    this.onCefQuery,
  });
}
