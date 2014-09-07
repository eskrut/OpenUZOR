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
            view_->update();
            settings_->beginGroup("lastRead");
            settings_->setValue("catalog", QFileInfo(indName).absolutePath());
            settings_->endGroup();
        }
    }));
}
