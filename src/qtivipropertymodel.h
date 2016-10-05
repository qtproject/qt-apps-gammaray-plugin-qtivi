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

#include "qtivipropertyoverrider.h"

#include <QAbstractItemModel>
#include <QHash>
#include <QString>

#include <vector>

class QIviProperty;

namespace GammaRay {

class Probe;

class QtIviPropertyModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QtIviPropertyModel(Probe *probe);

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private slots:
    void objectAdded(QObject *obj);
    void objectRemoved(QObject *obj);
    void objectReparented(QObject *obj);
    void propertyValueChanged(const QVariant &value);

public:
    struct IviProperty // public so we can have a free function in the .cpp file dealing with it
    {
        explicit IviProperty(QIviProperty *value, const QMetaProperty &metaProperty);
        IviProperty();
        QString name;
        QIviProperty *value;
        QtIviPropertyOverrider overrider;
    };

private:
    int indexOfPropertyCarrier(const QObject *carrier) const;

    struct IviPropertyCarrier
    {
        QObject *carrier;
        std::vector<IviProperty> iviProperties;
        int indexOfProperty(const QIviProperty *property) const;
    };

    /// property tree model
    // "property carriers" are objects that have QIviProperty (static Qt meta) properties

    std::vector<IviPropertyCarrier> m_propertyCarriers;
    // m_seenObjects is not strictly needed currently but it helps debugging and will become more useful
    // when some of the current simplifying assumptions about object relationships must be discarded.
    QHash<QObject *, bool> m_seenObjects; // bool meaning: whether it has properties of type QtIVIProperty *
};

}

#endif
