from __future__ import annotations

import json
import os
from typing import List, Optional, TYPE_CHECKING

if TYPE_CHECKING:
    from ..side_tools import SideTools

from PySide6.QtCore import QUrl, Signal, Slot
from PySide6.QtWebEngineCore import QWebEnginePage, QWebEngineSettings
from PySide6.QtWebEngineWidgets import QWebEngineView
from PySide6.QtWebChannel import QWebChannel
from .bridge import MapBridge


class _DebugPage(QWebEnginePage):
    def javaScriptConsoleMessage(self, level, message, line, sourceId):
        levels = {0: "INFO", 1: "WARNING", 2: "ERROR"}
        print(f"[JS {levels.get(level, level)}] {sourceId}:{line} - {message}")


class MapPanel(QWebEngineView):
    """
    - HTML의 window.toggleOverlay(name, on), window.setWaypoints(payload) 호출 래퍼.
    - bind_tools()를 통해 SideTools 시그널을 연결.
    - QWebChannel(MapBridge)로 웹에서 추가된 포인트를 오른쪽 패널로 전달.
    """
    OVERLAY_DEPTH = "depth"
    OVERLAY_COASTLINE = "coastline"
    OVERLAY_WEATHER = "weather"
    interactivePointAdded = Signal(str, float, float, str)

    def __init__(self, parent=None):
        super().__init__(parent)
        page = _DebugPage(self)
        self.setPage(page)

        s = self.page().settings()
        s.setAttribute(QWebEngineSettings.JavascriptEnabled, True)
        s.setAttribute(QWebEngineSettings.LocalContentCanAccessRemoteUrls, True)
        s.setAttribute(QWebEngineSettings.LocalContentCanAccessFileUrls, True)

        self._ready: bool = False
        self._js_queue: List[str] = []
        self._tools: Optional[SideTools] = None

        # --- WebChannel / Bridge 설정 ---
        self.bridge = MapBridge()
        self.channel = QWebChannel(self.page())
        self.channel.registerObject("bridge", self.bridge)
        self.page().setWebChannel(self.channel)
        self.bridge.pointAdded.connect(self._on_bridge_point_added)

        # HTML 로드
        here = os.path.dirname(os.path.abspath(__file__))
        html_path = os.path.join(here, "map.html")
        if not os.path.exists(html_path):
            print("[MapPanel] map.html 경로가 잘못되었습니다:", html_path)

        self.load(QUrl.fromLocalFile(html_path))
        self.loadFinished.connect(self._on_load_finished)

    # ----------------------------
    # Public: 외부 위젯(SideTools) 바인딩
    # ----------------------------
    def bind_tools(self, tools: SideTools) -> None:
        """
        SideTools의 시그널을 MapPanel의 JS 호출에 연결.
        """
        self._tools = tools

        # 버튼 토글 → 오버레이 토글
        tools.depthToggled.connect(lambda on: self.toggle_overlay(self.OVERLAY_DEPTH, on))
        tools.coastlineToggled.connect(lambda on: self.toggle_overlay(self.OVERLAY_COASTLINE, on))
        tools.weatherToggled.connect(lambda on: self.toggle_overlay(self.OVERLAY_WEATHER, on))

        # 페이지가 이미 로드되어 있다면 즉시 동기화
        if self._ready:
            self._sync_overlays()

    # ----------------------------
    # QWebEngineView lifecycle
    # ----------------------------
    def _on_load_finished(self, ok: bool):
        print("[MapPanel] loadFinished:", ok)
        self._ready = True

        bootstrap = r"""
          window.__overlayQueue = window.__overlayQueue || [];
          if (typeof window.toggleOverlay !== 'function') {
            window.toggleOverlay = function(n, o){ window.__overlayQueue.push([n, !!o]); };
          }
          window.__routeQueue = window.__routeQueue || [];
          if (typeof window.setWaypoints !== 'function') {
            window.setWaypoints = function(p){ window.__routeQueue.push(p); };
          }
          window.__infoQueue = window.__infoQueue || [];
          if (typeof window.setLayerInfo !== 'function') {
            window.setLayerInfo = function(msg){ window.__infoQueue.push(String(msg)); };
          }

          (function ensureBridge(){
            function bind(){
              if (window.bridge) return;
              if (window.qt && window.qt.webChannelTransport && window.QWebChannel) {
                new QWebChannel(window.qt.webChannelTransport, function(channel){
                  window.bridge = channel.objects.bridge;
                });
              } else {
                if (!window.QWebChannel) {
                  console.warn('[Bridge] QWebChannel 미탑재: qwebchannel.js 포함 필요');
                }
              }
            }
            bind();
            var t = setInterval(function(){
              if (window.bridge) return clearInterval(t);
              bind();
            }, 50);
            setTimeout(function(){ clearInterval(t); }, 5000);
          })();
        """
        self.page().runJavaScript(bootstrap)

        while self._js_queue:
            code = self._js_queue.pop(0)
            self.page().runJavaScript(code)
        self._sync_overlays()

    @Slot(str, float, float, str)
    def _on_bridge_point_added(self, t: str, lat: float, lon: float, port: str):
        self.interactivePointAdded.emit(t, lat, lon, port)

    # ----------------------------
    # Internal helpers
    # ----------------------------
    def _exec_js(self, code: str):
        if self._ready:
            self.page().runJavaScript(code)
        else:
            self._js_queue.append(code)

    def _sync_overlays(self) -> None:
        if not self._tools:
            return
        try:
            self.toggle_overlay(self.OVERLAY_DEPTH, self._tools.btn_depth.isChecked())
            self.toggle_overlay(self.OVERLAY_COASTLINE, self._tools.btn_coast.isChecked())
            self.toggle_overlay(self.OVERLAY_WEATHER, self._tools.btn_weather.isChecked())
        except Exception as e:
            print("[MapPanel] _sync_overlays error:", e)

    # ----------------------------
    # Public: JS API wrappers
    # ----------------------------
    def set_waypoints(self, payload: dict):
        js = f"""
          if (typeof window.clearInteractive === 'function') {{
            window.clearInteractive();
          }}

          if (typeof window.setWaypoints !== 'function') {{
            window.__routeQueue = window.__routeQueue || [];
            window.setWaypoints = function(p){{ window.__routeQueue.push(p); }};
          }}
          window.setWaypoints({json.dumps(payload, ensure_ascii=False)});
        """
        self._exec_js(js)

    def toggle_overlay(self, name: str, on: bool):
        js = f"""
          if (typeof window.toggleOverlay !== 'function') {{
            window.__overlayQueue = window.__overlayQueue || [];
            window.toggleOverlay = function(n, o){{ window.__overlayQueue.push([n, !!o]); }};
          }}
          window.toggleOverlay({json.dumps(name)}, {str(on).lower()});
        """
        self._exec_js(js)

    def setLayerInfo(self, text: str):
        js = f"""
          if (typeof window.setLayerInfo !== 'function') {{
            window.__infoQueue = window.__infoQueue || [];
            window.setLayerInfo = function(msg) {{ window.__infoQueue.push(String(msg)); }};
          }}
          window.setLayerInfo({json.dumps(text, ensure_ascii=False)});
        """
        self._exec_js(js)

    def undo_interactive(self):
        self._exec_js("window.undoInteractive && window.undoInteractive();")

    def clear_interactive(self):
        self._exec_js("window.clearInteractive && window.clearInteractive();")
