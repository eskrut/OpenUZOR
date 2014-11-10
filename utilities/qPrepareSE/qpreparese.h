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
    QPushButton make_;

    SEBuilder *seBuilder_;
signals:
private slots:
    void onChangeDirPressed();
public slots:
    void onDirChanged(const QString &dirName);
    void onMake();

};

#endif // QPREPARESE_H
