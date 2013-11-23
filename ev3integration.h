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
  \brief Integration class for the Lego Mindstorms EV3. Based on Qt qlinuxfbintegration.h.
  */

#ifndef EV3INTEGRATION_H
#define EV3INTEGRATION_H

#include <qpa/qplatformintegration.h>

class QAbstractEventDispatcher;
class Ev3Screen;

class Ev3Integration : public QPlatformIntegration
{
public:
    Ev3Integration(const QStringList & _paramList);
    ~Ev3Integration();

    bool hasCapability(QPlatformIntegration::Capability cap) const;

    QPlatformPixmap *createPlatformPixmap(QPlatformPixmap::PixelType type) const;
    QPlatformWindow *createPlatformWindow(QWindow *window) const;
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const;
    QAbstractEventDispatcher *guiThreadEventDispatcher() const;
    QList<QPlatformScreen *> screens() const;
    QPlatformFontDatabase *fontDatabase() const;

private:
    Ev3Screen *mprimaryScreen;
    QPlatformFontDatabase *mfontDb;
    QAbstractEventDispatcher *meventDispatcher;
};//Ev3Integration

#endif // EV3INTEGRATION_H

