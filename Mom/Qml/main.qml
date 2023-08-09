import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Controls 2.4
import Qt.labs.settings 1.0

Item
{
    id: root

    MmBarWindow
    {
        id: bar1
        objectName: "bar1"
    }

    MmBarWindow
    {
        id: bar2
        objectName: "bar2"
    }
}
