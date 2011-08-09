import QtQuick 1.0

QtObject {
    id: root
    property string thumbnail: root.getThumbnail(themeResourcePath + "/media/DefaultAudio.png", themeResourcePath + "/media/DefaultVideo.png")
    property string artist: root.getMetaData("artist", qsTr("Unknown Artist"))
    property string album: root.getMetaData("album", qsTr("Unknown Album"))
    property string title: root.getMetaData("title", qsTr("Unknown Title"))
    property string track: root.getMetaData("track", "")
    property string mediaId: root.getMetaData("id", "0")

    function getMetaData(role, defaultValue) {
        return playlist ? (playlist.data(playlist.currentIndex, role) || defaultValue) : ""
    }

    function getThumbnail(defaultAudio, defaultVideo) {
        if (playlist && playlist.currentIndex != -1) {
            var thumbnail = playlist.data(playlist.currentIndex, "previewUrl")
            if (thumbnail != "")
                return thumbnail;
        }
        return mediaItem.hasVideo ? defaultVideo : defaultAudio;
    }
}
