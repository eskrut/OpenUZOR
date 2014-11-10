#include "qpreparese.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QString>
#include <QVBoxLayout>

QPrepareSE::QPrepareSE(QWidget *parent) :
    QDialog(parent),
    seBuilder_(nullptr)
{
    QVBoxLayout *lo = new QVBoxLayout;
    setLayout(lo);

    QHBoxLayout *catalogLO = new QHBoxLayout;
    catalogName_.setText(catalogD_.absolutePath());
    catalogLO->addWidget(&catalogName_);
    catalogB_.setText(tr("CHOUSE_CATALOG"));
    catalogLO->addWidget(&catalogB_);
    catalogLO->addWidget(&indFileName_);
    lo->addLayout(catalogLO);

    seedChB_.setText(tr("USE_SEED"));
    inverseChB_.setText(tr("USE_INVERSE"));

    QHBoxLayout *settingsLO = new QHBoxLayout;
    settingsLO->addWidget(new QLabel(tr("PARTITION")));
    settingsLO->addWidget(&partitionLine_);
    settingsLO->addWidget(new QLabel(tr("IMBALANCE")));
    settingsLO->addWidget(&maxImbalanceLine_);
    settingsLO->addWidget(&seedChB_);
    settingsLO->addWidget(&inverseChB_);
    settingsLO->addWidget(&levelBaseName_);
    lo->addLayout(settingsLO);

    make_.setText(tr("MAKE"));
    lo->addWidget(&make_);
    make_.setEnabled(false);

    connect(&catalogB_, &QPushButton::pressed, this, &QPrepareSE::onChangeDirPressed);
}

void QPrepareSE::onChangeDirPressed()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("CHOUSE_IND_FILE"), QString(), QString("Indexes file (*.sba)"));
    if(fileName.isEmpty()) return;
    QFileInfo fi(fileName);
    catalogD_ = fi.absoluteDir();
    catalogName_.setText(fi.absolutePath());
    indFileName_.setText(fi.fileName());
    indFullFileName_ = fi.absoluteFilePath();
}

void QPrepareSE::onDirChanged(const QString &dirName)
{

}

void QPrepareSE::onMake()
{
    sbfMesh *mesh = new sbfMesh;
    mesh->readIndFromFile(indFullFileName_.toLocal8Bit());
    // IF THERE ARE SOME REASONS TO MAKE PARTITNONS ON VOLUME INSTEAD OF CUT... PLEASE READ MANs OF METIS :)
    seBuilder_ = new SEBuilder(mesh, true);
    delete mesh;
}
