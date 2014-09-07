#ifndef SBFVIEW_H
#define SBFVIEW_H

#include "QVTKWidget.h"
#include "vtkSmartPointer.h"

class SbfModel;

class SbfView : public QVTKWidget
{
    Q_OBJECT
public:
    explicit SbfView(QWidget *parent = 0);
private:
    SbfModel *model_;
    vtkSmartPointer<vtkRenderer> renderer_;

public:
    void setModel(SbfModel *model);
signals:

public slots:

};

#endif // SBFVIEW_H
