library webview;

import 'dart:async';
import 'dart:convert';

import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

part 'webview_cursor.dart';
part 'webview_events_listener.dart';
part 'async_channel_message.dart';

const MethodChannel _pluginChannel = MethodChannel("webview_cef");
bool _hasCallStartCEF = false;
final _cefStarted = Completer();

_startCEF() async {
  if (!_hasCallStartCEF) {
    _hasCallStartCEF = true;
    _pluginChannel.invokeMethod("startCEF");
    _pluginChannel.setMethodCallHandler((call) async {
      if (call.method == 'onCEFInitialized') {
        _cefStarted.complete();
      }
    });
  }

  await _cefStarted.future;
}

const _kEventTitleChanged = "titleChanged";
const _kEventURLChanged = "urlChanged";
const _kEventCursorChanged = "cursorChanged";
const _kEventScrollOffsetChanged = "scrollOffsetChanged";
const _kEventLoadingProgressChanged = "loadingProgressChanged";
const _kEventLoadingStateChanged = "loadingStateChanged";
const _kEventLoadStart = "loadStart";
const _kEventLoadEnd = "loadEnd";
const _kEventLoadError = "loadError";
const _kEventAsyncChannelMessage = 'asyncChannelMessage';

class WebViewController extends ValueNotifier<bool> {
  static int _id = 0;

  final Completer<void> _creatingCompleter = Completer<void>();
  late final int _browserID;
  int _textureId = 0;
  bool _isDisposed = false;
  WebviewEventsListener? _listener;
  late final MethodChannel _broswerChannel;
  late final EventChannel _eventChannel;
  StreamSubscription? _eventStreamSubscription;

  final ValueNotifier<CursorType> _cursorType = ValueNotifier(CursorType.pointer);

  Future<void> get ready => _creatingCompleter.future;

  WebViewController() : super(false);

  /// Initializes the underlying platform view.
  Future<void> initialize() async {
    if (_isDisposed) {
      return Future<void>.value();
    }

    await _startCEF();

    try {
      _browserID = ++_id;
      _broswerChannel = MethodChannel('webview_cef/$_browserID');
      _broswerChannel.setMethodCallHandler(_methodCallhandler);
      _textureId = await _pluginChannel.invokeMethod<int>('createBrowser', _browserID) ?? 0;
      _eventChannel = EventChannel('webview_cef/$_browserID/events');
      _eventStreamSubscription = _eventChannel.receiveBroadcastStream().listen(_handleBrowserEvents);
    } on PlatformException catch (e) {
      _creatingCompleter.completeError(e);
    }

    return _creatingCompleter.future;
  }

  Future<dynamic> _methodCallhandler(MethodCall call) async {
    switch (call.method) {
      case 'onBrowserCreated':
        _creatingCompleter.complete();
        value = true;
        return null;
      case 'onCefQuery':
        _listener?.onCefQuery?.call(request: call.arguments);
        return null;
    }

    return null;
  }

  _handleBrowserEvents(dynamic event) {
    final m = event as Map<dynamic, dynamic>;
    switch (m['type']) {
      case _kEventURLChanged:
        _listener?.onUrlChanged?.call(m['value'] as String);
        return;
      case _kEventTitleChanged:
        _listener?.onTitleChanged?.call(m['value'] as String);
        return;
      case _kEventCursorChanged:
        _cursorType.value = CursorType.values[m['value'] as int];
        return;
      case _kEventScrollOffsetChanged:
        final offset = m['value'] as Map<dynamic, dynamic>;
        _listener?.onScrollOffsetChanged?.call(offset['x'] as double, offset['y'] as double);
        return;
      case _kEventLoadingProgressChanged:
        _listener?.onLoadingProgressChanged?.call(m['value'] as double);
        return;
      case _kEventLoadingStateChanged:
        _listener?.onLoadingStateChanged?.call(m['value'] as bool);
        return;
      case _kEventLoadStart:
        _listener?.onLoadStart?.call(m['value'] as String);
        return;
      case _kEventLoadEnd:
        _listener?.onLoadEnd?.call(m['value'] as int);
        return;
      case _kEventLoadError:
        final data = m['value'] as Map<dynamic, dynamic>;
        _listener?.onLoadError?.call(
          data['errorCode'] as int,
          data['errorText'] as String,
          data['failedUrl'] as String,
        );
        return;
      case _kEventAsyncChannelMessage:
        _AsyncChannelMessageManager.handleChannelEvents(m['value']);
        return;
      default:
    }
  }

  setWebviewListener(WebviewEventsListener listener) {
    _listener = listener;
  }

  @override
  Future<void> dispose() async {
    await _creatingCompleter.future;
    if (!_isDisposed) {
      _isDisposed = true;
      await _broswerChannel.invokeMethod('dispose');
      _eventStreamSubscription?.cancel();
      _cursorType.dispose();
    }
    super.dispose();
  }

