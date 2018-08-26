import QtQuick 2.7
import org.kde.kwin.mousemark 1.0

Item {
    id: root

    MouseMarkCanvas {
        id: canvas
        objectName: "canvas"
        anchors.fill: parent
    }
}
