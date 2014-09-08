#include "sbfviewmainwindow.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QShortcut>

SbfViewMainWindow::SbfViewMainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    settings_ = new QSettings("NICKR", "SbfView", this);
    model_ = new SbfModel(this, settings_);
    view_ = new SbfView(this);
    view_->setModel(model_);

    setCentralWidget(view_);

    settings_->beginGroup("MainWindow");
    resize(settings_->value("size", QSize(400, 400)).toSize());
    move(settings_->value("pos", QPoint(200, 200)).toPoint());
    settings_->endGroup();

    initializeShortCuts();
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
    Q_ASSERT(connect(openSC, &QShortcut::activated, [=](){
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
    }));
    auto setViewX = new QShortcut(QKeySequence("Alt+X"), this);
    auto setViewY = new QShortcut(QKeySequence("Alt+Y"), this);
    auto setViewZ = new QShortcut(QKeySequence("Alt+Z"), this);
    auto setViewI = new QShortcut(QKeySequence("Alt+C"), this);
    Q_ASSERT(connect(setViewX, &QShortcut::activated, view_, &SbfView::setViewYZ));
    Q_ASSERT(connect(setViewY, &QShortcut::activated, view_, &SbfView::setViewZX));
    Q_ASSERT(connect(setViewZ, &QShortcut::activated, view_, &SbfView::setViewXY));
    Q_ASSERT(connect(setViewI, &QShortcut::activated, view_, &SbfView::setViewXYZ));

    auto toggleGrid = new QShortcut(QKeySequence("Ctrl+G"), this);
    Q_ASSERT(connect(toggleGrid, &QShortcut::activated, [=](){
        view_->setEdgeVisible(!view_->edgeVisible());
    }));
}
