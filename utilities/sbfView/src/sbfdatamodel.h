#ifndef SBFDATAMODEL_H
#define SBFDATAMODEL_H

#include <QStandardItemModel>
#include "sbfdataitem.h"

class SbfDataModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit SbfDataModel(QObject *parent = 0);

public:
    QList<SbfDataItem *> items(SbfDataItem::Type type = SbfDataItem::Type::Any) const;
    QStringList names(SbfDataItem::Type type = SbfDataItem::Type::Any) const;
signals:

public slots:

};

#endif // SBFDATAMODEL_H
