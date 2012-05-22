/*
 * VopiTest, Video call test with QXmpp.
 * Copyright (C) 2012  Gonzalo Exequiel Pedone
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with This program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Email   : hipersayan DOT x AT gmail DOT com
 * Web-Site: http://hipersayanx.blogspot.com/
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTimer>
#include <QFile>
#include <QProcess>
#include <QMainWindow>
#include <QXmppClient.h>
#include <QXmppRosterManager.h>
#include <QXmppCallManager.h>
#include <QXmppRtpChannel.h>
#include <opencv2/opencv.hpp>

#include "ui_mainwindow.h"

class MainWindow: public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);

    protected:
        void changeEvent(QEvent *e);

    private:
        QXmppClient m_client;
        QStringList m_roster;
        QXmppCall *m_call;
        QXmppCallManager m_callManager;
        QTimer m_timer;
        qint32 m_fps;
        cv::VideoCapture m_webcam;
        QFile m_output;

        quint8 clamp(qint32 value);
        quint8 med(quint8 v1, quint8 v2);
        quint8 rgb2y(quint8 r, quint8 g, quint8 b);
        quint8 rgb2u(quint8 r, quint8 g, quint8 b);
        quint8 rgb2v(quint8 r, quint8 g, quint8 b);
        qint32 y2uv(qint32 y, qint32 width);
        qint32 yuv2r(quint8 y, quint8 u, quint8 v);
        qint32 yuv2g(quint8 y, quint8 u, quint8 v);
        qint32 yuv2b(quint8 y, quint8 u, quint8 v);
        QXmppVideoFrame imageToVideoFrame(const QImage &image);
        QImage videoFrameToImage(const QXmppVideoFrame &videoFrame);

    private slots:
        void on_btnCall_clicked();
        void on_btnConnect_clicked();
        void on_btnDisconnect_clicked();
        void on_btnEndCall_clicked();
        void audioModeChanged(QIODevice::OpenMode mode);
        void callConnected();
        void callFinished();
        void callReceived(QXmppCall *call);
        void callStarted(QXmppCall *call);
        void connected();
        void disconnected();
        void presenceChanged(const QString &bareJid, const QString &resource);
        void readFrames(const QByteArray &ba);
        void rosterReceived();
        void stateChanged(QXmppCall::State state);
        void videoModeChanged(QIODevice::OpenMode mode);
        void writeFrame();
};

#endif // MAINWINDOW_H
