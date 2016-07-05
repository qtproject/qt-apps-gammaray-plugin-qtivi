/****************************************************************************
**
** Copyright (C) 2016 Klaralvdalens Datakonsult AB, a KDAB Group company.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtIvi plug-in for GammaRay.
**
** $QT_BEGIN_LICENSE:GPL-QTAS$
** Commercial License Usage
** Licensees holding valid commercial Qt Automotive Suite licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and The Qt Company.  For
** licensing terms and conditions see https://www.qt.io/terms-conditions.
** For further information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qtivisupport.h"

#include <core/metaobject.h>
#include <core/metaobjectrepository.h>

#include <QIviAbstractFeature>
#include <QIviServiceObject>
#include <QIviZonedFeatureInterface>

#include <QDebug>

using namespace GammaRay;

QtIviSupport::QtIviSupport(ProbeInterface* probe, QObject* parent) :
    QObject(parent)
{
    Q_UNUSED(probe);
    registerMetaTypes();
}

void QtIviSupport::registerMetaTypes()
{
    qRegisterMetaType<QIviServiceObject*>();

    MetaObject *mo = 0;
    MO_ADD_METAOBJECT0(QIviServiceInterface);
    MO_ADD_PROPERTY_RO(QIviServiceInterface, QStringList, interfaces);

    MO_ADD_METAOBJECT2(QIviServiceObject, QObject, QIviServiceInterface);

    MO_ADD_METAOBJECT1(QIviAbstractFeature, QObject);
    MO_ADD_PROPERTY_RO(QIviAbstractFeature, QIviAbstractFeature::Error, error);

    MO_ADD_METAOBJECT1(QIviZonedFeatureInterface, QObject);
    MO_ADD_PROPERTY_RO(QIviZonedFeatureInterface, QStringList, availableZones);
}

