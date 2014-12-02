#ifndef SBFVIEWMAINWINDOW_H
#define SBFVIEWMAINWINDOW_H

#include <QMainWindow>
#include "sbfview.h"
#include "sbfmodel.h"
#include "sbftoolbar.h"

class SbfViewMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit SbfViewMainWindow(QWidget *parent = 0);
    ~SbfViewMainWindow();
private:
    SbfModel *model_;
    SbfView *view_;
    SbfToolBar *toolBar_;
    QSettings *settings_;

    void initializeShortCuts();
    void initializeConnections();

signals:

public slots:

};

#endif // SBFVIEWMAINWINDOW_H
