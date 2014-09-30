#include "sbftoolbar.h"

SbfToolBar::SbfToolBar(SbfDataModel *dataModel, QWidget *parent) :
    QToolBar(parent)
{
    dataBox_ = new QComboBox(this);
    dataBox_->setModel(dataModel);
    addWidget(dataBox_);

    componentBox_ = new QComboBox(this);
    componentBox_->addItems(QStringList() << tr("Magnutude") << tr("X") << tr("Y") << tr("Z"));
    addWidget(componentBox_);

    Q_ASSERT(connect(dataBox_, &QComboBox::currentTextChanged, this, &SbfToolBar::curentArrayChanged));
    Q_ASSERT(connect(componentBox_, static_cast<void (QComboBox::*)(const int)>(&QComboBox::currentIndexChanged), this, &SbfToolBar::curentComponentChanged));
}
