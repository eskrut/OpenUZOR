#ifndef SBFDATAITEM_H
#define SBFDATAITEM_H

#include <QStandardItem>

class SbfDataItem : public QStandardItem
{
public:
    enum Type {
        Material = QStandardItem::UserType + 1,
        SELevels,
        FloatVector,
        DoubleVector,
        FloatScalar,
        DoubleScalar
    };

    enum VtkType {
        CellData,
        NodeData
    };

    enum Role {
        TypeRequest = Qt::UserRole + 1,
        VtkTypeRequest,
        SbfPointerRequest,
        VtkPointerRequest
    };

    explicit SbfDataItem(const QString &text, Type type, VtkType vtkType);

private:
    Type type_;
    VtkType vtkType_;
    void *sbfData_;
    void *vtkData_;
public:
    int type() const;
    QVariant data(int role) const;
    void setData(const QVariant &value, int role);
};

#endif // SBFDATAITEM_H
