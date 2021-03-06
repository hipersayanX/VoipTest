# VopiTest, Video call test with QXmpp.
# Copyright (C) 2012  Gonzalo Exequiel Pedone
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with This program.  If not, see <http://www.gnu.org/licenses/>.
#
# Email   : hipersayan DOT x AT gmail DOT com
# Web-Site: http://hipersayanx.blogspot.com/

FORMS += mainwindow.ui

HEADERS += mainwindow.h

QT += core gui network xml multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += main.cpp\
           mainwindow.cpp

TARGET = VoipTest
TEMPLATE = app

INCLUDEPATH += \
    qxmpp/src/base \
    qxmpp/src/client

LIBS += -L./qxmpp/src -lqxmpp_d

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += speex vpx theoradec theoraenc opencv
}

#unix {
#    INCLUDEPATH += /usr/include/qxmpp

#    CONFIG += link_pkgconfig
#    PKGCONFIG += speex vpx theoradec theoraenc qxmpp opencv
#}

#win32 {
#    INCLUDEPATH += win32/include/qxmpp
#    LIBS += -Lwin32/lib -lqxmpp
#}
