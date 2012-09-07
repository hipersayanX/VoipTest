# VoipTest, Video call test with QXmpp #

## Intro ##

Small video call test program using [QXmpp](http://code.google.com/p/qxmpp/).

## Compiling ##

You must compile and install the latest version of [QXmpp](http://code.google.com/p/qxmpp/) (0.7.3) with Speex, Vpx and Theora codecs support.  
For ArchLinux users, three PKGBUILD are provided:

* __qxmpp-full__: Version with Speex, Vpx and Theora support.
* __qxmpp-full-git__: Git version with Speex, Vpx and Theora support.
* __qxmpp-static-full-git__: Static Git version with Speex, Vpx and Theora support.

You can install one of these with:

    cd qxmpp-xxx
    makepkg -i

Also you must install [OpenCV](http://opencv.willowgarage.com/wiki/).  
To compile and run VoipTest:

    qmake && make
