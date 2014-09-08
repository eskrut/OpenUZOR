#include "sbfview.h"
#include "sbfmodel.h"
#include "vtkSmartPointer.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkDataSetMapper.h"
#include "vtkProperty.h"
#include "vtkCellData.h"
#include "vtkThreshold.h"
#include "vtkTextProperty.h"

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

    GetRenderWindow()->AddRenderer(renderer_);
}

void SbfView::setModel(SbfModel *model)
{
    renderer_->RemoveActor(actor_);
    renderer_->RemoveActor(bar_);
    model_ = model;
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkDataSetMapper::New();
    mapper->SetInputData(model_->grid());
    auto range = model_->grid()->GetScalarRange();
    qDebug() << QString("Cur range is") << range[0] << range[1];
    mapper->ImmediateModeRenderingOn();
    mapper->SetScalarRange(range);
    bar_->SetLookupTable(mapper->GetLookupTable());
    bar_->SetTitle("Materials");
    bar_->GetLabelTextProperty()->SetColor(0, 0, 0);
    bar_->GetTitleTextProperty()->SetColor(0, 0, 0);
    bar_->SetHeight(0.5);
    bar_->SetWidth(0.1);
    actor_ = vtkActor::New();
    actor_->SetMapper(mapper);
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
