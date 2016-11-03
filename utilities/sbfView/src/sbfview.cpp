#include "sbfview.h"
#include "sbfmodel.h"
#include "vtkSmartPointer.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkProperty.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkThreshold.h"
#include "vtkTextProperty.h"
#include "vtkTransform.h"
#include "vtkMath.h"
#include "vtkPolyDataMapper.h"
#include "vtkGeometryFilter.h"

#include "vtkAxesActor.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkLight.h"
#include "vtkLightCollection.h"
#include "vtkLightKit.h"

#include <cmath>

#include <QDebug>
#include <QColor>

SbfView::SbfView(QWidget *parent) :
    QVTKWidget(parent)
{
    renderer_ = vtkRenderer::New();
    actor_ = vtkActor::New();
    bar_ = vtkScalarBarActor::New();
    cam_ = vtkCamera::New();
    cam_->SetParallelProjection(1);
    renderer_->SetActiveCamera(cam_);
    renderer_->ResetCamera();
    renderer_->SetBackground(1.0, 1.0, 1.0);

    vtkRenderWindow *renWin = GetRenderWindow();
    renWin->AddRenderer(renderer_);

    auto axesWidget = vtkOrientationMarkerWidget::New();
    axesWidget->SetDefaultRenderer( renderer_ );
    vtkRenderWindowInteractor* rwi = renderer_->GetRenderWindow()->GetInteractor();
    axesWidget->SetInteractor( rwi );

    vtkAxesActor* axes = vtkAxesActor::New();
    axesWidget->SetOrientationMarker ( axes );
    axes->Delete();

    axesWidget->SetEnabled(1);
    axesWidget->InteractiveOn();
}

void SbfView::setModel(SbfModel *model)
{
    renderer_->RemoveActor(actor_);
    renderer_->RemoveActor(bar_);
    model_ = model;
    mapper_ = vtkDataSetMapper::New();
    warp_ = vtkWarpVector::New();
    lt_ = vtkLookupTable::New();
    lt_->Build();
    matLt_ = vtkLookupTable::New();
    matLt_->Build();
    warp_->SetInputData(model_->grid());
    warp_->SetScaleFactor(1);
    mapper_->SetInputData(model_->grid());
    mapper_->SetLookupTable(lt_);
    auto range = model_->grid()->GetScalarRange();
    qDebug() << QString("Cur range is") << range[0] << range[1];
    mapper_->ImmediateModeRenderingOn();
    mapper_->SetScalarRange(range);
    bar_->SetLookupTable(mapper_->GetLookupTable());
    bar_->SetTitle("Materials");
    bar_->GetLabelTextProperty()->SetColor(0, 0, 0);
    bar_->GetTitleTextProperty()->SetColor(0, 0, 0);
    bar_->SetHeight(0.5);
    bar_->SetWidth(0.1);
    actor_ = vtkActor::New();
    actor_->SetMapper(mapper_);
    auto bounds = actor_->GetBounds();
    auto pos = actor_->GetPosition();
    actor_->SetPosition(pos[0]-(bounds[1] + bounds[0])/2, pos[1]-(bounds[3] + bounds[2])/2, pos[2]-(bounds[5] + bounds[4])/2);
    actor_->SetOrigin(0, 0, 0);
    renderer_->AddActor(actor_);
    renderer_->AddActor(bar_);
    setViewXYZ();
    setArrayToMap(QString::fromStdString("materials"), -1);

    update();

//    qDebug() << renderer_->GetLights()->GetNumberOfItems();

//    vtkLight *light = vtkLight::New();
//    light->SetLightTypeToSceneLight();
//    light->SetPosition(10, 10, 10);
////    light->SetPositional(true); // required for vtkLightActor below
//    light->SetConeAngle(10);
//    light->SetFocalPoint(0, 0, 0);
//    light->SetDiffuseColor(1,1,1);
//    light->SetAmbientColor(1,1,1);
//    light->SetSpecularColor(1,1,1);

//    renderer_->AddLight(light);
    vtkLightKit *lightKit = vtkLightKit::New();
    lightKit->AddLightsToRenderer(renderer_);
    lightKit->SetKeyLightIntensity(0.8);
}

