#include "sbfviewmainwindow.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QShortcut>
#include <QToolBar>
#include <QComboBox>

//TEST
#include "sbfMesh.h"
#include "sbfNode.h"
#include "sbfElement.h"

SbfViewMainWindow::SbfViewMainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    settings_ = new QSettings("NRCKI", "SbfView", this);
    model_ = new SbfModel(this, settings_);
    view_ = new SbfView(this);
    view_->setModel(model_);

    toolBar_ = new SbfToolBar(model_->dataModel(), this);
    addToolBar(toolBar_);

    setCentralWidget(view_);

    settings_->beginGroup("MainWindow");
    resize(settings_->value("size", QSize(400, 400)).toSize());
    move(settings_->value("pos", QPoint(200, 200)).toPoint());
    settings_->endGroup();

    initializeShortCuts();
    initializeConnections();
}

SbfViewMainWindow::~SbfViewMainWindow()
{
    settings_->beginGroup("MainWindow");
    settings_->setValue("size", size());
    settings_->setValue("pos", pos());
    settings_->endGroup();
    settings_->sync();
}

void SbfViewMainWindow::initializeShortCuts()
{
    //Open model files
    auto openSC = new QShortcut(QKeySequence::Open, this);
    connect(openSC, &QShortcut::activated, [=](){
        settings_->beginGroup("lastRead");
        auto lastDir = settings_->value("catalog").toString();
        settings_->endGroup();
        QString indName, crdName, mtrName;
        indName = QFileDialog::getOpenFileName(this, tr("OPEN_FILE_IND"), lastDir, "*.sba");
        if(!indName.length()) return;
        crdName = QFileDialog::getOpenFileName(this, tr("OPEN_FILE_CRD"), QFileInfo(indName).absolutePath(), "*.sba");
        if(!crdName.length()) return;
        mtrName = QFileDialog::getOpenFileName(this, tr("OPEN_FILE_MTR"), QFileInfo(crdName).absolutePath(), "*.sba");
        if ( !model_->readModel(indName, crdName, mtrName) ) {
            view_->setModel(model_);
            settings_->beginGroup("lastRead");
            settings_->setValue("catalog", QFileInfo(indName).absolutePath());
            settings_->endGroup();
        }
    });
    auto addDataSC = new QShortcut(QKeySequence("Ctrl+D"), this);
    connect(addDataSC, &QShortcut::activated, [=](){
        settings_->beginGroup("lastRead");
        auto lastDir = settings_->value("catalog").toString();
        settings_->endGroup();
        QStringList dataFileNames = QFileDialog::getOpenFileNames(this, tr("OPEN_FILE_DATA"), lastDir, "*.sba");
        for(auto name : dataFileNames)
            model_->addData(name);
        report(model_->mesh()->numDVData(), model_->mesh()->numFVData(), model_->mesh()->numSolDData(), model_->mesh()->numSolFData());
    });
    auto setViewX = new QShortcut(QKeySequence("Alt+X"), this);
    auto setViewY = new QShortcut(QKeySequence("Alt+Y"), this);
    auto setViewZ = new QShortcut(QKeySequence("Alt+Z"), this);
    auto setViewI = new QShortcut(QKeySequence("Alt+C"), this);
    connect(setViewX, &QShortcut::activated, view_, &SbfView::setViewYZ);
    connect(setViewY, &QShortcut::activated, view_, &SbfView::setViewZX);
    connect(setViewZ, &QShortcut::activated, view_, &SbfView::setViewXY);
    connect(setViewI, &QShortcut::activated, view_, &SbfView::setViewXYZ);

    auto toggleGrid = new QShortcut(QKeySequence("Ctrl+G"), this);
    connect(toggleGrid, &QShortcut::activated, [=](){
        view_->setEdgeVisible(!view_->edgeVisible());
    });

    auto rotObjectX = new QShortcut(QKeySequence("Down"), this);
    connect(rotObjectX, &QShortcut::activated, [=](){
        view_->rotateViewScreenX(settings_->value("defaultRot", 10).toDouble());
    });
    auto rotObjectXm = new QShortcut(QKeySequence("Up"), this);
    connect(rotObjectXm, &QShortcut::activated, [=](){
        view_->rotateViewScreenX(-settings_->value("defaultRot", 10).toDouble());
    });
    auto rotObjectY = new QShortcut(QKeySequence("Right"), this);
    connect(rotObjectY, &QShortcut::activated, [=](){
        view_->rotateViewScreenY(settings_->value("defaultRot", 10).toDouble());
    });
    auto rotObjectYm = new QShortcut(QKeySequence("Left"), this);
    connect(rotObjectYm, &QShortcut::activated, [=](){
        view_->rotateViewScreenY(-settings_->value("defaultRot", 10).toDouble());
    });

    auto toggleTest = new QShortcut(QKeySequence("Ctrl+T"), this);
    connect(toggleTest, &QShortcut::activated, [=](){
        //Make simple test mesh
        std::unique_ptr<sbfMesh> m(sbfMesh::makeBlock(1, 2, 3, 1, 2, 3));
        NodesData<float> data("test", m.get());
        for(int ct = 0; ct < m->numNodes(); ++ct) {
            data.data(ct, 0) = m->node(ct).z()*m->node(ct).z();
            data.data(ct, 1) = m->node(ct).y()*0;
            data.data(ct, 2) = m->node(ct).z()*0;
        }
        for(int ct = 0; ct < m->numElements(); ++ct) {
            m->elem(ct).setMtr(ct+1);
        }
        m->writeMeshToFiles();
        data.writeToFile<float>();
        model_->readModel("ind.sba", "crd.sba", "mtr001.sba");
        model_->addData("test0001.sba");
    });
}

void SbfViewMainWindow::initializeConnections()
{
    connect(toolBar_, &SbfToolBar::curentArrayChanged, [=](const QString &name){
        view_->setArrayToMap(name, toolBar_->currentComponent());
    });
    connect(toolBar_, &SbfToolBar::curentComponentChanged, [=](const int component){
        view_->setArrayToMap(toolBar_->currentName(), component-1);
    });
    connect(toolBar_, &SbfToolBar::warpFactorChanged, view_, &SbfView::setWarpFactor);
}
