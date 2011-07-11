CREATE TABLE directories (
    path TEXT NOT NULL,
    type TEXT NOT NULL,
    name TEXT NOT NULL,
    PRIMARY KEY (path, type)
);

CREATE TABLE music (
    id INTEGER PRIMARY KEY, /* alias for ROWID, see http://www.sqlite.org/autoinc.html */
    filepath TEXT NOT NULL UNIQUE,

    /* taglib */
    title TEXT,
    album TEXT,
    artist TEXT,
    track INTEGER,
    year INTEGER,
    genre TEXT,
    comment TEXT,
    thumbnail TEXT,
    uri TEXT,

    length INTEGER,
    bitrate INTEGER,
    samplerate INTEGER,

    /* stat information */
    directory TEXT NOT NULL,
    mtime INTEGER NOT NULL,
    ctime INTEGER NOT NULL,
    filesize INTEGER NOT NULL
);

CREATE INDEX idx_album ON music (album);

CREATE INDEX idx_artist ON music (artist);

CREATE TABLE video (
    id INTEGER PRIMARY KEY, /* alias for ROWID, see http://www.sqlite.org/autoinc.html */
    filepath TEXT NOT NULL UNIQUE,

    title TEXT,
    thumbnail TEXT,
    uri TEXT,

    /* stat information */
    directory TEXT NOT NULL,
    mtime INTEGER NOT NULL,
    ctime INTEGER NOT NULL,
    filesize INTEGER NOT NULL
);

CREATE TABLE picture (
    id INTEGER PRIMARY KEY, /* alias for ROWID, see http://www.sqlite.org/autoinc.html */
    filepath TEXT NOT NULL UNIQUE,

    title TEXT,
    thumbnail TEXT,

    year INTEGER,
    month INTEGER,

    /* exif */
    comments TEXT,
    description TEXT,
    created TEXT,
    camera_model TEXT,
    camera_make TEXT,

    latitude REAL,
    longitude REAL,
    altitude REAL,

    orientation INTEGER,

    aperture TEXT,
    focal_length TEXT,
    exposure_time TEXT,
    exposure_mode TEXT,
    white_balance TEXT,
    light_source TEXT,
    iso_speed TEXT,
    digital_zoom_ratio TEXT,
    flash_usage TEXT,
    color_space TEXT,

    /* stat information */
    directory TEXT NOT NULL,
    mtime INTEGER NOT NULL,
    ctime INTEGER NOT NULL,
    filesize INTEGER NOT NULL
);

CREATE TABLE radio (
    id INTEGER PRIMARY KEY, /* alias for ROWID, see http://www.sqlite.org/autoinc.html */
    filepath TEXT NOT NULL UNIQUE,

    title TEXT,
    thumbnail TEXT,

    length INTEGER,
    uri TEXT,

    /* stat information */
    directory TEXT NOT NULL,
    mtime INTEGER NOT NULL,
    ctime INTEGER NOT NULL,
    filesize INTEGER NOT NULL
);
