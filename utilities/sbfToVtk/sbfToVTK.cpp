#include "sbfToVTK.h"

#include <memory>
#include <algorithm>
#include <limits>

#include "vtkDataSet.h"
#include <vtkUnstructuredGrid.h>
#include <vtkSmartPointer.h>
#include <vtkXMLReader.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>
//#include <vtkXMLPUnstructuredGridReader.h>
//#include <vtkXMLPUnstructuredGridWriter.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkCellTypes.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtksys/SystemTools.hxx>

VTKCellType sbfTypeToVTK(ElementType type);
std::vector<int> sbfIndexesToVTK(ElementType type, std::vector<int> indexes);

sbfToVTKWriter::sbfToVTKWriter():
    indName_("ind.sba"),
    crdName_("crd.sba"),
    mtrName_(""),//This should be constructed by mtrBaseName_, mtrNumDigits_ and sbaExtention_
    catalog_("./"),
    vtkName_("vtk.vtu"),
    mtrBaseName_("mtr"),
    namePrefix_(""),
    mtrNumDigits_(3),
    levelBaseName_("level"),
    levelNumDigits_(3),
    sbaExtention_("sba"),
    solutionBundleNames_(std::list<SbaNameParts> () ),
    nodesDataNames_(std::list<SbaNameParts> () ),
    flagValidNames_(false),
    flagSaveLevels_(true),
    stepsToWrite_(0),
    flagUseCompression_(true)
{
    if(!catalog_.empty() && !(*catalog_.rbegin() != '/' || *catalog_.rbegin() != '\\'))
        catalog_ += "/";
}

sbfToVTKWriter::~sbfToVTKWriter()
{

}

void sbfToVTKWriter::check()
{
    // Check base files
    std::stringstream fileNameStream;
    std::ifstream testFile;

    std::list<std::string> namesToCheck;
    namesToCheck.push_back(namePrefix_ + indName_);
    namesToCheck.push_back(namePrefix_ + crdName_);
    fileNameStream.str("");
    fileNameStream << namePrefix_ + mtrBaseName_ << std::setw(mtrNumDigits_) << std::setfill('0') << 1 << "." << sbaExtention_;
    mtrName_ = fileNameStream.str();
    namesToCheck.push_back(mtrName_);

    flagValidNames_ = true;
    for(std::list<std::string>::iterator it = namesToCheck.begin(); it != namesToCheck.end() && flagValidNames_ != false; it++){
        fileNameStream.str("");
        fileNameStream << catalog_ << *it;
        testFile.open(fileNameStream.str().c_str(), std::ios_base::binary);
        if(!testFile.good()) flagValidNames_ = false;
        testFile.close();
        fileNameStream.str("");
    }
    bool flagNextDataExisted = true;
    int stepCount = 1, numSteps = 0;
    while(flagNextDataExisted){
        bool flagSomeFileReadSuccess = false;
        for(std::list<SbaNameParts>::iterator it = nodesDataNames_.begin(); it != nodesDataNames_.end(); it++){
            const int numComponents = 3;
            std::auto_ptr<NodesData<sbfReadWriteType, numComponents> > nodesData(new NodesData<sbfReadWriteType, numComponents>(catalog_+(*it).base, 0));
            nodesData->setNumDigits((*it).numDigits);
            nodesData->setStep(stepCount);
            if(nodesData->exist()) flagSomeFileReadSuccess = true;
        }
        for(std::list<SbaNameParts>::iterator it = solutionBundleNames_.begin(); it != solutionBundleNames_.end(); it++){
            const int numArrays = 20;
            std::auto_ptr<SolutionBundle<sbfReadWriteType, numArrays> > solutionBundle(new SolutionBundle<sbfReadWriteType, numArrays>(catalog_+(*it).base, 0));
            solutionBundle->setNumDigits((*it).numDigits);
            solutionBundle->setStep(stepCount);
            if(solutionBundle->exist()) flagSomeFileReadSuccess = true;
        }
        //check mtr files
        fileNameStream.str("");
        fileNameStream << catalog_ << "/" << namePrefix_ + mtrBaseName_ << std::setw(mtrNumDigits_) << std::setfill('0') << stepCount << "." << sbaExtention_;
        std::ifstream mtrTest(fileNameStream.str().c_str(), std::ios_base::binary);
        if(mtrTest.good()){
            flagSomeFileReadSuccess = true;
            mtrTest.close();
        }
        if(flagSomeFileReadSuccess) {stepCount++;numSteps++;}
        else flagNextDataExisted = false;
    }
    stepsToWrite_ = numSteps;
}

