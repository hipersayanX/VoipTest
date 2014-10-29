#!/bin/sh

cd qxmpp
qmake-qt5 PREFIX=/usr QXMPP_USE_SPEEX=1 QXMPP_USE_VPX=1 QXMPP_USE_THEORA=1 qxmpp.pro
make
