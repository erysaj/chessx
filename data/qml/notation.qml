import QtQuick 2.0
import QtQuick.Controls 2.0

ListView {
    width: 300
    height: 200

    orientation: Qt.Vertical
    focus: true

    highlight: Rectangle {
        anchors.fill: parent
        color: "blue"
    }

    model: ListModel {
        ListElement { name: "Name 0" }
        ListElement { name: "Name 1" }
        ListElement { name: "Name 2" }
        ListElement { name: "Name 3" }
        ListElement { name: "Name 4" }
        ListElement { name: "Name 5" }
        ListElement { name: "Name 6" }
        ListElement { name: "Name 7" }
        ListElement { name: "Name 8" }
        ListElement { name: "Name 9" }
        ListElement { name: "Name 10" }
        ListElement { name: "Name 11" }
        ListElement { name: "Name 12" }
        ListElement { name: "Name 13" }
        ListElement { name: "Name 14" }
        ListElement { name: "Name 15" }
        ListElement { name: "Name 16" }
        ListElement { name: "Name 17" }
        ListElement { name: "Name 18" }
        ListElement { name: "Name 19" }
    }

    delegate: Rectangle {
        width: parent.width
        height: 40
        color: "lightgray"

        Label {
            anchors.centerIn: parent
            text: name
        }
    }
}
