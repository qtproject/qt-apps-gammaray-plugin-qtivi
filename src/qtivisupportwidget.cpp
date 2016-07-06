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

#include "qtivisupportwidget.h"
#include "qtivipropertymodel.h"

#include <common/objectbroker.h>
#include <common/endpoint.h>

#include <QTreeView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>

using namespace GammaRay;

QtIVIWidget::QtIVIWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("QtIVIWidget");
    QAbstractItemModel *propertyModel
        = ObjectBroker::model(QStringLiteral("com.kdab.GammaRay.PropertyModel"));

    QVBoxLayout *vbox = new QVBoxLayout(this);

    auto objectTreeView = new QTreeView(this);
    objectTreeView->header()->setObjectName("objectTreeViewHeader");
    vbox->addWidget(objectTreeView);

    QItemSelectionModel *selectionModel = ObjectBroker::selectionModel(propertyModel);
    objectTreeView->setModel(propertyModel);
}
