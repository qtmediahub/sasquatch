import QtQuick 1.0

Item {
    property variant source

    //state
    property variant playing
    property variant hasAudio
    property variant hasVideo
    property variant volume
    property variant position
    property variant seekable
    property variant status
    property variant paused
    property variant playbackRate
    property variant duration

    //Having a hard time deciding what to maintain
    //As (setable) property and what to keep as function
    function play() { /*noimpl*/ }
    function stop() { /*noimpl*/ }
}
