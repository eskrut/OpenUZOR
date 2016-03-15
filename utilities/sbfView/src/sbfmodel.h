#ifndef SBFMODEL_H
#define SBFMODEL_H

#include <QObject>
#include <QString>
#include "vtkSmartPointer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkBoxClipDataSet.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "sbfMesh.h"
#include <array>

class QSettings;
class SbfDataModel;

class SbfModel : public QObject
{
    Q_OBJECT
public:
    explicit SbfModel(QObject *parent = nullptr, QSettings *settings = nullptr);
private:
    QSettings *settings_;
    vtkSmartPointer<vtkUnstructuredGrid> grid_;
    vtkSmartPointer<vtkBoxClipDataSet> clipped_;
    std::array<float, 6> clipBoxBounds_;
    vtkSmartPointer<vtkPoints> points_;
    vtkSmartPointer<vtkCellArray> cells_;
    sbfMesh *mesh_;
    SbfDataModel *dataModel_;

public:
    const sbfMesh *mesh() const { return mesh_; }
    int readModel(const QString &indName, const QString &crdName, const QString &mtrName);
    void addData(const QString &fileName, const QString &arrayName = QString());
    vtkSmartPointer<vtkUnstructuredGrid> grid() const { return clipped_->GetOutput(); }
    SbfDataModel *dataModel() const { return dataModel_; }

    void setClipBounds(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);
    void unsetClipBounds();
    void clipXPlus();
    void clipXMinus();
    void clipYPlus();
    void clipYMinus();
    void clipZPlus();
    void clipZMinus();
private:
    void updateClipped();
signals:

public slots:

};

//TODO this is copy from sbfToVTK - should be placed in some .h file
VTKCellType sbfTypeToVTK(ElementType type);
std::vector<int> sbfIndexesToVTK(ElementType type, std::vector<int> indexes);

#endif // SBFMODEL_H
