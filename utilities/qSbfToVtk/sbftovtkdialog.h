#ifndef SBFTOVTKDIALOG_H
#define SBFTOVTKDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QSettings>
#include <QRegExp>
#include <QMessageBox>
#include "sbf.h"
#include "sbfToVTK.h"

namespace Ui {
class SbfToVtkDialog;
}

class SbfToVtkDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SbfToVtkDialog(QWidget *parent = 0);
    ~SbfToVtkDialog();
    
private slots:
    void on_catalogB_clicked();
    void updateMtrName();
    void updateCatalogContents();
    void on_chooseInd_clicked();
    void on_chooseCrd_clicked();
    void on_chooseMtr_clicked();
    void on_chooseAll_clicked();
    void on_chooseNodePB_clicked();
    void on_chooseBundlePB_clicked();
    void on_convertB_clicked();
    void on_cancelB_clicked();
public slots:
    void setCatalog(QString);

private:
    Ui::SbfToVtkDialog *ui;

    QString catalog;
    QString indFileName;
    QString crdFileName;
    QString mtrFileName;
    QString levelBaseName;
    QString vtkFileName;
    QSettings *settings;
    QString patternRE;
    QStringList stdNamesNode, stdNamesBundle;

    int numLastCatalogs;
};

#endif // SBFTOVTKDIALOG_H
