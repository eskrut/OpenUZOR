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

#include "vtkDepthSortPolyData.h"

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

    renderer_->SetUseDepthPeeling(true);

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

//    mapper_->SetInputConnection(model_->grid()/*warp_->GetOutputPort()*/);
    mapper_->SetInputData(model_->grid());

    mapper_->SetLookupTable(lt_);
    auto range = model_->grid()->GetScalarRange();
    mapper_->ImmediateModeRenderingOn();
    mapper_->SetScalarRange(range);
    bar_->SetLookupTable(mapper_->GetLookupTable());
    bar_->SetTitle("Materials");
    bar_->GetLabelTextProperty()->SetColor(0, 0, 0);
    bar_->GetTitleTextProperty()->SetColor(0, 0, 0);
    bar_->GetTitleTextProperty()->SetFontSize(5);
    bar_->GetLabelTextProperty()->SetFontSize(5);
    bar_->SetHeight(0.5);
    bar_->SetWidth(0.1);
    actor_ = vtkActor::New();
    actor_->SetMapper(mapper_);
    auto bounds = actor_->GetBounds();
    auto pos = actor_->GetPosition();
    actor_->SetPosition(pos[0]-(bounds[1] + bounds[0])/2, pos[1]-(bounds[3] + bounds[2])/2, pos[2]-(bounds[5] + bounds[4])/2);
    actor_->SetOrigin(0, 0, 0);
    actor_->GetProperty()->SetRepresentationToSurface();
    renderer_->AddActor(actor_);
    renderer_->AddActor(bar_);

    //    setViewXYZ();
    setViewAngleHeight(180, 10);

    setArrayToMap(QString::fromStdString("materials"), -1);

    update();

    vtkLightKit *lightKit = vtkLightKit::New();
    lightKit->AddLightsToRenderer(renderer_);
    lightKit->SetKeyLightIntensity(1.0);
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
    cam_->UpdateViewport(renderer_);
    renderer_->ResetCamera();
    update();
}

void SbfView::setViewAngleHeight(float angle, float height) {
    float x = std::sin(angle / 180.0 * std::atan(1)*4);
    float y = std::cos(angle / 180.0 * std::atan(1)*4);
    float z = height;
    setView({x, y, z}, {x, y, 2*z});
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
    setView({2, 2, 2}, {-std::sqrt(3), -std::sqrt(3), 2*std::sqrt(3)});
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

void SbfView::setEdgeWidth(int w)
{
    actor_->GetProperty()->SetLineWidth(w);
}

void SbfView::setOpacity(float opacity)
{
    actor_->GetProperty()->SetOpacity(opacity);
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

//TODO allowe warp and mapping by different fields
void SbfView::setArrayToMap(QString name, int component)
{
    int arrayID = -1;
    arrayInMap_.clear();
    auto cellArray = model_->grid()->GetCellData()->GetArray(name.toStdString().c_str(), arrayID);
    auto nodeArray = model_->grid()->GetPointData()->GetArray(name.toStdString().c_str(), arrayID);
    if( cellArray || nodeArray )
        arrayInMap_ = name;
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
    else if(nodeArray) {
        mapper_->SetScalarMode(VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
        auto range = nodeArray->GetRange(component);
        auto rangeIt = fixedRanges_.find(std::make_pair(name.toStdString(), component));
        if( rangeIt != fixedRanges_.end() ) {
            range[0] = rangeIt->first;
            range[1] = rangeIt->second;
        }
        mapper_->SetScalarRange(range);
        lt_->SetTableRange(range);
        size_t numColors = 16;
        auto getLinear = [](float x0, float x1, float y0, float y1, float xcur){
            return y0 + (y1 - y0)/(x1 - x0)*(xcur - x0);
        };
        lt_->SetNumberOfColors(numColors);
        float r = 0;
        float g = 0;
        float b = 1;
        for(size_t ct = 0; ct < numColors; ++ct){
            float fraction = static_cast<float>(ct)/numColors;
            if( fraction <= 0.25) {
                r = 0; b = 1;
                g = getLinear(0, 0.25, 0, 1, fraction);
            }
            else if( fraction <= 0.5) {
                r = 0; g = 1;
                b = getLinear(0.25, 0.5, 1, 0, fraction);
            }
            else if( fraction <= 0.75) {
                b = 0; g = 1;
                r = getLinear(0.5, 0.75, 0, 1, fraction);
            }
            else if( fraction <= 1.0) {
                b = 0; r = 1;
                g = getLinear(0.75, 1.0, 1, 0, fraction);
            }
            else throw std::logic_error("Should not be possible");
            lt_->SetTableValue(ct, r, g, b);
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
    for(int ct = 0; ct < range[1] - range[0] + 1; ++ct ){
        QColor c = QColor::fromHsv(static_cast<int>(60+57.6*ct + 1.173*ct*ct + 0.027*ct*ct*ct)%360, 255/*230+(25+ct*7)%25*/, 255/*150+(104+ct*19)%105*/);
        matLt_->SetTableValue(ct, c.redF(), c.greenF(), c.blueF());
//        matLt_->SetTableValue(ct, (rand()%256)/256.0, (rand()%256)/256.0, (rand()%256)/256.0);
        matLt_->SetAnnotation(ct, vtkStdString(std::to_string(ct)));
    }
    matLt_->Build();
}
