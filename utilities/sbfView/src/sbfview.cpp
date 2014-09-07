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

SbfView::SbfView(QWidget *parent) :
    QVTKWidget(parent)
{
    renderer_ = vtkRenderer::New();

    vtkSmartPointer<vtkCamera> cam = vtkCamera::New();
    cam->SetParallelProjection(1);
    renderer_->SetActiveCamera(cam);
    renderer_->ResetCamera();
    renderer_->SetBackground(1.0, 1.0, 1.0);

    GetRenderWindow()->AddRenderer(renderer_);
}

void SbfView::setModel(SbfModel *model)
{
    model_ = model;
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkDataSetMapper::New();
    mapper->SetInputData(model_->grid());
    mapper->ImmediateModeRenderingOn();
    vtkSmartPointer<vtkActor> actor = vtkActor::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetEdgeVisibility(1);
    renderer_->AddActor(actor);
    renderer_->ResetCamera();
    renderer_->SetBackground(1.0, 1.0, 1.0);
}

void SbfView::resetView()
{
    renderer_->ResetCamera();
}
