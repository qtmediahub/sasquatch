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
    thumbnail BLOB,

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

