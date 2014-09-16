#ifndef SBFMODEL_H
#define SBFMODEL_H

#include <QObject>
#include <QString>
#include "vtkSmartPointer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "sbfMesh.h"

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
    vtkSmartPointer<vtkPoints> points_;
    vtkSmartPointer<vtkCellArray> cells_;
    sbfMesh *mesh_;
    SbfDataModel *dataModel_;

public:
    const sbfMesh *mesh() const { return mesh_; }
    int readModel(const QString &indName, const QString &crdName, const QString &mtrName);
    void addData(const QString &fileName);
    vtkSmartPointer<vtkUnstructuredGrid> grid() const { return grid_; }
    SbfDataModel *dataModel() const { return dataModel_; }
signals:

public slots:

};

//TODO this is copy from sbfToVTK - should be placed in some .h file
VTKCellType sbfTypeToVTK(ElementType type);
std::vector<int> sbfIndexesToVTK(ElementType type, std::vector<int> indexes);

#endif // SBFMODEL_H
