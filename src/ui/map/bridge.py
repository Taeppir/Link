from PySide6.QtCore import QObject, Signal, Slot

class MapBridge(QObject):
    pointAdded = Signal(str, float, float, str)

    @Slot(str, float, float, str)
    def addInteractivePoint(self, t, lat, lon, port):
        self.pointAdded.emit(t, lat, lon, port)
