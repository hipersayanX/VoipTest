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

#include <QDebug>
#include <QMessageBox>
#include <QAudioInput>
#include <QAudioOutput>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
    this->setupUi(this);
    this->m_call = NULL;

    this->m_client.addExtension(&this->m_callManager);

    this->m_fps = 30;

    this->m_timerOutgoing.setInterval(1000.0 / this->m_fps);

    QObject::connect(&this->m_timerOutgoing,
                     SIGNAL(timeout()),
                     this,
                     SLOT(writeFrame()));

    QObject::connect(&this->m_client,
                     SIGNAL(connected()),
                     this,
                     SLOT(connected()));

    QObject::connect(&this->m_client,
                     SIGNAL(disconnected()),
                     this,
                     SLOT(disconnected()));

    QObject::connect(&this->m_client.rosterManager(),
                     SIGNAL(rosterReceived()),
                     this,
                     SLOT(rosterReceived()));

    QObject::connect(&this->m_client.rosterManager(),
                     SIGNAL(presenceChanged(const QString &, const QString &)),
                     this,
                     SLOT(presenceChanged(const QString &, const QString &)));

    QObject::connect(&this->m_callManager,
                     SIGNAL(callReceived(QXmppCall *)),
                     this,
                     SLOT(callReceived(QXmppCall *)));

    QObject::connect(&this->m_callManager,
                     SIGNAL(callStarted(QXmppCall *)),
                     this,
                     SLOT(callStarted(QXmppCall *)));
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);

    switch (e->type())
    {
        case QEvent::LanguageChange:
            this->retranslateUi(this);
        break;

        default:
        break;
    }
}

inline quint8 MainWindow::clamp(qint32 value)
{
    return (uchar) ((value > 255)? 255: ((value < 0)? 0: value));
}

inline quint8 MainWindow::med(quint8 v1, quint8 v2)
{
    return ((v1 + v2) >> 1);
}

inline quint8 MainWindow::rgb2y(quint8 r, quint8 g, quint8 b)
{
    return (( 66  * r + 129 * g + 25  * b + 128) >> 8) + 16;
}

inline quint8 MainWindow::rgb2u(quint8 r, quint8 g, quint8 b)
{
    return ((-38  * r - 74  * g + 112 * b + 128) >> 8) + 128;
}

inline quint8 MainWindow::rgb2v(quint8 r, quint8 g, quint8 b)
{
    return (( 112 * r - 94  * g - 18  * b + 128) >> 8) + 128;
}

inline qint32 MainWindow::y2uv(qint32 y, qint32 width)
{
    return (qint32) (((qint32) (y / width) >> 1) * width / 2.0 + (qint32) ((y % width) / 2.0));
}

inline qint32 MainWindow::yuv2r(quint8 y, quint8 u, quint8 v)
{
    Q_UNUSED(u)

    return ((298 * (y - 16) + 409 * (v - 128) + 128) >> 8);
}

inline qint32 MainWindow::yuv2g(quint8 y, quint8 u, quint8 v)
{
    return ((298 * (y - 16) - 100 * (u - 128) - 208 * (v - 128) + 128) >> 8);
}

inline qint32 MainWindow::yuv2b(quint8 y, quint8 u, quint8 v)
{
    Q_UNUSED(v)

    return ((298 * (y - 16) + 516 * (u - 128) + 128) >> 8);
}

QXmppVideoFrame MainWindow::imageToVideoFrame(const QImage &image)
{
    QXmppVideoFrame videoFrame(2 * image.width() * image.height(),
                               image.size(),
                               2 * image.width(),
                               QXmppVideoFrame::Format_YUYV);

    const quint8 *iBits = (const quint8 *) image.bits();
    quint8 *oBits = (quint8 *) videoFrame.bits();

    for (qint32 i = 0; i < 3 * image.width() * image.height(); i += 6)
    {
        quint8 r1 = iBits[i];
        quint8 g1 = iBits[i + 1];
        quint8 b1 = iBits[i + 2];

        quint8 r2 = iBits[i + 3];
        quint8 g2 = iBits[i + 4];
        quint8 b2 = iBits[i + 5];

        // y1
        *oBits++ = this->rgb2y(r1, g1, b1);

        // u
        *oBits++ = this->rgb2u(this->med(r1, r2), this->med(g1, g2), this->med(b1, b2));

        // y2
        *oBits++ = this->rgb2y(r2, g2, b2);

        // v
        *oBits++ = this->rgb2v(this->med(r1, r2), this->med(g1, g2), this->med(b1, b2));
    }

    return videoFrame;
}

