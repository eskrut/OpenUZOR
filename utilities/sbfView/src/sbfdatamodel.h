#ifndef SBFDATAMODEL_H
#define SBFDATAMODEL_H

#include <QStandardItemModel>

class SbfDataModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit SbfDataModel(QObject *parent = 0);

signals:

public slots:

};

#endif // SBFDATAMODEL_H
