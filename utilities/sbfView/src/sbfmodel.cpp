#include "sbfmodel.h"
#include <QFileInfo>
#include <QSettings>
#include "sbfNode.h"
#include "sbfElement.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkIntArray.h"
#include "vtkFloatArray.h"
#include "sbfdatamodel.h"
#include "sbfdataitem.h"

SbfModel::SbfModel(QObject *parent, QSettings *settings) :
    QObject(parent),
    settings_(settings)
{
    grid_ = vtkUnstructuredGrid::New();
    points_ = vtkPoints::New();
    cells_ = vtkCellArray::New();
    mesh_ = new sbfMesh;

    dataModel_ = new SbfDataModel(this);

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
            mtrData->SetValue(ct, elem->mtr());
        }
        grid_->SetPoints(points_.GetPointer());
        grid_->SetCells(cellTypes.front(), cells_.GetPointer());
        grid_->GetCellData()->AddArray(mtrData);
        grid_->GetCellData()->SetScalars(mtrData);

//        SbfDataItem *noneItem = new SbfDataItem("None", SbfDataItem::None, SbfDataItem::NodeData);
//        dataModel_->invisibleRootItem()->appendRow(noneItem);
        SbfDataItem *mtrDataItem = new SbfDataItem(mtrData->GetName(), SbfDataItem::Material, SbfDataItem::CellData);
        dataModel_->invisibleRootItem()->appendRow(mtrDataItem);
    }
    return status;
}

void SbfModel::addData(const QString &fileName)
{
    std::string catalog, baseName, suf;
    int numDigits;
    auto f = fileName;
    f = f.replace("\\", "/");
    QFileInfo fi(f);
    catalog = fi.absolutePath().toStdString();
    suf = fi.suffix().toStdString();
    QRegExp re("(0+)(\\d+)$");
    int pos = re.indexIn(fi.baseName());
    if (pos < 0)
        throw std::runtime_error(("Cant parce file name" + fileName.toStdString()).c_str());
    QString leadingZeros = re.cap(1);
    QString count = re.cap(2);
    numDigits = leadingZeros.size() + count.size();
    baseName = fi.baseName().toStdString();
    baseName.erase(baseName.size()-numDigits);
    const int numNodes = mesh_->numNodes();
    try {
        NodesData<float, 3> *dataF = new NodesData<float, 3>(fileName.toStdString(), mesh_);
        if(dataF->readFromFile(baseName.c_str(), count.toInt(), ".sba", numDigits, catalog.c_str()) != 0)
            throw std::runtime_error("not this type");
        mesh_->addFVData(dataF);
        vtkFloatArray *data(vtkFloatArray::New());
        data->SetName(baseName.c_str());
        data->SetNumberOfComponents(3);
        data->SetNumberOfTuples(numNodes);
        for(int ct = 0; ct < 3; ++ct) for(int ctNode = 0; ctNode < numNodes; ++ctNode)
            data->SetComponent(ctNode, ct, dataF->data(ctNode, ct));
        grid_->GetPointData()->AddArray(data);
        auto item = new SbfDataItem(baseName.c_str(), SbfDataItem::FloatVector, SbfDataItem::NodeData);
        item->setData(qVariantFromValue(static_cast<void*>(dataF)), SbfDataItem::SbfPointerRequest);
        item->setData(qVariantFromValue(static_cast<void*>(data)), SbfDataItem::VtkPointerRequest);
        dataModel_->invisibleRootItem()->appendRow(item);
        return;
    }
    catch(...) {
    }
    try {
        NodesData<double, 3> *dataD = new NodesData<double, 3>(fileName.toStdString(), mesh_);
        if(dataD->readFromFile(baseName.c_str(), count.toInt(), ".sba", numDigits, catalog.c_str()) != 0)
            throw std::runtime_error("not this type");
        mesh_->addDVData(dataD);
        return;
    }
    catch(...) {
    }
    try {
        const int numArrays = 20;
        SolutionBundle<float, numArrays> *Fsol = new SolutionBundle<float>(fileName.toStdString(), mesh_->numNodes());
        if(Fsol->readFromFile(baseName.c_str(), count.toInt(), ".sba", numDigits, catalog.c_str()) != 0)
            throw std::runtime_error("not this type");
        mesh_->addSolutionBundle(Fsol);
        for(int ct = 0; ct < numArrays; ++ct) {
            auto array = Fsol->array(ct);
            if(array) {
                vtkFloatArray *data(vtkFloatArray::New());
                data->SetName((baseName+"/"+Fsol->name(ct)).c_str());
                data->SetNumberOfComponents(1);
                data->SetNumberOfTuples(numNodes);
                for(int ctNode = 0; ctNode < numNodes; ++ctNode)
                    data->SetComponent(ctNode, 1, array->data(ctNode, 1));
                grid_->GetPointData()->AddArray(data);
                auto item = new SbfDataItem(data->GetName(), SbfDataItem::FloatScalar, SbfDataItem::NodeData);
                item->setData(qVariantFromValue(static_cast<void*>(array)), SbfDataItem::SbfPointerRequest);
                item->setData(qVariantFromValue(static_cast<void*>(data)), SbfDataItem::VtkPointerRequest);
                dataModel_->invisibleRootItem()->appendRow(item);
            }
        }
        return;
    }
    catch(...) {
    }
    try {
        SolutionBundle<double> *Dsol = new SolutionBundle<double>(fileName.toStdString(), mesh_->numNodes());
        if(Dsol->readFromFile(baseName.c_str(), count.toInt(), ".sba", numDigits, catalog.c_str()) != 0)
            throw std::runtime_error("not this type");
        mesh_->addSolutionBundle(Dsol);

        return;
    }
    catch(...) {
    }
    throw std::runtime_error(("Cant interpret file " + fileName.toStdString()).c_str());
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