QImage MainWindow::videoFrameToImage(const QXmppVideoFrame &videoFrame)
{
    QImage image(videoFrame.size(), QImage::Format_RGB888);

    qint32 width = videoFrame.size().width();
    qint32 height = videoFrame.size().height();

    const quint8 *iBits = (const quint8 *) videoFrame.bits();
    quint8 *oBits = (quint8 *) image.bits();
    const quint8 *yp, *up, *vp;

    switch (videoFrame.pixelFormat())
    {
        case QXmppVideoFrame::Format_YUYV:
            for (qint32 i = 0; i < 2 * width * height; i += 4)
            {
                quint8 y1 = iBits[i];
                quint8 u  = iBits[i + 1];
                quint8 y2 = iBits[i + 2];
                quint8 v  = iBits[i + 3];

                // r1
                *oBits++ = this->clamp(this->yuv2r(y1, u, v));

                // g1
                *oBits++ = this->clamp(this->yuv2g(y1, u, v));

                // b1
                *oBits++ = this->clamp(this->yuv2b(y1, u, v));

                // r2
                *oBits++ = this->clamp(this->yuv2r(y2, u, v));

                // g2
                *oBits++ = this->clamp(this->yuv2g(y2, u, v));

                // b2
                *oBits++ = this->clamp(this->yuv2b(y2, u, v));
            }
        break;

        case QXmppVideoFrame::Format_YUV420P:
            yp = iBits;
            up = yp + width * height;
            vp = up + width * height / 4;

            for (qint32 i = 0; i < width * height; i++)
            {
                quint8 y = yp[i];
                quint8 u = up[this->y2uv(i, width)];
                quint8 v = vp[this->y2uv(i, width)];

                // r
                *oBits++ = this->clamp(this->yuv2r(y, u, v));

                // g
                *oBits++ = this->clamp(this->yuv2g(y, u, v));

                // b
                *oBits++ = this->clamp(this->yuv2b(y, u, v));
            }
        break;

        default:
        break;
    }

    return image;
}

void MainWindow::on_btnConnect_clicked()
{
    this->m_client.connectToServer(this->txtUser->text(), this->txtPassword->text());
}

void MainWindow::on_btnCall_clicked()
{
    QList<QListWidgetItem *> jids = this->lswRoster->selectedItems();

    if (jids.isEmpty())
        return;

    this->m_call = this->m_callManager.call(jids[0]->text());

    QObject::connect(this->m_call,
                     SIGNAL(connected()),
                     this,
                     SLOT(callConnected()));

    QObject::connect(this->m_call,
                     SIGNAL(finished()),
                     this,
                     SLOT(callFinished()));

    QObject::connect(this->m_call,
                     SIGNAL(stateChanged(QXmppCall::State)),
                     this,
                     SLOT(stateChanged(QXmppCall::State)));

    QObject::connect(this->m_call,
                     SIGNAL(audioModeChanged(QIODevice::OpenMode)),
                     this,
                     SLOT(audioModeChanged(QIODevice::OpenMode)));

    QObject::connect(this->m_call,
                     SIGNAL(videoModeChanged(QIODevice::OpenMode)),
                     this,
                     SLOT(videoModeChanged(QIODevice::OpenMode)));
}

void MainWindow::on_btnDisconnect_clicked()
{
    this->m_client.disconnectFromServer();
}

