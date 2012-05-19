# VopiTest, Video call test with QXmpp #

## Intro ##

Small video call test program using [QXmpp](http://code.google.com/p/qxmpp/).

## Compiling ##

You must compile and install the latest version of [QXmpp](http://code.google.com/p/qxmpp/) (0.4.0) with Speex, Vpx and Theora codecs support.  
For ArchLinux users, a PKGBUILD is provided, you can install [QXmpp](http://code.google.com/p/qxmpp/) with:

    cd qxmpp
    makepkg -i

Also you must install [OpenCV](http://opencv.willowgarage.com/wiki/).  
To compile and run VoipTest:

    qmake && make
