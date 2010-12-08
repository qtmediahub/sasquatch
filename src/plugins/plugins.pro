TEMPLATE = subdirs

SUBDIRS = music \
          video \
          scripts \
          programs \
          dashboard \
          customcursor

!mac: SUBDIRS += picture
