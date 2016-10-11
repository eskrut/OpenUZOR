#ifndef SBFVIEW_H
#define SBFVIEW_H

#include "QVTKWidget.h"
#include "vtkSmartPointer.h"
#include "vtkCamera.h"
#include "vtkActor.h"
#include "vtkScalarBarActor.h"
#include "vtkDataSetMapper.h"
#include "vtkWarpVector.h"
#include "vtkLookupTable.h"
#include "vtkGeometryFilter.h"
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
    vtkSmartPointer<vtkLookupTable> lt_;
    vtkSmartPointer<vtkLookupTable> matLt_;
    vtkSmartPointer<vtkDataSetMapper> mapper_;
    vtkSmartPointer<vtkWarpVector> warp_;
    double warpFactor_;

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
    void rotateView(const std::array<double, 3> &axis, double angle);
    void rotateViewScreenX(double angle);
    void rotateViewScreenY(double angle);
    bool edgeVisible();
    void setEdgeVisible(bool on);

    double warpFactor() const;
    void setWarpFactor(double factor);

    void setArrayToMap(QString name, int component = -1,
                       double low = std::numeric_limits<double>::quiet_NaN(),
                       double high = std::numeric_limits<double>::quiet_NaN());
private:
    void fillMtrLt(vtkDataArray *array);
};

#endif // SBFVIEW_H
