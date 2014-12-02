#ifndef SBFTOOLBAR_H
#define SBFTOOLBAR_H

#include <QToolBar>
#include <QComboBox>
#include <QToolButton>
#include <QLineEdit>
#include <QAction>
#include "sbfdatamodel.h"

class SbfToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit SbfToolBar(SbfDataModel *dataModel, QWidget *parent = 0);
private:
    SbfDataModel *dataModel_;
    QComboBox *dataBox_;
    QComboBox *componentBox_;
    QWidget *warpWidget_;
    QToolButton *warpOnButton_;
    QLineEdit *warpScaleLE_;
    QAction *wAct_;
public:
    QString currentName() const { return dataBox_->currentText(); }
    int currentComponent() const { return componentBox_->currentIndex() - 1; }
signals:
    void curentArrayChanged(QString);
    void curentComponentChanged(int);
    void warpFactorChanged(double);

public slots:
private slots:
    void onArrayChanged(int ID);

};

#endif // SBFTOOLBAR_H
