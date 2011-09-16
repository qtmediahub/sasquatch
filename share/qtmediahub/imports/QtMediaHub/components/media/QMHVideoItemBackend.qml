import QtQuick 1.0
import QtMultimediaKit 1.1

QMHMediaBackend {
    id: root
    anchors.fill: parent
    //property variant source
    //in
    Binding { target: video; property: "source"; value: root.source }

//state
//    property variant hasAudio
//    property variant hasVideo
//    property variant volume
//    property variant position
//    property variant seekable
//    property variant status
//    property variant paused
//    property variant playbackRate

    //out
    Binding { target: root; property: "hasAudio"; value: video.hasAudio }
    Binding { target: root; property: "hasVideo"; value: video.hasVideo }
    Binding { target: root; property: "seekable"; value: video.seekable }
    Binding { target: root; property: "status";   value: video.status }
    Binding { target: root; property: "duration"; value: video.duration }

    //bidirectional
    Binding { target: root; property: "playing"; value: video.playing }
    Binding { target: root; property: "volume"; value: video.volume }
    Binding { target: root; property: "position"; value: video.position }
    Binding { target: root; property: "paused"; value: video.paused }
    Binding { target: root; property: "playbackRate"; value: video.playbackRate }
    //Have to explicitly bind both ways?
    Binding { target: video; property: "playing"; value: root.playing }
    Binding { target: video; property: "volume"; value: root.volume }
    Binding { target: video; property: "position"; value: root.position }
    Binding { target: video; property: "paused"; value: root.paused }
    Binding { target: video; property: "playbackRate"; value: root.playbackRate }

//    function stop() { /*noimpl*/ }
//    function play() { /*noimpl*/ }

    function play() { video.play() }
    function stop() { video.stop() }

    Video {
        id: video
        anchors.fill: parent

        volume: runtime.config.value("media-volume", 0.1)

        //Work around VideoItem shortcomings
        property int _seekPos : -1

        onSeekableChanged : {
            if (seekable && _seekPos != -1) {
                position = _seekPos
                _seekPos = -1
            }
        }

        function seek(pos) {
            //This works? what happens when already seekable?
            _seekPos = pos
        }
    }
}
