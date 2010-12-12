TEMPLATE = subdirs

SUBDIRS = music \
          video \
          scripts \
          programs \
          system \
          dashboard \
          customcursor

!mac: SUBDIRS += picture
