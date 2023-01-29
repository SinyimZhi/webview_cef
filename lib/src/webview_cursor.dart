part of webview;

/// Cursor type values.
/// https://bitbucket.org/chromiumembedded/cef/src/ecc89d7d93ebafd0ed86716b09f20ab4206a21ae/include/internal/cef_types.h#lines-2310
enum CursorType {
  pointer,
  cross,
  hand,
  ibeam,
  wait,
  help,
  eastResize,
  northResize,
  northEastResize,
  northWestResize,
  southResize,
  southEastResize,
  southWestResize,
  westResize,
  northSouthResize,
  eastWestResize,
  northEastSouthWestResize,
  northWestSouthEastResize,
  columnResize,
  rowResize,
  middlePanning,
  eastPanning,
  northPanning,
  northEastPanning,
  northWestPanning,
  southPanning,
  southEastPanning,
  southWestPanning,
  westPanning,
  move,
  verticalText,
  cell,
  contextMenu,
  alias,
  progress,
  nodrop,
  copy,
  none,
  notAllowed,
  zoomIn,
  zoomOut,
  grab,
  grabbing,
  middlePanningVertical,
  middlePanningHorizontal,
  custom,
  dndNone,
  dndMove,
  dndCopy,
  dndLink,
}

extension CursorTypeExtension on CursorType {
  SystemMouseCursor get transform {
    switch (this) {
      case CursorType.pointer: return SystemMouseCursors.basic;
      case CursorType.cross: return SystemMouseCursors.precise;
      case CursorType.hand: return SystemMouseCursors.click;
      case CursorType.ibeam: return SystemMouseCursors.text;
      case CursorType.wait: return SystemMouseCursors.wait;
      case CursorType.help: return SystemMouseCursors.help;
      case CursorType.eastResize: return SystemMouseCursors.resizeRight;
      case CursorType.northResize: return SystemMouseCursors.resizeUp;
      case CursorType.northEastResize: return SystemMouseCursors.resizeUpRight;
      case CursorType.northWestResize: return SystemMouseCursors.resizeUpLeft;
      case CursorType.southResize: return SystemMouseCursors.resizeDown;
      case CursorType.southEastResize: return SystemMouseCursors.resizeDownRight;
      case CursorType.southWestResize: return SystemMouseCursors.resizeDownLeft;
      case CursorType.westResize: return SystemMouseCursors.resizeLeft;
      case CursorType.northSouthResize: return SystemMouseCursors.resizeUpDown;
      case CursorType.eastWestResize: return SystemMouseCursors.resizeLeftRight;
      case CursorType.northEastSouthWestResize: return SystemMouseCursors.resizeUpRightDownLeft;
      case CursorType.northWestSouthEastResize: return SystemMouseCursors.resizeUpLeftDownRight;
      case CursorType.columnResize: return SystemMouseCursors.resizeColumn;
      case CursorType.rowResize: return SystemMouseCursors.resizeRow;
      case CursorType.middlePanning: return SystemMouseCursors.allScroll;
      case CursorType.move: return SystemMouseCursors.move;
      case CursorType.verticalText: return SystemMouseCursors.verticalText;
      case CursorType.cell: return SystemMouseCursors.cell;
      case CursorType.contextMenu: return SystemMouseCursors.contextMenu;
      case CursorType.alias: return SystemMouseCursors.alias;
      case CursorType.progress: return SystemMouseCursors.progress;
      case CursorType.nodrop: return SystemMouseCursors.noDrop;
      case CursorType.copy: return SystemMouseCursors.copy;
      case CursorType.none: return SystemMouseCursors.none;
      case CursorType.notAllowed: return SystemMouseCursors.forbidden;
      case CursorType.zoomIn: return SystemMouseCursors.zoomIn;
      case CursorType.zoomOut: return SystemMouseCursors.zoomOut;
      case CursorType.grab: return SystemMouseCursors.grab;
      case CursorType.grabbing: return SystemMouseCursors.grabbing;
      /// @TODO Supports custom cursor
      // case CursorType.custom: return MouseCursor();
      default: return SystemMouseCursors.basic;
    }
  }
}
