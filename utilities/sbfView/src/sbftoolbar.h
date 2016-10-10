#ifndef SBFTOOLBAR_H
#define SBFTOOLBAR_H

#include <QToolBar>
#include <QComboBox>
#include <QToolButton>
#include <QLineEdit>
#include <QAction>
#include <QSortFilterProxyModel>
#include "sbfdatamodel.h"

class ProxyFilter;

class SbfToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit SbfToolBar(SbfDataModel *dataModel, QWidget *parent = 0);
private:
    SbfDataModel *dataModel_;
    ProxyFilter *proxy_;
    QComboBox *dataBox_;
    QComboBox *componentBox_;
    QWidget *warpWidget_;
    QToolButton *warpOnButton_;
    QLineEdit *warpScaleLE_;
    QAction *wAct_;
public:
    QString currentName() const;
    int currentComponent() const { return componentBox_->currentIndex() - 1; }
    void addAllowed(const QString &field);
signals:
    void curentArrayChanged(QString);
    void curentComponentChanged(int);
    void warpFactorChanged(double);

public slots:
private slots:
    void onArrayChanged(int ID);

};

class ProxyFilter : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ProxyFilter(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}
private:
    QStringList allowed_;
    QStringList rejected_;

public:
    void addAllowed(const QString &field) {
        allowed_ << field;
        invalidateFilter();
    }

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
    {
        auto cur = sourceModel()->data(sourceModel()->index(source_row, 0, source_parent)).toString();
        if(allowed_.size() && allowed_.contains(cur, Qt::CaseInsensitive) && ! rejected_.contains(cur, Qt::CaseInsensitive))
            return true;
        else if(rejected_.contains(cur, Qt::CaseInsensitive))
            return false;
        return false;
    }
};

#endif // SBFTOOLBAR_H
