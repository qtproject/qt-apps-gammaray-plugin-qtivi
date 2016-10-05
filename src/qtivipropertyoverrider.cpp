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

#include "qtivipropertyoverrider.h"

#include <QObject>

#include <QIviProperty>
#include <1.0.0/QtIviCore/private/qiviproperty_p.h>

/*
 For the purpose of overriding, there is a big difference between read-only and read-write properties.
 Read-write properties are effectively settings for lower layers from higher layers, with reading them
 back as a convenience function.
 Read-only properties are information about lower layers for higher layers.
 So the information flow is exactly opposite for the two kinds of properties. Thus, overriding should
 work as follows for them:
 - Read-write properties: Overriding allows the overrider to set the value, with the same effects as
   setting the value from the UI. The regular setter will not  change the value anymore, but stash away
   the value for later when override is disabled.
 - Readonly properties: Overriding allows the overrider to set the value, with the same effects as
   setting the value from the backend.

 Unfortunately, QIviProperties cannot currently be explicitly readonly - they can only ignore
 setValue() calls...
 */

// Note: Those subclasses of QSlotObjectBase are not invoked through the ReadProperty / WriteProperty
// "opcodes" of the metaobject system, they are just regular methods, for which the calling convention
// is that a[0] points to the return value, a[1] to the first argument, a[2] to the second etc.

class OverrideValueSetter : public QtPrivate::QSlotObjectBase
{
    static void impl(int which, QSlotObjectBase *this_, QObject *r, void **a, bool *ret)
    {
        switch (which) {
        case Destroy:
            delete static_cast<OverrideValueSetter*>(this_);
            break;
        case Call: {
#if 0       // If we actually wanted to propagate writes coming from the real backend - which would
            // undo the override... It seems better to ignore writes from the real backend.
            QtIviPropertyOverrider *const overrider = static_cast<OverrideValueSetter*>(this_)->m_overrider;
            const QMetaType::Type mt = static_cast<QMetaType::Type>(overrider->m_overrideValue.userType());
            void *const firstArgument = a[1];
            overrider->m_overrideValue = QVariant(mt, firstArgument);
#endif
            break;
        }
        case Compare:
            *ret = false; // not implemented
            break;
        case NumOperations: ;
        }
    }
public:
    QtIviPropertyOverrider *m_overrider;
    explicit OverrideValueSetter(QtIviPropertyOverrider *overrider)
        : QSlotObjectBase(&impl), m_overrider(overrider) {}
};

class OverrideValueGetter : public QtPrivate::QSlotObjectBase
{
    static void impl(int which, QSlotObjectBase *this_, QObject *r, void **a, bool *ret)
    {
        Q_UNUSED(r)
        switch (which) {
        case Destroy:
            delete static_cast<OverrideValueGetter*>(this_);
            break;
        case Call: {
            QtIviPropertyOverrider *const overrider = static_cast<OverrideValueGetter*>(this_)->m_overrider;
            const QMetaType::Type mt = static_cast<QMetaType::Type>(overrider->m_overrideValue.userType());
            void *const returnValue = a[0];
            // There is no QMetaType::assign(), but we can still avoid re-allocating the storage.
            // No reallocation is a hard requirement in case the instance is not heap-allocated.
            QMetaType::destruct(mt, returnValue);
            QMetaType::construct(mt, returnValue, overrider->m_overrideValue.constData());
            break;
        }
        case Compare:
            *ret = false; // not implemented
            break;
        case NumOperations: ;
        }
    }
public:
    QtIviPropertyOverrider *m_overrider;
    explicit OverrideValueGetter(QtIviPropertyOverrider *overrider)
        : QSlotObjectBase(&impl), m_overrider(overrider) {}
};

QtIviPropertyOverrider::QtIviPropertyOverrider(QIviProperty *overriddenProperty, bool userWritable)
   : m_prop(overriddenProperty),
     m_overrideEnabled(false),
     m_userWritable(userWritable)
{
    // access to m_overrideValue[G|S]etter is guarded by m_overrideEnabled so they don't need to be
    // initialized
}

QtIviPropertyOverrider::QtIviPropertyOverrider()
   : m_prop(nullptr),
     m_overrideEnabled(false),
     m_userWritable(false)
{
}

QtIviPropertyOverrider::QtIviPropertyOverrider(QtIviPropertyOverrider &&other)
{
    *this = std::move(other);
}

QtIviPropertyOverrider &QtIviPropertyOverrider::operator=(QtIviPropertyOverrider &&other)
{
    m_prop = other.m_prop;
    m_overrideValue = other.m_overrideValue;
    m_overrideEnabled = other.m_overrideEnabled;
    m_originalValueSetter = other.m_originalValueSetter;
    m_originalValueGetter = other.m_originalValueGetter;

    if (m_overrideEnabled) {
        // fix the backlinks
        QIviPropertyPrivate *pPriv = QIviPropertyPrivate::get(m_prop);
        static_cast<OverrideValueSetter *>(pPriv->m_valueSetter)->m_overrider = this;
        static_cast<OverrideValueGetter *>(pPriv->m_valueGetter)->m_overrider = this;

        // this is enough to make destroying the other one harmless
        other.m_overrideEnabled = false;
    }
    return *this;
}

QtIviPropertyOverrider::~QtIviPropertyOverrider()
{
    disableOverride();
}

void QtIviPropertyOverrider::setOverrideValue(const QVariant &value)
{
    if (!m_overrideEnabled) {
        m_overrideEnabled = true;
        QIviPropertyPrivate *pPriv = QIviPropertyPrivate::get(m_prop);
        m_originalValueGetter = pPriv->m_valueGetter;
        m_originalValueSetter = pPriv->m_valueSetter;

        pPriv->m_valueGetter = new OverrideValueGetter(this);
        pPriv->m_valueSetter = new OverrideValueSetter(this);
    }
    m_overrideValue = value;
}

void QtIviPropertyOverrider::disableOverride()
{
    if (m_overrideEnabled) {
        m_overrideEnabled = false;
        QIviPropertyPrivate *pPriv = QIviPropertyPrivate::get(m_prop);
        // ### should we really call destroyIfLastRef()?
        pPriv->m_valueGetter->destroyIfLastRef();
        pPriv->m_valueSetter->destroyIfLastRef();
        pPriv->m_valueGetter = m_originalValueGetter;
        pPriv->m_valueSetter = m_originalValueSetter;
    }
}

bool QtIviPropertyOverrider::overrideEnabled() const
{
    return m_overrideEnabled;
}

QVariant QtIviPropertyOverrider::value() const
{
    return m_prop->value();
}
