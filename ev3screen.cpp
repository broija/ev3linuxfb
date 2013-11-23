/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

/*!
  \author Broija
  \date 2013-11-22
  \brief Screen class for the Lego Mindstorms EV3. Based on Qt qlinuxfbscreen.cpp.
  \attention This file is provided 'as is' with no warranty of any kind. I can't be held responsible for any damage caused to your computer system, Lego Mindstorms EV3 or anything else.
  */

#include "ev3screen.h"
#include <QtPlatformSupport/private/qfbcursor_p.h>
#include <QtGui/QPainter>
#include <QByteArray>

#include <private/qcore_unix_p.h> // overrides QT_OPEN
#include <qdebug.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <linux/kd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include <linux/fb.h>

#define INVERT_PIXELS 0
#define HOST_BUILD 0

namespace
{
    const int SCREEN_PIX_WIDTH = 178;
    const int SCREEN_PIX_HEIGHT = 128;

    /// \warning truly inaccurate.
    const qreal SCREEN_MM_WIDTH = 50.;
    const qreal SCREEN_MM_HEIGHT = 27.;

    const int FRAMEBUFFER_WIDTH = 60;//60-byte wide : 178 pixels, 3bits per byte ==> 59.3333 bytes needed
    const int FRAMEBUFFER_LENGTH = FRAMEBUFFER_WIDTH * SCREEN_PIX_HEIGHT;

#if INVERT_PIXELS
    const uchar IMAGE_TO_FB_LOOKUP_TABLE[] = {0x00,0xE0,0x1C,0xFC,0x03,0xE3,0x1F,0xFF};
#else
    /*
    EV3 Framebuffer (/dev/fb0):

    3bits set to 1 ==> pixel = 1

    3 bits in the image = 1 byte in the framebuffer

    1 byte = 3 pixels probably for 32-bit alignment reasons.
    60 bytes * 8 bits / 32 bits = 15 groups of 32 bits
    60 bytes * 128 lines * 8 bits / 32 bits = 1920 groups of 32 bits
      Image         Framebuffer
        0   ==>   00000000 : 0x00
        1   ==>   11100000 : 0xE0
        2   ==>   00011100 : 0x1C
        3   ==>   11111100 : 0xFC
        4   ==>   00000011 : 0x03
        5   ==>   11100011 : 0xE3
        6   ==>   00011111 : 0x1F
        7   ==>   11111111 : 0xFF  */
    const uchar IMAGE_TO_FB_LOOKUP_TABLE[] = {0xFF,0x1F,0xE3,0x03,0xFC,0x1C,0xE0,0x00};//Inverted so that bits set to 1 in the image are set to 0 in the framebuffer.
#endif
}//namespace

Ev3Screen::Framebuffer::Framebuffer(int _width, int _height, Format _format)
    :QImage(_width,_height,_format)
{}//Framebuffer

//------------------------------

Ev3Screen::Framebuffer::~Framebuffer()
{
    if (mfbFd != -1)
    {
        munmap(mfbData, FRAMEBUFFER_LENGTH);
        close(mfbFd);
    }//if (mfbFd != -1)
}//~Framebuffer

//------------------------------

bool Ev3Screen::Framebuffer::init()
{
    bool result = false;

    const char * fbFileName = "/dev/fb0";

    mfbFd = -1;

    if (access(fbFileName, R_OK|W_OK) == 0)
        mfbFd = QT_OPEN(fbFileName, O_RDWR);

    if (mfbFd == -1)
        qWarning("Failed to open framebuffer %s : %s",fbFileName,strerror(errno));
    else
    {
        uchar * data = static_cast<unsigned char *>(mmap(0, FRAMEBUFFER_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, mfbFd, 0));

        if (reinterpret_cast<long>(data) == -1)
        {
            qWarning("Failed to mmap framebuffer: %s", strerror(errno));
            return false;
        }//if (reinterpret_cast<long>(data) == -1)

        mfbData = data;

        result = true;
    }//if (mfbFd == -1)

    return result;
}//Framebuffer::init

//------------------------------

bool Ev3Screen::Framebuffer::display()
{
    bool result = true;
    int fbOffset = 0;

    const uchar * pBits = constBits();
    QSize imageSize = QImage::size();
    int imageHeight = imageSize.height();

    int bytesPerLine = QImage::bytesPerLine();

    int image24bits;
    int rowByteOffset;
    int colByteOffset;
    int pixelCount;
    int remainingBits;

    for (int row = 0; row < imageHeight; row++)
    {
        rowByteOffset = row * bytesPerLine;
        pixelCount = 0;

        for (int col = 0; col < bytesPerLine && pixelCount < SCREEN_PIX_WIDTH; col+=3)
        {
            colByteOffset = rowByteOffset + col;

            //We must regroup bytes by 3 because the lookup table allows to match 3 bits only
            //whereas a byte contains 3 + 3 + 2 bits
            image24bits  =  pBits[colByteOffset++];
            image24bits |= (pBits[colByteOffset++] << 8);
            image24bits |= (pBits[colByteOffset]   << 16);

            remainingBits = 24;

            while (remainingBits && pixelCount < SCREEN_PIX_WIDTH)
            {
                //3 bits in the image = 1 byte in the framebuffer
                mfbData[fbOffset++] = IMAGE_TO_FB_LOOKUP_TABLE[image24bits & 0x7];
                pixelCount += 3;
                remainingBits -= 3;
                image24bits >>= 3;
            }//while (remainingBits && pixelCount < SCREEN_PIX_WIDTH)

        }//for (int col = 0; col < bytesPerLine && pixelCount < SCREEN_PIX_WIDTH; col+=3)
    }//for (int row = 0; row < imageSize.height(); row++)

    return result;
}//display

//------------------------------

Ev3Screen::Ev3Screen()
    :mBlitter(0)
{
}//Ev3Screen

//----------------------------------

Ev3Screen::~Ev3Screen()
{
    delete mBlitter;
    delete mpEv3ScreenBuffer;
}//~Ev3Screen

//----------------------------------

bool Ev3Screen::initialize(const QStringList &)
{
    mDepth = 1;
    mFormat = QImage::Format_MonoLSB;

    mGeometry = QRect(QPoint(0, 0), QSize(SCREEN_PIX_WIDTH, SCREEN_PIX_HEIGHT));
    mPhysicalSize = QSizeF(SCREEN_MM_WIDTH, SCREEN_MM_HEIGHT);

    QFbScreen::initializeCompositor();
    mpEv3ScreenBuffer = new Framebuffer(SCREEN_PIX_WIDTH, SCREEN_PIX_HEIGHT, mFormat);

    mCursor = new QFbCursor(this);

    return mpEv3ScreenBuffer->init();
}//initialize

//----------------------------------

QRegion Ev3Screen::doRedraw()
{
    QRegion touched = QFbScreen::doRedraw();

    if (touched.isEmpty())
        return touched;

    if (!mBlitter)
        mBlitter = new QPainter(mpEv3ScreenBuffer);

    QVector<QRect> rects = touched.rects();
    for (int i = 0; i < rects.size(); i++)
        mBlitter->drawImage(rects[i], *mScreenImage, rects[i]);

    mpEv3ScreenBuffer->display();

    return touched;
}//doRedraw

//----------------------------------

