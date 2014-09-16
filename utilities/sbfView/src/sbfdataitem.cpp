#include "sbfdataitem.h"

SbfDataItem::SbfDataItem(const QString &text, Type type, VtkType vtkType) :
    QStandardItem(text),
    type_(type),
    vtkType_(vtkType),
    sbfData_(nullptr),
    vtkData_(nullptr)
{
}

int SbfDataItem::type() const
{
    return static_cast<int>(type_);
}

QVariant SbfDataItem::data(int role) const
{
    switch (role) {
    case TypeRequest:
        return QVariant::fromValue(static_cast<int>(type_));
        break;
    case VtkTypeRequest:
        return QVariant::fromValue(static_cast<int>(vtkType_));
        break;
    case SbfPointerRequest:
        return qVariantFromValue(static_cast<void*>(sbfData_));
        break;
    case VtkPointerRequest:
        return qVariantFromValue(static_cast<void*>(vtkData_));
        break;
    default:
        return QStandardItem::data(role);
        break;
    }
}

void SbfDataItem::setData(const QVariant &value, int role)
{
    switch (role) {
    case TypeRequest:
        type_ = static_cast<Type>(value.toInt());
        break;
    case VtkTypeRequest:
        vtkType_ = static_cast<VtkType>(value.toInt());
        break;
    case SbfPointerRequest:
        sbfData_ = static_cast<void*>(value.value<void *>());
        break;
    case VtkPointerRequest:
        vtkData_ = static_cast<void*>(value.value<void *>());
        break;
    default:
        QStandardItem::setData(value, role);
        break;
    }
}