void MainWindow::on_btnEndCall_clicked()
{
    if (this->m_call)
    {
        this->m_call->hangup();

        QObject::disconnect(this->m_call,
                            SIGNAL(connected()),
                            this,
                            SLOT(callConnected()));

        QObject::disconnect(this->m_call,
                            SIGNAL(finished()),
                            this,
                            SLOT(callFinished()));

        QObject::disconnect(this->m_call,
                            SIGNAL(stateChanged(QXmppCall::State)),
                            this,
                            SLOT(stateChanged(QXmppCall::State)));

        QObject::disconnect(this->m_call,
                            SIGNAL(audioModeChanged(QIODevice::OpenMode)),
                            this,
                            SLOT(audioModeChanged(QIODevice::OpenMode)));

        QObject::disconnect(this->m_call,
                            SIGNAL(videoModeChanged(QIODevice::OpenMode)),
                            this,
                            SLOT(videoModeChanged(QIODevice::OpenMode)));

        this->m_call = NULL;
    }

    this->stackedWidget->setCurrentIndex(0);
    this->stbStatusBar->showMessage("");
}

void MainWindow::audioModeChanged(QIODevice::OpenMode mode)
{
    QXmppRtpAudioChannel *channel = this->m_call->audioChannel();

    // prepare audio format
    QAudioFormat format;

    format.setFrequency(channel->payloadType().clockrate());
    format.setChannels(channel->payloadType().channels());
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    // the size in bytes of the audio buffers to/from sound devices
    // 160 ms seems to be the minimum to work consistently on Linux/Mac/Windows
    const int bufferSize = (format.frequency() * format.channels() * (format.sampleSize() / 8) * 160) / 1000;

    if (mode & QIODevice::ReadOnly)
    {
        // initialise audio output
        QAudioOutput *audioOutput = new QAudioOutput(format, this);

        audioOutput->setBufferSize(bufferSize);
        audioOutput->start(channel);
    }

    if (mode & QIODevice::WriteOnly)
    {
        // initialise audio input
        QAudioInput *audioInput = new QAudioInput(format, this);

        audioInput->setBufferSize(bufferSize);
        audioInput->start(channel);
    }
}

void MainWindow::callConnected()
{
    if (this->m_call->direction() == QXmppCall::OutgoingDirection)
        this->m_call->startVideo();

    this->stackedWidget->setCurrentIndex(1);
}

void MainWindow::callFinished()
{
    if (this->m_call->direction() == QXmppCall::OutgoingDirection)
        this->m_call->stopVideo();

    this->stackedWidget->setCurrentIndex(0);
    this->stbStatusBar->showMessage("");
}

void MainWindow::callReceived(QXmppCall *call)
{
    if (QMessageBox::question(this,
                              "Accept Call?",
                              "Accept Call from " + call->jid() + "?",
                              QMessageBox::Ok | QMessageBox::Cancel,
                              QMessageBox::Cancel) == QMessageBox::Ok)
    {

        this->m_call = call;

        QObject::connect(this->m_call,
                         SIGNAL(connected()),
                         this,
                         SLOT(callConnected()));

        QObject::connect(this->m_call,
                         SIGNAL(finished()),
                         this,
                         SLOT(callFinished()));

        QObject::connect(this->m_call,
                         SIGNAL(stateChanged(QXmppCall::State)),
                         this,
                         SLOT(stateChanged(QXmppCall::State)));

        QObject::connect(this->m_call,
                         SIGNAL(audioModeChanged(QIODevice::OpenMode)),
                         this,
                         SLOT(audioModeChanged(QIODevice::OpenMode)));

        QObject::connect(this->m_call,
                         SIGNAL(videoModeChanged(QIODevice::OpenMode)),
                         this,
                         SLOT(videoModeChanged(QIODevice::OpenMode)));

        call->accept();
    }
    else
        call->hangup();
}

void MainWindow::callStarted(QXmppCall *call)
{
    this->m_call = call;
}

void MainWindow::connected()
{
    this->lblStatus->setText("Connected");
    this->m_client.clientPresence().setType(QXmppPresence::Available);
}

void MainWindow::disconnected()
{
    this->lblStatus->setText("Disconnected");
    this->m_roster.clear();
    this->lswRoster->clear();
}

void MainWindow::presenceChanged(const QString &bareJid, const QString &resource)
{
    QStringList roster = this->m_client.rosterManager().getRosterBareJids();

    if (!roster.contains(bareJid))
        return;

    QXmppPresence presence = this->m_client.rosterManager().getPresence(bareJid, resource);
    QString jid = bareJid + "/" + resource;

    if (presence.status().type() == QXmppPresence::Status::Online)
    {
        if (!this->m_roster.contains(jid))
            this->m_roster << jid;
    }
    else
    {
        if (this->m_roster.contains(jid))
            this->m_roster.removeAll(jid);
    }

    this->lswRoster->clear();
    this->lswRoster->addItems(this->m_roster);
}