  /// Loads the given [url].
  Future<void> loadUrl(String url) async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    return _broswerChannel.invokeMethod('loadUrl', url);
  }

  Future<String?> getUrl() async {
    if (_isDisposed) {
      return null;
    }
    assert(value);
    return _broswerChannel.invokeMethod('getUrl');
  }

  Future<void> stopLoad() async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    return _broswerChannel.invokeMethod('stopLoad');
  }

  /// Reloads the current document.
  Future<void> reload() async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    return _broswerChannel.invokeMethod('reload');
  }

  Future<bool> canGoForward() async {
    assert(value);
    return await _broswerChannel.invokeMethod('canGoForward');
  }

  Future<void> goForward() async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    return _broswerChannel.invokeMethod('goForward');
  }

  Future<bool> canGoBack() async {
    assert(value);
    return await _broswerChannel.invokeMethod('canGoBack');
  }

  Future<void> goBack() async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    return _broswerChannel.invokeMethod('goBack');
  }

  Future<dynamic> evaluateJavaScript(String code, [bool throwEvalError = false]) async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    final message = EvaluateJavaScriptMessage(code, throwEvalError: throwEvalError);
    return _AsyncChannelMessageManager.invokeMethod(_broswerChannel, message);
  }

  Future<void> openDevTools() async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    return _broswerChannel.invokeMethod('openDevTools');
  }

  Future<void> _unfocus() async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    return _broswerChannel.invokeMethod('unfocus');
  }

  /// Moves the virtual cursor to [position].
  Future<void> _cursorMove(Offset position) async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    return _broswerChannel
        .invokeMethod('cursorMove', [position.dx.round(), position.dy.round()]);
  }

  Future<void> _cursorDragging(Offset position) async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    return _broswerChannel.invokeMethod(
        'cursorDragging', [position.dx.round(), position.dy.round()]);
  }

  Future<void> _cursorClickDown(Offset position) async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    return _broswerChannel.invokeMethod(
        'cursorClickDown', [position.dx.round(), position.dy.round()]);
  }

  Future<void> _cursorClickUp(Offset position) async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    return _broswerChannel.invokeMethod(
        'cursorClickUp', [position.dx.round(), position.dy.round()]);
  }

  /// Sets the horizontal and vertical scroll delta.
  Future<void> _setScrollDelta(Offset position, int dx, int dy) async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    return _broswerChannel.invokeMethod(
        'setScrollDelta', [position.dx.round(), position.dy.round(), dx, dy]);
  }

  /// Sets the surface size to the provided [size].
  Future<void> _setSize(double dpi, Size size, Offset viewOffset) async {
    if (_isDisposed) {
      return;
    }
    assert(value);
    return _broswerChannel
        .invokeMethod('setSize', [dpi, size.width, size.height, viewOffset.dx, viewOffset.dy]);
  }
}

class WebView extends StatefulWidget {
  final WebViewController controller;

  const WebView(this.controller, {Key? key}) : super(key: key);

  @override
  WebViewState createState() => WebViewState();
}

class WebViewState extends State<WebView> {
  final GlobalKey _key = GlobalKey();
  final _focusNode = FocusNode();

  WebViewController get _controller => widget.controller;

  @override
  void initState() {
    super.initState();
    // Report initial surface size
    WidgetsBinding.instance
        .addPostFrameCallback((_) => _reportSurfaceSize(context));
  }

  @override
  void dispose() {
    _focusNode.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return SizedBox.expand(key: _key, child: _buildInner());
  }

  Widget _buildInner() {
    return NotificationListener<SizeChangedLayoutNotification>(
      onNotification: (notification) {
        _reportSurfaceSize(context);
        return true;
      },
      child: SizeChangedLayoutNotifier(
        child: Focus(
          focusNode: _focusNode,
          autofocus: false,
          onFocusChange: (focused) {
            if (!focused) _controller._unfocus();
          },
          child: Listener(
            onPointerHover: (ev) {
              _controller._cursorMove(ev.localPosition);
            },
            onPointerDown: (ev) async {
              _controller._cursorClickDown(ev.localPosition);

              // Fixes for getting focus immediately.
              if (!_focusNode.hasFocus) {
                _focusNode.unfocus();
                await Future<void>.delayed(const Duration(milliseconds: 1));
                if (mounted) FocusScope.of(context).requestFocus(_focusNode);
              }
            },
            onPointerUp: (ev) {
              _controller._cursorClickUp(ev.localPosition);
            },
            onPointerMove: (ev) {
              _controller._cursorDragging(ev.localPosition);
            },
            onPointerSignal: (signal) {
              if (signal is PointerScrollEvent) {
                _controller._setScrollDelta(signal.localPosition,
                    signal.scrollDelta.dx.round(), signal.scrollDelta.dy.round());
              }
            },
            onPointerPanZoomUpdate: (event) {
              _controller._setScrollDelta(event.localPosition,
                  event.panDelta.dx.round(), event.panDelta.dy.round());
            },
            child: ValueListenableBuilder<CursorType>(
              valueListenable: _controller._cursorType,
              child: Texture(textureId: _controller._textureId),
              builder: (context, value, child) {
                return MouseRegion(
                  cursor: value.transform,
                  child: child,
                );
              },
            ),
          ),
        ),
      ),
    );
  }

  void _reportSurfaceSize(BuildContext context) async {
    final box = _key.currentContext?.findRenderObject() as RenderBox?;
    if (box != null) {
      final dpi = MediaQuery.of(context).devicePixelRatio;
      await _controller.ready;
      final translation = box.getTransformTo(null).getTranslation();
      unawaited(_controller._setSize(
        dpi,
        Size(box.size.width, box.size.height),
        Offset(translation.x, translation.y),
        // box.localToGlobal(Offset.zero),
      ));
    }
  }
}