void SbfView::setFixedRange(const QString &name, int component, double low, double height)
{
    fixedRanges_.insert(std::make_pair(name.toStdString(), component),
                        std::make_pair(low, height));
}

void SbfView::resetView()
{
    renderer_->ResetCamera();
}

void SbfView::setView(const std::array<double, 3> &position, const std::array<double, 3> &up)
{
    cam_->SetPosition(position.data());
    cam_->SetViewUp(up.data());
    renderer_->ResetCamera();
    update();
}

void SbfView::setViewXY()
{
    setView({0, 0, 1}, {0, 1, 0});
}

void SbfView::setViewYZ()
{
    setView({1, 0, 0}, {0, 0, 1});
}

void SbfView::setViewZX()
{
    setView({0, 1, 0}, {0, 0, 1});
}

void SbfView::setViewXYZ()
{
    setView({1, 1, 1}, {-std::sqrt(3), -std::sqrt(3), 2*std::sqrt(3)});
}

void SbfView::rotateView(const std::array<double, 3> &axis, double angle)
{
    std::array<double, 3> pos, up;
    cam_->GetPosition(pos.data());
    cam_->GetViewUp(up.data());
    vtkSmartPointer<vtkTransform> trans(vtkTransform::New());
    trans->RotateWXYZ(angle, axis.data());
    trans->TransformVector(pos.data(), pos.data());
    trans->TransformVector(up.data(), up.data());
    cam_->SetPosition(pos.data());
    cam_->SetViewUp(up.data());
    renderer_->ResetCamera();
    update();
}

void SbfView::rotateViewScreenX(double angle)
{
    std::array<double, 3> pos, up, rot;
    cam_->GetPosition(pos.data());
    cam_->GetViewUp(up.data());
    vtkMath::Cross(pos.data(), up.data(), rot.data());
    vtkSmartPointer<vtkTransform> trans(vtkTransform::New());
    trans->RotateWXYZ(angle, rot.data());
    trans->TransformVector(pos.data(), pos.data());
    trans->TransformVector(up.data(), up.data());
    cam_->SetPosition(pos.data());
    cam_->SetViewUp(up.data());
    renderer_->ResetCamera();
    update();
}

void SbfView::rotateViewScreenY(double angle)
{
    std::array<double, 3> pos, up;
    cam_->GetPosition(pos.data());
    cam_->GetViewUp(up.data());
    vtkSmartPointer<vtkTransform> trans(vtkTransform::New());
    trans->RotateWXYZ(angle, up.data());
    trans->TransformVector(pos.data(), pos.data());
    cam_->SetPosition(pos.data());
    renderer_->ResetCamera();
    update();
}

bool SbfView::edgeVisible()
{
    return actor_->GetProperty()->GetEdgeVisibility();
}

void SbfView::setEdgeVisible(bool on)
{
    actor_->GetProperty()->SetEdgeVisibility(on);
    update();
}

double SbfView::warpFactor() const
{
    return warp_->GetScaleFactor();
}

void SbfView::setWarpFactor(double factor)
{
    warpFactor_ = factor;
    warp_->SetScaleFactor(factor);
    warp_->Update();
}