void MainWindow::readFrames()
{
    foreach (QXmppVideoFrame frame, this->m_call->videoChannel()->readFrames())
    {
        if (!frame.isValid())
            continue;

        // Vpx packet could not be decoded: Truncated packet or corrupt partition 0 length

        const QImage image = this->videoFrameToImage(frame);
        this->lblIncomingFrame->setPixmap(QPixmap::fromImage(image));
    }
}

void MainWindow::rosterReceived()
{
}

void MainWindow::stateChanged(QXmppCall::State state)
{
    switch (state)
    {
        case QXmppCall::ConnectingState:
            this->stbStatusBar->showMessage("Connecting Call");
        break;

        case QXmppCall::ActiveState:
            this->stbStatusBar->showMessage("Active Call");
        break;

        case QXmppCall::DisconnectingState:
            this->stbStatusBar->showMessage("Disconnecting Call");
        break;

        case QXmppCall::FinishedState:
            this->stbStatusBar->showMessage("Finished Call");
        break;

        default:
            this->stbStatusBar->showMessage("");
        break;
    }
}

void MainWindow::videoModeChanged(QIODevice::OpenMode mode)
{
    if (mode & QIODevice::ReadOnly)
    {
        QXmppVideoFormat videoFormat;

        this->m_webcam.open(this->cbxWebcam->currentIndex());

        // Default Decoder Format
        // {
        //     frameRate =  15
        //     frameSize =  QSize(320, 240)
        //     pixelFormat =  18
        // }
        //
        // Default Encoder Format
        // {
        //     frameRate =  15
        //     frameSize =  QSize(320, 240)
        //     pixelFormat =  21
        // }

        videoFormat.setFrameRate(this->m_fps);
        videoFormat.setFrameSize(QSize(this->m_webcam.get(CV_CAP_PROP_FRAME_WIDTH),
                                       this->m_webcam.get(CV_CAP_PROP_FRAME_HEIGHT)));

        // Vpx    -> QXmppVideoFrame::Format_YUYV
        // Theora -> QXmppVideoFrame::Format_YUV420P
        //           QXmppVideoFrame::Format_YUYV
        //
        // PixelFormat
        // {
        //     Format_Invalid = 0,
        //     Format_RGB32 = 3,
        //     Format_RGB24 = 4,
        //     Format_YUV420P = 18,
        //     Format_UYVY = 20,
        //     Format_YUYV = 21
        // }

        videoFormat.setPixelFormat(QXmppVideoFrame::Format_YUYV);
        this->m_call->videoChannel()->setEncoderFormat(videoFormat);

        if (!this->m_timerOutgoing.isActive())
        {
            QObject::connect(&this->m_timerOutgoing,
                             SIGNAL(timeout()),
                             this,
                             SLOT(writeFrame()));

            QObject::connect(&this->m_timerIngoing,
                             SIGNAL(timeout()),
                             this,
                             SLOT(readFrames()));

            this->m_timerOutgoing.start();
            this->m_timerIngoing.start();
        }
    }
    else if (mode == QIODevice::NotOpen)
    {
        this->m_webcam.release();

        QObject::disconnect(&this->m_timerOutgoing,
                            SIGNAL(timeout()),
                            this,
                            SLOT(writeFrame()));

        QObject::disconnect(&this->m_timerIngoing,
                            SIGNAL(timeout()),
                            this,
                            SLOT(readFrames()));

        this->m_timerOutgoing.stop();
        this->m_timerIngoing.stop();
    }
}

void MainWindow::writeFrame()
{
    if (!this->m_call)
        return;

    cv::Mat mat;

    this->m_webcam >> mat;

    QImage imageBGR((const uchar *)mat.data, mat.cols, mat.rows, QImage::Format_RGB888);
    QImage imageRGB = imageBGR.rgbSwapped();

    const QXmppVideoFrame frame = this->imageToVideoFrame(imageRGB);

    this->m_call->videoChannel()->writeFrame(frame);

    this->lblOutgoingFrame->setPixmap(QPixmap::fromImage(imageRGB));
}
