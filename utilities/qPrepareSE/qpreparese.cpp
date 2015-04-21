#include "qpreparese.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>
#include <QVBoxLayout>
#include <QStyle>

QPrepareSE::QPrepareSE(QWidget *parent) :
    QDialog(parent),
    seBuilder_(nullptr)
{
    settings_ = new QSettings("RNCKI", "qPrepareSE", this);
    QVBoxLayout *lo = new QVBoxLayout;
    setLayout(lo);

    QHBoxLayout *catalogLO = new QHBoxLayout;
    catalogName_.setText(catalogD_.absolutePath());
    catalogLO->addWidget(&catalogName_);
    catalogB_.setText(tr("CHOUSE_IND_FILE"));
    inverseChB_.setToolTip(tr("CHOUSE_IND_FILE_TOOLTIP"));
    catalogLO->addWidget(&catalogB_);
    catalogLO->addWidget(&indFileName_);
    catalogLO->addStretch(10);
    lo->addLayout(catalogLO);

    seedChB_.setText(tr("USE_SEED"));
    seedChB_.setToolTip(tr("USE_SEED_TOOLTIP"));
    inverseChB_.setText(tr("USE_INVERSE"));
    inverseChB_.setToolTip(tr("USE_INVERSE_TOOLTIP"));
    partitionLine_.setToolTip(tr("PARTITION_TOOLTIP"));
    maxImbalanceLine_.setToolTip(tr("IMBALANCE_TOOLTIP"));
    levelBaseName_.setToolTip(tr("LEVELBASE_TOOLTIP"));
    nodeConnectionChb_.setText(tr("NODECONNECTION"));
    nodeConnectionChb_.setToolTip(tr("NODECONNECTION_TOOLTIP"));

    QHBoxLayout *settingsLO = new QHBoxLayout;
    settingsLO->addWidget(new QLabel(tr("PARTITION")));
    settingsLO->addWidget(&partitionLine_);
    settingsLO->addWidget(new QLabel(tr("IMBALANCE")));
    settingsLO->addWidget(&maxImbalanceLine_);
    settingsLO->addWidget(new QLabel(tr("LEVELBASE")));
    settingsLO->addWidget(&levelBaseName_);
    lo->addLayout(settingsLO);
    settingsLO = new QHBoxLayout;
    settingsLO->addWidget(&seedChB_);
    settingsLO->addWidget(&inverseChB_);
    settingsLO->addWidget(&nodeConnectionChb_);
    lo->addLayout(settingsLO);

    make_.setText(tr("MAKE"));
    lo->addWidget(&make_);
    make_.setEnabled(false);

    connect(&make_, &QPushButton::pressed, this, &QPrepareSE::onMake);

    connect(&catalogB_, &QPushButton::pressed, this, &QPrepareSE::onChangeDirPressed);
    auto lastFile = settings_->value("lastIndFile").toString();
    if(!lastFile.isEmpty()) onIndFileChanged(lastFile);
    partitionLine_.setText(settings_->value("lastPart", QString("64,16,4")).toString());
    maxImbalanceLine_.setText(settings_->value("lastImbalance", QString("1.3,1.1")).toString());
    levelBaseName_.setText(settings_->value("levelBaseName", QString("level")).toString());
}

void QPrepareSE::onChangeDirPressed()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("CHOUSE_IND_FILE"), QString(), QString("Indexes file (*.sba)"));
    if(fileName.isEmpty()) return;
    onIndFileChanged(fileName);
}

void QPrepareSE::onIndFileChanged(const QString &indFileName)
{
    QFileInfo fi(indFileName);
    if(fi.exists()) {
        catalogD_ = fi.absoluteDir();
        catalogName_.setText(fi.absolutePath());
        indFileName_.setText(fi.fileName());
        indFullFileName_ = fi.absoluteFilePath();
        settings_->setValue("lastDir", fi.absolutePath());
        settings_->setValue("lastIndFile", indFullFileName_);
        make_.setEnabled(true);
    }
    else
        make_.setEnabled(false);
}

void QPrepareSE::onMake()
{
    std::vector<int> numTargetByLayers;
    std::vector<double> maxImbalance;
    auto parts = partitionLine_.text().split(QRegExp("\\D"), QString::SkipEmptyParts);
    for(auto p : parts) numTargetByLayers.push_back(p.toInt());
    parts = maxImbalanceLine_.text().split(",", QString::SkipEmptyParts);
    for(auto p : parts) maxImbalance.push_back(p.toDouble());
    auto checkPart = [&]()->bool
    {
            for(size_t ct = 0; ct < numTargetByLayers.size()-1; ++ct)
            if(numTargetByLayers[ct] <= numTargetByLayers[ct+1])
            return true;
    return false;
    };
    if(numTargetByLayers.size() == 0 || checkPart()) {
        partitionLine_.setStyleSheet("border: 2px solid red");
        return;
    }
    if(maxImbalance.size() == 0) {
        maxImbalanceLine_.setStyleSheet("border: 2px solid red");
        return;
    }
    sbfMesh *mesh = new sbfMesh;
    mesh->readIndFromFile(indFullFileName_.toLocal8Bit());
    // IF THERE ARE SOME REASONS TO MAKE PARTITNONS ON VOLUME INSTEAD OF CUT... PLEASE READ MANs OF METIS :)
    seBuilder_ = new SEBuilder(mesh, true);
    seBuilder_->setUseSeed(seedChB_.checkState());
    seBuilder_->setUseInversePartition(inverseChB_.checkState());
    //    seBuilder_->setNumIterations(numIterations);
    //    seBuilder_->setNumCuts(numCut);
    seBuilder_->setUseConnectionByFaces(!nodeConnectionChb_.checkState());
    make_.setText(tr("PROCESSING"));
    try {
        seBuilder_->make(numTargetByLayers, maxImbalance);
    }
    catch(const std::exception &e){
        QMessageBox error;
        error.setIconPixmap(style()->standardPixmap(QStyle::SP_MessageBoxCritical));
        error.setText(tr("PART_FAILED_WITH_ERROR")+"\n"+QString::fromLocal8Bit(e.what()));
        error.setWindowTitle("Error!");
        error.exec();
        delete mesh;
        make_.setText(tr("MAKE"));
        return;
    }
    catch(...){
        QMessageBox error;
        error.setIconPixmap(style()->standardPixmap(QStyle::SP_MessageBoxCritical));
        error.setText(tr("PART_FAILED_WITH_UNKNOWN_ERROR"));
        error.setWindowTitle("Error!");
        error.exec();
        delete mesh;
        make_.setText(tr("MAKE"));
        return;
    }

    seBuilder_->write((catalogD_.absolutePath()+"/"+levelBaseName_.text()).toLocal8Bit());
    delete mesh;
    settings_->setValue("lastPart", partitionLine_.text());
    settings_->setValue("lastImbalance", maxImbalanceLine_.text());
    settings_->setValue("levelBaseName", levelBaseName_.text());
    make_.setText(tr("MAKE"));
}
