#ifndef SBFMODEL_H
#define SBFMODEL_H

#include <QObject>
#include <QString>
#include "vtkSmartPointer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkBoxClipDataSet.h"
#include "vtkClipPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkDataArray.h"
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
    vtkSmartPointer<vtkClipPolyData/*vtkBoxClipDataSet*/> clipped_;
    std::array<float, 6> clipBoxBounds_;
    vtkSmartPointer<vtkPoints> points_;
    vtkSmartPointer<vtkCellArray> cells_;
    sbfMesh *mesh_;
    SbfDataModel *dataModel_;

public:
    const sbfMesh *mesh() const { return mesh_; }
    int readModel(const QString &indName, const QString &crdName, const QString &mtrName);
    int reReadLast();
    enum class GessType {
        None = 0,
        NodeFloat,
        NodeDouble,
        SolBundleFloat,
        SolBundleDouble
    };
    void addData(const QString &fileName,
                 const QString &arrayName = QString(),
                 GessType gType = GessType::None,
                 int numPlaceholders = -1,
                 const std::list<std::string> &names = std::list<std::string>({})
                 );
    vtkDataArray *data(const QString &arrayName) const;
    vtkSmartPointer<vtkUnstructuredGrid> grid() const { return grid_;}
    vtkSmartPointer<vtkPolyData> clipped() const { return /*clipped_->GetOutput();*/clipped_->GetClippedOutput(); }
    vtkPoints *points() const {return points_;}
    vtkCellArray *cells() const {return cells_;}
    SbfDataModel *dataModel() const { return dataModel_; }

    void setClipBounds(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);
    void unsetClipBounds();
    void clipXPlus();
    void clipXMinus();
    void clipYPlus();
    void clipYMinus();
    void clipZPlus();
    void clipZMinus();

    //FIXME this function added to resolve update of arrays in clipped after update in grid
    //should be replaced with auto update
    void updateClippedData();

private:
    void updateClipped();
signals:

public slots:

};

//TODO this is copy from sbfToVTK - should be placed in some .h file
VTKCellType sbfTypeToVTK(ElementType type);
std::vector<int> sbfIndexesToVTK(ElementType type, std::vector<int> indexes);

#endif // SBFMODEL_H