void SbfView::setArrayToMap(QString name, int component)
{
    int arrayID = -1;
    auto cellArray = model_->grid()->GetCellData()->GetArray(name.toStdString().c_str(), arrayID);
    bar_->SetDrawTickLabels(true);
    if(cellArray) {
        model_->grid()->GetCellData()->SetScalars(cellArray);
        mapper_->SetInputData(model_->grid());
        mapper_->SetScalarModeToUseCellData();
        mapper_->SelectColorArray(arrayID);
        auto range = cellArray->GetRange(component);
        auto rangeIt = fixedRanges_.find(std::make_pair(name.toStdString(), component));
        if( rangeIt != fixedRanges_.end() ) {
            range[0] = rangeIt->first;
            range[1] = rangeIt->second;
        }
        if(cellArray->GetDataType() == VTK_INT) {
            fillMtrLt(cellArray);
            bar_->SetDrawTickLabels(false);
            mapper_->SetLookupTable(matLt_);
        }
        mapper_->SetScalarRange(range);
        bar_->SetTitle(name.toStdString().c_str());
        bar_->SetLookupTable(mapper_->GetLookupTable());
        actor_->SetMapper(mapper_);
        update();
        return;
    }
    auto nodeArray = model_->grid()->GetPointData()->GetArray(name.toStdString().c_str(), arrayID);
    if(nodeArray) {
        mapper_->SetScalarMode(VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
        auto range = nodeArray->GetRange(component);
        auto rangeIt = fixedRanges_.find(std::make_pair(name.toStdString(), component));
        if( rangeIt != fixedRanges_.end() ) {
            range[0] = rangeIt->first;
            range[1] = rangeIt->second;
        }
        mapper_->SetScalarRange(range);
        lt_->SetTableRange(range);
        size_t numColors = 8;
        lt_->SetNumberOfColors(numColors);
        for(size_t ct = 0; ct < numColors/2; ++ct){
            auto r = 0.0;
            auto g = ct/(numColors/2.0);
            auto b = (numColors/2-ct)/(numColors/2.0);
            lt_->SetTableValue(ct, r, g, b);
        }
        for(size_t ct = 0; ct < numColors/2; ++ct){
            auto r = ct/(numColors/2.0);
            auto g = (numColors/2-ct)/(numColors/2.0);
            auto b = 0.0;
            lt_->SetTableValue(ct+numColors/2, r, g, b);
        }
        lt_->Build();
        mapper_->SetLookupTable(lt_/*nodeArray->GetLookupTable()*/);
        mapper_->SelectColorArray(arrayID);
        if(component == -1)
            mapper_->GetLookupTable()->SetVectorMode(vtkScalarsToColors::MAGNITUDE);
        else {
            mapper_->GetLookupTable()->SetVectorMode(vtkScalarsToColors::COMPONENT);
            mapper_->GetLookupTable()->SetVectorComponent(component);
        }
        bar_->SetTitle(name.toStdString().c_str());
        bar_->SetLookupTable(lt_);
        actor_->SetMapper(mapper_);
        if(nodeArray->GetNumberOfComponents() == 3) {
            model_->grid()->GetPointData()->SetActiveVectors(name.toStdString().c_str());
            warp_->SetInputData(model_->grid());
            warp_->SetScaleFactor(warpFactor_);
            warp_->Update();
            mapper_->SetInputConnection(warp_->GetOutputPort());
        }
        else
            mapper_->SetInputData(model_->grid());
        update();
        return;
    }
}

void SbfView::fillMtrLt(vtkDataArray *array)
{
    matLt_ = vtkLookupTable::New();
    auto range = array->GetRange(0);
    matLt_->SetNumberOfTableValues(range[1] - range[0] + 1);
    for(int ct = 0/*range[0]*/; ct <= range[1]; ++ct ){
        QColor c = QColor::fromHsv(static_cast<int>(60+57.6*ct + 1.173*ct*ct + 0.027*ct*ct*ct)%360, 255/*230+(25+ct*7)%25*/, 255/*150+(104+ct*19)%105*/);
        matLt_->SetTableValue(ct, c.redF(), c.greenF(), c.blueF());
//        matLt_->SetTableValue(ct, (rand()%256)/256.0, (rand()%256)/256.0, (rand()%256)/256.0);
        matLt_->SetAnnotation(ct, vtkStdString(std::to_string(ct)));
    }
    matLt_->Build();
}
