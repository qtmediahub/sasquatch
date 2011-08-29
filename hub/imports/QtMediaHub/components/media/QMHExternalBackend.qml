import QtQuick 1.0
import AbstractMediaPlayer 1.0

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
    status: AbstractMediaPlayer.NoMedia
    paused: false
    playbackRate: 1.0


    onSourceChanged: {
        runtime.mediaPlayer.source = source
        //Benefit of the doubt!
        status = AbstractMediaPlayer.Buffered
        hasAudio = true
        playing = true
    }

    onPausedChanged:
        //Why are these discrete functions again girish?
        if (paused)
            runtime.mediaPlayer.pause()
        else
            runtime.mediaPlayer.resume()

    //Having a hard time deciding what to maintain
    //As (setable) property and what to keep as function
    function play() { runtime.mediaPlayer.play() }
    function stop() { runtime.mediaPlayer.stop() }
}
