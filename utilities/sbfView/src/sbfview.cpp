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

#include "vtkAxesActor.h"
#include "vtkOrientationMarkerWidget.h"

#include <cmath>

#include <QDebug>

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
    lt_ = vtkLookupTable::New();
    lt_->Build();
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
    bounds = actor_->GetBounds();
    actor_->SetOrigin(0, 0, 0);
    renderer_->AddActor(actor_);
    renderer_->AddActor(bar_);
    setViewXYZ();
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

bool SbfView::edgeVisible()
{
    return actor_->GetProperty()->GetEdgeVisibility();
}

void SbfView::setEdgeVisible(bool on)
{
    actor_->GetProperty()->SetEdgeVisibility(on);
    update();
}

void SbfView::setArrayToMap(QString name, int component)
{
    int arrayID = -1;
    auto cellArray = model_->grid()->GetCellData()->GetArray(name.toStdString().c_str(), arrayID);
    if(cellArray) {
//        model_->grid()->GetCellData()->SetScalars(cellArray);
        mapper_->SetScalarModeToUseCellData();
        auto range = cellArray->GetRange(component);
        qDebug() << QString("Cur cell range is") << range[0] << range[1];
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
        qDebug() << QString("Cur node range is") << range[0] << range[1];
        mapper_->SetScalarRange(range);
        lt_->SetTableRange(range);
        lt_->Build();
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
        update();
        return;
    }
}
