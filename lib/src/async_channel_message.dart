// library webview;
part of webview;

class _AsyncChannelMessageManager {
  static int _id = 0;
  static final Map<int, AsyncChannelMessage> _channelMessages = {};

  static int get nextID => ++_id;
  static registerMessageCallback(AsyncChannelMessage m) {
    assert(!_channelMessages.containsKey(m.id));
    _channelMessages[m.id] = m;
  }

  static const _keyID = '_id_';
  static const _keyResult = '_result_';
  static const _keyError = '_error_';

  static bool isEvalError(String s) => s == "Evaluate Error";

  static handleChannelEvents(dynamic result) {
    final m = result as Map<dynamic, dynamic>;
    assert(m[_keyID] is int);
    final id = m[_keyID] as int;
    assert(_channelMessages.containsKey(id));
    final message = _channelMessages[id]!;
    if (m.containsKey(_keyError)) {
      final errorMsg = m[_keyError] as String;
      if (isEvalError(errorMsg)) {
        if (!message.throwEvalError) {
          debugPrint(formatEvalError(m));
          message._completer.complete();
        } else {
          message._completer.completeError(errorMsg, StackTrace.fromString(formatEvalError(m)));
        }
      } else {
        message._completer.completeError(errorMsg);
      }
    } else {
      final data = m[_keyResult] as String;
      if (data.isEmpty) {
        message._completer.complete();
      } else {
        message._completer.complete(jsonDecode(data));
      }
    }
  }

  static formatEvalError(Map<dynamic, dynamic> m) {
    final buff = StringBuffer();
    buff.writeln('${m['message']}\n  at <${m['file']}>:${m['line']}:${m['column']}');
    buff.writeln('${m['sourceLine']}\n');

    return buff.toString();
  }

  static Future<T> invokeMethod<T>(MethodChannel methodChannel, AsyncChannelMessage<T> message) async {
    _AsyncChannelMessageManager.registerMessageCallback(message);
    final Map<String, dynamic> args = {};
    message.setArguments(args);
    await methodChannel.invokeMethod(message.method, args);
    return message._completer.future;
  }
}

abstract class AsyncChannelMessage<T> {
  final int id;
  final String method;
  final bool throwEvalError;
  final Completer<T> _completer = Completer();

  AsyncChannelMessage(this.method, {
    this.throwEvalError = false,
  }) : id = _AsyncChannelMessageManager.nextID;

  @mustCallSuper
  setArguments(Map<String, dynamic> m) {
    m[_AsyncChannelMessageManager._keyID] = id;
  }
}

class EvaluateJavaScriptMessage extends AsyncChannelMessage {
  final String code;

  EvaluateJavaScriptMessage(this.code, {
    bool throwEvalError = false,
  }) : super('evaluateJavaScript', throwEvalError: throwEvalError);

  @override
  setArguments(Map<String, dynamic> m) {
    super.setArguments(m);
    m['code'] = code;
  }
}
