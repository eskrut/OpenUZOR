#ifndef SBFVIEW_H
#define SBFVIEW_H

#include "QVTKWidget.h"
#include "vtkSmartPointer.h"
#include "vtkCamera.h"
#include "vtkActor.h"
#include "vtkScalarBarActor.h"
#include "vtkDataSetMapper.h"
#include <array>

class SbfModel;

class SbfView : public QVTKWidget
{
    Q_OBJECT
public:
    explicit SbfView(QWidget *parent = 0);
private:
    SbfModel *model_;
    vtkSmartPointer<vtkRenderer> renderer_;
    vtkSmartPointer<vtkCamera> cam_;
    vtkSmartPointer<vtkActor> actor_;
    vtkSmartPointer<vtkScalarBarActor> bar_;
    vtkSmartPointer<vtkDataSetMapper> mapper_;

public:
    void setModel(SbfModel *model);
signals:

public slots:
    void resetView();
    void setView(const std::array<double, 3> &position, const std::array<double, 3> &up);
    void setViewXY();
    void setViewYZ();
    void setViewZX();
    void setViewXYZ();
    bool edgeVisible();
    void setEdgeVisible(bool on);

    void setArrayToMap(QString name, int component = -1);
};

#endif // SBFVIEW_H
