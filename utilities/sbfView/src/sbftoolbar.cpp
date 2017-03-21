#include "sbftoolbar.h"
#include "sbfdataitem.h"

#include <QHBoxLayout>
#include <QAction>

SbfToolBar::SbfToolBar(SbfDataModel *dataModel, QWidget *parent) :
    QToolBar(parent),
    dataModel_(dataModel)
{
    dataBox_ = new QComboBox(this);
    proxy_ = new ProxyFilter(this);
    proxy_->setSourceModel(dataModel_);
    dataBox_->setModel(proxy_);
    addWidget(dataBox_);

    componentBox_ = new QComboBox(this);
    componentBox_->addItems(QStringList() << tr("Magnutude") << tr("X") << tr("Y") << tr("Z"));
    addWidget(componentBox_);

    warpWidget_ = new QWidget();
    warpOnButton_ = new QToolButton(warpWidget_);
    warpOnButton_->setText("Warp");
    warpOnButton_->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    warpOnButton_->setCheckable(true);
    warpScaleLE_ = new QLineEdit("1.0", warpWidget_);
    warpScaleLE_->setValidator(new QDoubleValidator());
    warpScaleLE_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    auto warpLO = new QHBoxLayout(warpWidget_);
    warpLO->setMargin(0);
    warpWidget_->setLayout(warpLO);
    warpLO->addWidget(warpOnButton_);
    warpLO->addWidget(warpScaleLE_);
    wAct_ = addWidget(warpWidget_);
    wAct_->setVisible(false);

    connect(dataBox_, &QComboBox::currentTextChanged, this, &SbfToolBar::curentArrayChanged);
    connect(dataBox_, static_cast<void (QComboBox::*)(const int)>(&QComboBox::currentIndexChanged), this, &SbfToolBar::onArrayChanged);
    connect(componentBox_, static_cast<void (QComboBox::*)(const int)>(&QComboBox::currentIndexChanged), this, &SbfToolBar::curentComponentChanged);
    connect(warpScaleLE_, &QLineEdit::textChanged, [=](const QString &text){
        warpFactorChanged(text.toDouble());
    });

    //Step controller
    stepWidget_ = new QWidget();
    stepID_ = new QSpinBox(stepWidget_);
    stepID_->setRange(0, 0);
    stepID_->setValue(0);
    stepFirst_ = new QToolButton(stepWidget_);
    stepFirst_->setText("First");
    stepFirst_->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    stepPrev_ = new QToolButton(stepWidget_);
    stepPrev_->setText("Prev");
    stepPrev_->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    stepNext_ = new QToolButton(stepWidget_);
    stepNext_->setText("Next");
    stepNext_->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    stepLast_ = new QToolButton(stepWidget_);
    stepLast_->setText("Last");
    stepLast_->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    auto stepLO = new QHBoxLayout(stepWidget_);
    stepLO->setMargin(0);
    stepWidget_->setLayout(stepLO);
    stepLO->addWidget(stepFirst_);
    stepLO->addWidget(stepPrev_);
    stepLO->addWidget(stepID_);
    stepLO->addWidget(stepNext_);
    stepLO->addWidget(stepLast_);
    stepAct_ = addWidget(stepWidget_);
}

QString SbfToolBar::currentName() const {
    return proxy_->mapToSource(proxy_->index(dataBox_->currentIndex(), 0)).data().toString();
}

void SbfToolBar::addAllowed(const QString &field)
{
    proxy_->addAllowed(field);
}

void SbfToolBar::setWarpFactor(double warp)
{
    warpScaleLE_->setText(QString::number(warp));
}

void SbfToolBar::onArrayChanged(int ID)
{
    ID = proxy_->mapToSource(proxy_->index(ID, 0)).row();
    SbfDataItem *item = reinterpret_cast<SbfDataItem*>(dataModel_->invisibleRootItem()->child(ID));
    if( item->data(SbfDataItem::Role::VtkTypeRequest).toInt() == static_cast<int>(SbfDataItem::VtkType::NodeData))
    {
        if (item->type() == SbfDataItem::Type::DoubleVector || item->type() == SbfDataItem::Type::FloatVector)
            wAct_->setVisible(true);
        else
            wAct_->setVisible(false);
    }
    else wAct_->setVisible(false);
}

void SbfToolBar::setDataSteps(int first, int cur, int last)
{

}
