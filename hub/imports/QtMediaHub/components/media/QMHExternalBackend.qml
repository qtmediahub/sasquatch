import QtQuick 1.0
import MediaBackendInterface 1.0

QMHMediaBackend {
//    property variant source

    //state
//    property variant playing
//    property variant hasAudio
//    property variant hasVideo
//    property variant volume
//    property variant position
//    property variant seekable
//    property variant status
//    property variant paused
//    property variant playbackRate
    playing: false
    hasAudio: true
    hasVideo: false
    volume: 0.5
    position: 0
    seekable: true
    status: MediaBackendInterface.NoMedia
    paused: false
    playbackRate: 1.0


    onSourceChanged: {
        runtime.mediaBackendInterface.loadUri(source)
        //Benefit of the doubt!
        status = MediaBackendInterface.Buffered
        hasAudio = true
        playing = true
    }

    onPausedChanged:
        //Why are these discrete functions again girish?
        if (paused)
            runtime.mediaBackendInterface.pause()
        else
            runtime.mediaBackendInterface.resume()

    //Having a hard time deciding what to maintain
    //As (setable) property and what to keep as function
    function play() { runtime.mediaBackendInterface.play() }
    function stop() { runtime.mediaBackendInterface.stop() }
}
