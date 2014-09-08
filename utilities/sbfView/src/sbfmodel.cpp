#include "sbfmodel.h"
#include <QFileInfo>
#include <QSettings>
#include "sbfNode.h"
#include "sbfElement.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkIntArray.h"

SbfModel::SbfModel(QObject *parent, QSettings *settings) :
    QObject(parent),
    settings_(settings)
{
    grid_ = vtkUnstructuredGrid::New();
    points_ = vtkPoints::New();
    cells_ = vtkCellArray::New();
    mesh_ = new sbfMesh;

    if(settings_){
        settings_->beginGroup("lastRead");
        QString lastInd = settings_->value("ind").toString();
        QString lastCrd = settings_->value("crd").toString();
        QString lastMtr = settings_->value("mtr").toString();
        settings_->endGroup();
        if(lastInd.size() && lastCrd.size() && lastMtr.size())
            if(QFileInfo(lastInd).exists() && QFileInfo(lastCrd).exists() && QFileInfo(lastMtr).exists())
                readModel(lastInd, lastCrd, lastMtr);
    }
}

int SbfModel::readModel(const QString &indName, const QString &crdName, const QString &mtrName)
{
    int status = mesh_->readMeshFromFiles(indName.toLocal8Bit(), crdName.toLocal8Bit(), mtrName.toLocal8Bit());
    points_ = vtkPoints::New();
    cells_ = vtkCellArray::New();
    const int numCellArray = grid_->GetCellData()->GetNumberOfArrays();
    for(int ct = numCellArray-1; ct >= 0; --ct) grid_->GetCellData()->RemoveArray(ct);
    const int numPointArray = grid_->GetPointData()->GetNumberOfArrays();
    for(int ct = numPointArray-1; ct >= 0; --ct) grid_->GetPointData()->RemoveArray(ct);
    //TODO emit error message if something wrong
    if(settings_ && status == 0) {
        settings_->beginGroup("lastRead");
        settings_->setValue("ind", indName);
        settings_->setValue("crd", crdName);
        settings_->setValue("mtr", mtrName);
        settings_->endGroup();
    }
    if(status == 0) {
        const int numNodes = mesh_->numNodes();
        points_->SetNumberOfPoints(numNodes);
        for(int ct = 0; ct < numNodes; ++ct) {
            auto n = mesh_->node(ct);
            points_->SetPoint(ct, n.x(), n.y(), n.z());
        }
        const int numElems = mesh_->numElements();
        std::vector<VTKCellType> cellTypes;
        cellTypes.reserve(numElems);
        vtkSmartPointer<vtkIntArray> mtrData(vtkIntArray::New());
        mtrData->SetName("materials");
        mtrData->SetNumberOfComponents(1);
        mtrData->SetNumberOfValues(numElems);
        for(int ct = 0; ct < numElems; ++ct) {
            auto elem = mesh_->elemPtr(ct);
            auto indexes = elem->indexes();
            indexes = sbfIndexesToVTK(elem->type(), indexes);
            auto vtkIndexes = std::vector<vtkIdType>(indexes.begin(), indexes.end());
            cells_->InsertNextCell(elem->numNodes(), vtkIndexes.data());
            cellTypes.push_back(sbfTypeToVTK(elem->type()));
            mtrData->SetValue(ct, elem->mtr()+1);
        }
        grid_->SetPoints(points_.GetPointer());
        grid_->SetCells(cellTypes.front(), cells_.GetPointer());
        grid_->GetCellData()->AddArray(mtrData);
        grid_->GetCellData()->SetScalars(mtrData);
    }
    return status;
}

//TODO this is copy from sbfToVTK - should be placed in some .h file
VTKCellType sbfTypeToVTK(ElementType type)
{
    //TODO add all types
    switch(type){
    case ElementType::BEAM_LINEAR_3DOF: return VTK_LINE;
    case ElementType::BEAM_QUADRATIC_3DOF: return VTK_QUADRATIC_EDGE;
    case ElementType::BEAM_LINEAR_6DOF: return VTK_LINE;
    case ElementType::BEAM_QUADRATIC_6DOF: return VTK_QUADRATIC_EDGE;
    case ElementType::HEXAHEDRON_LINEAR: return VTK_HEXAHEDRON;
    case ElementType::HEXAHEDRON_QUADRATIC: return VTK_QUADRATIC_HEXAHEDRON;
    case ElementType::TETRAHEDRON_LINEAR: return VTK_TETRA;
    case ElementType::TETRAHEDRON_QUADRATIC: return VTK_QUADRATIC_TETRA;
    default: break;
    }
    return VTK_EMPTY_CELL;
}

std::vector<int> sbfIndexesToVTK(ElementType type, std::vector<int> indexes)
{
    std::vector<int> swappedIndexes = indexes;
    //TODO add all types
    switch(type){
    case ElementType::TETRAHEDRON_LINEAR: break;
    case ElementType::TETRAHEDRON_QUADRATIC: break;//TODO check this type numbering
    case ElementType::HEXAHEDRON_LINEAR: break;
    case ElementType::HEXAHEDRON_QUADRATIC:{
        swappedIndexes[16] = indexes[12];
        swappedIndexes[17] = indexes[13];
        swappedIndexes[18] = indexes[14];
        swappedIndexes[19] = indexes[15];
        swappedIndexes[12] = indexes[16];
        swappedIndexes[13] = indexes[17];
        swappedIndexes[14] = indexes[18];
        swappedIndexes[15] = indexes[19];
    }
        break;
    default: break;
    }
    return swappedIndexes;
}
