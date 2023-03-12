import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQml 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.10

AbDialog
{
    signal driveEntered(string val)
    title: "Initialize WSL"
    dialog_label: "Drive"
    dialog_text: "WSL is not found on your computer," +
                 " Please specify a drive\nthat you" +
                 " have at least 10GB space to" +
                 " download the 1GB\nKalB image" +
                 "\nExample: c (case insensitive)"
    height: 230
    width: 550

    onDialog_valChanged:
    {
        driveEntered(dialog_val);
    }
}
