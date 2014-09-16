#ifndef SBFTOOLBAR_H
#define SBFTOOLBAR_H

#include <QToolBar>
#include <QComboBox>
#include "sbfdatamodel.h"

class SbfToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit SbfToolBar(SbfDataModel *dataModel, QWidget *parent = 0);
private:
    QComboBox *dataBox_;
signals:
    void curentArrayChanged(QString);

public slots:

};

#endif // SBFTOOLBAR_H
