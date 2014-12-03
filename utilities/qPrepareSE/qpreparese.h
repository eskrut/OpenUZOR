#ifndef QPREPARESE_H
#define QPREPARESE_H

#include <QWidget>
#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDir>
#include <QString>
#include <QSettings>
#include "../prepareSE/sebuilder.h"

class QPrepareSE : public QDialog
{
    Q_OBJECT
public:
    explicit QPrepareSE(QWidget *parent = 0);
private:
    QDir catalogD_;
    QLabel catalogName_;
    QPushButton catalogB_;
    QLineEdit indFileName_;
    QString indFullFileName_;
    QLineEdit levelBaseName_;
    QLineEdit partitionLine_;
    QLineEdit maxImbalanceLine_;
    QCheckBox seedChB_;
    QCheckBox inverseChB_;
    QCheckBox nodeConnectionChb_;
    QPushButton make_;
    QSettings *settings_;

    SEBuilder *seBuilder_;
signals:
private slots:
    void onChangeDirPressed();
public slots:
    void onIndFileChanged(const QString &indFileName);
    void onMake();

};

#endif // QPREPARESE_H