int sbfToVTKWriter::write()
{
    cout << "Started" << endl;
    check();
    if(!flagValidNames_) return 1;

    std::auto_ptr<sbfMesh> mesh(new sbfMesh ());
    std::stringstream fullIName, fullCName, fullMName;
    fullIName << catalog_ << namePrefix_ + indName_;
    fullCName << catalog_ << namePrefix_ + crdName_;
    fullMName << catalog_ << mtrName_;
    if( mesh->readMeshFromFiles(fullIName.str().c_str(), fullCName.str().c_str(), fullMName.str().c_str()) != 0 )
        return 2;
    mesh->printInfo();

    vtkUnstructuredGrid * grid = vtkUnstructuredGrid::New();
    vtkCellArray * cells = vtkCellArray::New();
    vtkPoints * points = vtkPoints::New();

    points->SetNumberOfPoints(mesh->numNodes());
    for(int ct = 0; ct < mesh->numNodes(); ct++) { sbfNode node = mesh->node(ct); points->SetPoint(ct, node.x() , node.y(), node.z()); }
    grid->SetPoints(points);

    std::vector <int> cellTypes;
    cellTypes.reserve(mesh->numElements());
    for(int ct = 0; ct < mesh->numElements(); ct++){
        sbfElement *elem = mesh->elemPtr(ct);
        cellTypes.push_back(sbfTypeToVTK(elem->type()));
        cells->InsertNextCell(elem->numNodes());
        std::vector<int> indexes = sbfIndexesToVTK(elem->type(), elem->indexes());
        for(size_t ct1 = 0; ct1 < indexes.size(); ct1++) cells->InsertCellPoint(indexes[ct1]);
    }
    grid->SetCells(&cellTypes[0], cells);

    vtkXMLUnstructuredGridWriter * writer = vtkXMLUnstructuredGridWriter::New();
    writer->SetInputData(grid);
    std::stringstream fullVTKName;
    fullVTKName << catalog_ << vtkName_/* << "." << writer->GetDefaultFileExtension()*/;
    writer->SetFileName(fullVTKName.str().c_str());
    writer->SetByteOrderToLittleEndian();
    if(flagUseCompression_)
        writer->SetCompressorTypeToZLib();
    else
        writer->SetCompressorTypeToNone();
    writer->SetDataModeToAppended();

    grid->GetPointData()->AllocateArrays(nodesDataNames_.size());

    const int numComponents = 3;
    for(std::list<SbaNameParts>::iterator it = nodesDataNames_.begin(); it != nodesDataNames_.end(); it++){
        vtkFloatArray * array = vtkFloatArray::New();
        array->SetName((*it).base.c_str());
        array->SetNumberOfComponents(numComponents);
        array->SetNumberOfTuples(mesh->numNodes());
        for(int ct = 0; ct < mesh->numNodes(); ct++) for(int ct1 = 0; ct1 < numComponents; ct1++) array->SetComponent(ct, ct1, std::numeric_limits<sbfReadWriteType>::quiet_NaN());
        grid->GetPointData()->AddArray(array);
    }
    const int nuArrays = 20;

    //write SE level info if exist
    std::auto_ptr<sbfSELevelList> seList(new sbfSELevelList());
    seList->setMesh(mesh.get());
    seList->readFromFiles((catalog_ + levelBaseName_).c_str(), levelNumDigits_);
    std::vector<sbfSElement *> fakeSElements;//Regular elements
    std::vector< std::vector<sbfSElement *> > selements;
    selements = seList->selevels( & fakeSElements);
    std::vector< std::vector<int> > levelIndexes;
    levelIndexes.resize(seList->numLevels());
    for(int ct = 0; ct < seList->numLevels(); ct++){
        levelIndexes[ct].resize(mesh->numElements());
        vtkIntArray * array = vtkIntArray::New();
        std::stringstream name;
        name << levelBaseName_ << ct+1;
        array->SetName(name.str().c_str());
        array->SetNumberOfComponents(1);
        array->SetNumberOfTuples(mesh->numElements());
        for(int ct1 = 0; ct1 < mesh->numElements(); ct1++){
            sbfSElement * se = fakeSElements.at(ct1);
            for(int ct2 = 0; ct2 <= ct; ct2++) se = se->parent();
            array->SetComponent(ct1, 0, se->index());
        }
        grid->GetCellData()->AddArray(array);
    }

    if(stepsToWrite_ > 0){
        cout << "Will write " << stepsToWrite_ << " steps:" << endl;
        report.createNewProgress("Writing time steps");
        writer->SetNumberOfTimeSteps(stepsToWrite_);
    }

    writer->Start();
    if(stepsToWrite_ > 0){
        bool flagNextDataExisted = true;
        int stepCount = 1;
        while(flagNextDataExisted && stepCount <= stepsToWrite_){
//            cout << "step " << stepCount << " of " << stepsToWrite_ << endl;
            report.updateProgress(0, stepsToWrite_, stepCount);
            bool flagSomeFileReadSuccess = false;
            for(std::list<SbaNameParts>::iterator it = nodesDataNames_.begin(); it != nodesDataNames_.end(); it++){//Process nodes data
                std::unique_ptr<NodesData<sbfReadWriteType, numComponents> > nodesData(new NodesData<sbfReadWriteType, numComponents>(catalog_+(*it).base, mesh.get()));
                nodesData->setNumDigits((*it).numDigits);
                nodesData->setStep(stepCount);
                int readRez = nodesData->readFromFile<sbfReadWriteType>();
                if(readRez == 0){
                    int index;
                    vtkDataArray * darray = grid->GetPointData()->GetArray((*it).base.c_str(), index);
                    if(index > -1) darray->Delete();
                    //grid->GetPointData()->RemoveArray((*it).c_str());
                    vtkFloatArray * array = vtkFloatArray::New();
                    array->SetName((*it).base.c_str());
                    array->SetNumberOfComponents(numComponents);
                    array->SetNumberOfTuples(mesh->numNodes());
                    flagSomeFileReadSuccess = true;
                    for(int ct = 0; ct < mesh->numNodes(); ct++) for(int ct1 = 0; ct1 < numComponents; ct1++) array->SetComponent(ct, ct1, nodesData->data(ct, ct1));
                    //for(int ct = 0; ct < mesh->numNodes(); ct++) for(int ct1 = 0; ct1 < numComponents; ct1++) cout << nodesData->data(ct, ct1) << endl;
                    grid->GetPointData()->AddArray(array);
                }
                else{
                    nodesData->setStep(stepCount-1);
                    if(nodesData->exist()){
                        int index;
                        vtkDataArray * darray = grid->GetPointData()->GetArray((*it).base.c_str(), index);
                        if(index > -1) darray->Delete();
                        //grid->GetPointData()->RemoveArray((*it).c_str());
                        vtkFloatArray * array = vtkFloatArray::New();
                        array->SetName((*it).base.c_str());
                        array->SetNumberOfComponents(numComponents);
                        array->SetNumberOfTuples(mesh->numNodes());
                        for(int ct = 0; ct < mesh->numNodes(); ct++) for(int ct1 = 0; ct1 < numComponents; ct1++) array->SetComponent(ct, ct1, std::numeric_limits<sbfReadWriteType>::quiet_NaN());
                        grid->GetPointData()->AddArray(array);
                    }
                }
            }//Process nodes data

            for(std::list<SbaNameParts>::iterator it = solutionBundleNames_.begin(); it != solutionBundleNames_.end(); it++){//Process solution bundle data
                std::unique_ptr<SolutionBundle<sbfReadWriteType, nuArrays> > solBundle(new SolutionBundle<sbfReadWriteType, nuArrays>(catalog_+(*it).base, mesh->numNodes()));
                solBundle->setStep(stepCount);
                solBundle->setNumDigits((*it).numDigits);
                int readRez = solBundle->readFromFile<sbfReadWriteType>();
                if(readRez == 0){
                    for(int ct = 0; ct < nuArrays; ct++){
                        int index;
                        NodesData<sbfReadWriteType, 1> * data = solBundle->array(ct);
                        std::cout << ((*it).base + solBundle->name(ct)).c_str() << std::endl;
                        std::string arrName = (*it).base + solBundle->name(ct);
                        vtkDataArray * darray = grid->GetPointData()->GetArray(arrName.c_str(), index);
                        if(index > -1) darray->Delete();
                        if(data){
                            vtkFloatArray * array = vtkFloatArray::New();
                            array->SetName(((*it).base + solBundle->name(ct)).c_str());
                            array->SetNumberOfComponents(1);
                            array->SetNumberOfTuples(mesh->numNodes());
                            flagSomeFileReadSuccess = true;
                            for(int ct = 0; ct < mesh->numNodes(); ct++) for(int ct1 = 0; ct1 < 1; ct1++) array->SetComponent(ct, ct1, data->data(ct, ct1));
                            grid->GetPointData()->AddArray(array);
                        }
                    }
                }
                else{
                    solBundle->setStep(stepCount-1);
                    if(solBundle->exist()){
                        for(int ct = 0; ct < nuArrays; ct++){
                            int index;
                            NodesData<sbfReadWriteType, 1> * data = solBundle->array(ct);
                            std::string arrName = (*it).base + solBundle->name(ct);
                            vtkDataArray * darray = grid->GetPointData()->GetArray(arrName.c_str(), index);
                            if(index > -1) darray->Delete();
                            if(data){
                                vtkFloatArray * array = vtkFloatArray::New();
                                array->SetName(((*it).base + solBundle->name(ct)).c_str());
                                array->SetNumberOfComponents(1);
                                array->SetNumberOfTuples(mesh->numNodes());
                                for(int ct = 0; ct < mesh->numNodes(); ct++) for(int ct1 = 0; ct1 < 1; ct1++) array->SetComponent(ct, ct1, std::numeric_limits<sbfReadWriteType>::quiet_NaN());
                                grid->GetPointData()->AddArray(array);
                            }
                        }
                    }
                }
            }//Process solution bundle data

            {//Process material files
                std::stringstream fileNameStream;
                fileNameStream.str("");
                fileNameStream << catalog_ << "/" << namePrefix_ + mtrBaseName_ << std::setw(mtrNumDigits_) << std::setfill('0') << stepCount << "." << sbaExtention_;
                std::ifstream mtrFile(fileNameStream.str().c_str(), std::ios_base::binary);
                if(mtrFile.good()){
                    std::vector<int> mtr;
                    mtr.resize(mesh->numElements());
                    mtrFile.read(reinterpret_cast<char *>(&mtr[0]), sizeof(int)*mesh->numElements());
                    int index;
                    vtkDataArray * darray = grid->GetCellData()->GetArray("materials", index);
                    if(index > -1) darray->Delete();
                    vtkIntArray * array = vtkIntArray::New();
                    array->SetName("materials");
                    array->SetNumberOfComponents(1);
                    array->SetNumberOfTuples(mesh->numElements());
                    flagSomeFileReadSuccess = true;
                    for(int ct = 0; ct < mesh->numElements(); ct++) array->SetComponent(ct, 0, mtr[ct]);
                    grid->GetCellData()->AddArray(array);
                }
            }//Process material files

            if(flagSomeFileReadSuccess){
                writer->WriteNextTime(stepCount-1);
                stepCount++;
            }
            else flagNextDataExisted = false;
        }
    }
    else
        writer->Write();
    writer->Stop();
    std::cout << "Done" << std::endl;

    return 0;
}

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
