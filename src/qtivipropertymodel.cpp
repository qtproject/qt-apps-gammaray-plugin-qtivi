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

#include "qtivipropertymodel.h"

#include <core/probe.h>
#include <core/util.h>
#include <common/objectmodel.h>

#include <QIviProperty>

#include <QThread>
#include <QMutexLocker>
#include <QSignalMapper>

using namespace GammaRay;

QtIVIPropertyModel::QtIVIPropertyModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QtIVIPropertyModel::~QtIVIPropertyModel()
{
}

int QtIVIPropertyModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return ColumnCount;
}

int QtIVIPropertyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return m_sourceModel->rowCount();
}

QVariant QtIVIPropertyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_ASSERT(section >= 0);

    if (role == Qt::DisplayRole) {
        switch (section) {
        case AddressColumn:
            return tr("Address");
        case BackendValueColumn:
            return tr("Backend Value");
        case OverrideValueColumn:
            return tr("Override Value");
        case OverrideColumn:
            return tr("Override");
        default:
            return QVariant();
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant QtIVIPropertyModel::data(const QModelIndex &index, int role) const
{
    const int column = index.column();

    const QModelIndex sourceIndex = m_sourceModel->index(index.row(), 0);
    auto const property = qobject_cast<QIviProperty*>(sourceIndex.data(ObjectModel::ObjectRole).value<QObject *>());

    if (role == Qt::DisplayRole) {
        switch (column) {
        case AddressColumn:
            return Util::addressToString(property);
        case BackendValueColumn:
                return property->value();
        case OverrideValueColumn:
            return QVariant();
        case OverrideColumn:
            return QVariant();
        default:
            return QVariant();
        }
    }

    return QVariant();
}

void QtIVIPropertyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    beginResetModel();
    m_sourceModel = sourceModel;

    connect(m_sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(slotBeginInsertRows(QModelIndex,int,int)));
    connect(m_sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotEndInsertRows()));
    connect(m_sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(slotBeginRemoveRows(QModelIndex,int,int)));
    connect(m_sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(slotEndRemoveRows()));
    connect(m_sourceModel, SIGNAL(modelAboutToBeReset()),
            this, SLOT(slotBeginReset()));
    connect(m_sourceModel, SIGNAL(modelReset()),
            this, SLOT(slotEndReset()));
    connect(m_sourceModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(slotBeginReset()));
    connect(m_sourceModel, SIGNAL(layoutChanged()),
            this, SLOT(slotEndReset()));

    endResetModel();
}

void QtIVIPropertyModel::slotBeginRemoveRows(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), start, end);
    qDebug() << "remove row: " << start << " to " << end;
}

void QtIVIPropertyModel::slotEndRemoveRows()
{
    endRemoveRows();
}

void QtIVIPropertyModel::slotBeginInsertRows(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), start, end);
    m_pending_insertions.push_back(qMakePair(start, end));
}

void QtIVIPropertyModel::slotEndInsertRows()
{
    for (auto& range : m_pending_insertions)
    {
        for (int i = range.first; i <= range.second; ++i)
        {
            const QModelIndex sourceIndex = m_sourceModel->index(i, 0);
            auto const property = qobject_cast<QIviProperty*>(sourceIndex.data(ObjectModel::ObjectRole).value<QObject *>());

            connect(property, &QIviProperty::valueChanged, [this, property](const QVariant& value)
            {
                for (auto i = 0; i < rowCount(); ++i)
                {
                    const QModelIndex sourceIndex = m_sourceModel->index(i, 0);
                    auto const model_property = qobject_cast<QIviProperty*>(sourceIndex.data(ObjectModel::ObjectRole).value<QObject *>());

                    if (property == model_property)
                    {
                        emit dataChanged(createIndex(i, BackendValueColumn), createIndex(i, BackendValueColumn));
                        break;
                    }
                }
            });
        }
    }

    m_pending_insertions.clear();
    endInsertRows();
}

void QtIVIPropertyModel::slotBeginReset()
{
    beginResetModel();
}

void QtIVIPropertyModel::slotEndReset()
{
    endResetModel();
}
