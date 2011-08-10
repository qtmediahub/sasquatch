#include "xinebackend.h"

#include <xine.h>
#include <xine/xineutils.h>

class XineBackend::Private
{
public:
    Private();
    ~Private();

    QString mediaPath;
    xine_t              *xine;
    xine_stream_t       *stream;
    xine_audio_port_t   *ao_port;
    xine_event_queue_t  *event_queue;
};

XineBackend::Private::Private()
{
    char configfile[2048];

    xine = xine_new();
    sprintf(configfile, "%s%s", xine_get_homedir(), "/.xine/config");
    xine_config_load(xine, configfile);
    xine_init(xine);

    ao_port = xine_open_audio_driver(xine , "auto" , NULL);
    stream = xine_stream_new(xine, ao_port, NULL);
    event_queue = xine_event_new_queue(stream);
}

XineBackend::Private::~Private()
{
    xine_close(stream);
    xine_event_dispose_queue(event_queue);
    xine_dispose(stream);
    if (ao_port)
        xine_close_audio_driver(xine, ao_port);
    xine_exit(xine);

    //delete xine;
    //delete stream;
    //delete ao_port;
    //delete event_queue;
}

XineBackend::XineBackend(QObject *parent) :
    MediaBackendInterface(parent),
    d(new Private()) { /**/ }

XineBackend::~XineBackend()
{
    delete d;
    d = 0;
}

void XineBackend::loadUri(const QString &uri)
{
    //Strip the formatting off the incoming uri
    //file:// /
    d->mediaPath = uri.mid(7);
}

void XineBackend::stop() {
    xine_close(d->stream);
}

void XineBackend::pause() {
    xine_set_param(d->stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE);
}

void XineBackend::resume() {
    xine_set_param(d->stream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL);
}

void XineBackend::play()
{
    if ((!xine_open(d->stream, d->mediaPath.toAscii().constData()))
            || (!xine_play(d->stream, 0, 0))) {
        printf("Unable to open mrl '%s'\n", d->mediaPath.toAscii().constData());
    }
}

void XineBackend::mute(bool on)
{
    if (on) {
        xine_set_param(d->stream, XINE_PARAM_AUDIO_AMP_MUTE, 1);
    } else {
        xine_set_param(d->stream, XINE_PARAM_AUDIO_AMP_MUTE, 0);
    }
}

void XineBackend::setPosition(int position) { Q_UNUSED(position); }
void XineBackend::setPositionPercent(qreal position) { Q_UNUSED(position); }

void XineBackend::setVolumePercent(qreal volume) {
    //Wants an int for volume
    //Attempting to map to Video Item levels on local machine
    if (volume > 0.20) {
        qDebug() << "Exhibiting cowardise, capping volume";
        volume = 0.20;
    }
    xine_set_param(d->stream, XINE_PARAM_AUDIO_VOLUME, volume*100);
}
