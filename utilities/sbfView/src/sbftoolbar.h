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
    QComboBox *componentBox_;
public:
    QString currentName() const { return dataBox_->currentText(); }
    int currentComponent() const { return componentBox_->currentIndex() - 1; }
signals:
    void curentArrayChanged(QString);
    void curentComponentChanged(int);

public slots:

};

#endif // SBFTOOLBAR_H
