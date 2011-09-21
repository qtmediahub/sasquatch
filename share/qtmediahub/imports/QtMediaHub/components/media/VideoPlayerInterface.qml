import QtQuick 1.0

// The code here is merely documentation of the properties that
// are required to be implemented by a VideoPlayer
Item {
    property string source
    property bool playing
    property bool hasAudio
    property bool hasVideo
    property int volume
    property int position
    property bool seekable
    property variant status
    property bool paused
    property real playbackRate
    property int duration

    function play() { }
    function stop() { }
    function pause() { }
    function resume() { }
}
