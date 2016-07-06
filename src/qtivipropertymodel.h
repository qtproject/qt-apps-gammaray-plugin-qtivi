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

#ifndef GAMMARAY_QTIVI_QTIVIPROPERTYMODEL_H
#define GAMMARAY_QTIVI_QTIVIPROPERTYMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class QtIVIPropertyModel : public QAbstractTableModel
{
  Q_OBJECT

public:
    enum Column {
        AddressColumn,
        BackendValueColumn,
        OverrideValueColumn,
        OverrideColumn,
        /** Mark column count */
        ColumnCount
    };

    explicit QtIVIPropertyModel(QObject *parent = 0);
    ~QtIVIPropertyModel();

    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    void setSourceModel(QAbstractItemModel *sourceModel);

private slots:
    void slotBeginRemoveRows(const QModelIndex &parent, int start, int end);
    void slotEndRemoveRows();
    void slotBeginInsertRows(const QModelIndex &parent, int start, int end);
    void slotEndInsertRows();
    void slotBeginReset();
    void slotEndReset();

private:
    QVector<QPair<int, int>> m_pending_insertions;
    QAbstractItemModel *m_sourceModel;
};

#endif
