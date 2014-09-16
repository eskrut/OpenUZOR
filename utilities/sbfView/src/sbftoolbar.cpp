#include "sbftoolbar.h"

SbfToolBar::SbfToolBar(SbfDataModel *dataModel, QWidget *parent) :
    QToolBar(parent)
{
    dataBox_ = new QComboBox(this);
    dataBox_->setModel(dataModel);
    addWidget(dataBox_);

    Q_ASSERT(connect(dataBox_, &QComboBox::currentTextChanged, this, &SbfToolBar::curentArrayChanged));
}
