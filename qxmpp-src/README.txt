Put here the source code of QXmpp.
You must add this line at the beginning of the qxmpp.pri file:

QXMPP_LIBRARY_TYPE=staticlib

And this line to enable codecs support:

QXMPP_USE_SPEEX=1
QXMPP_USE_VPX=1
QXMPP_USE_THEORA=1
