#include "sbftovtkdialog.h"
#include "ui_sbftovtkdialog.h"

SbfToVtkDialog::SbfToVtkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SbfToVtkDialog)
{
    patternRE = QString("([A-Za-z_0-9]+[A-Za-z_1-9])(0[0-9]+)\\.([A-Za-z_]+$)");
    ui->setupUi(this);
    settings = new QSettings("RNCKI", "qSbfToVtk");
    QStringList knownDirs;
    knownDirs = settings->value("catalogs", QVariant(knownDirs)).toStringList();
    ui->catalogCB->addItems(knownDirs);
    if(ui->catalogCB->count()){
        catalog = ui->catalogCB->itemText(0) + "/";
        updateCatalogContents();
    }
    numLastCatalogs = 10;
    ui->sbaExtentionLE->setText(settings->value("sbaFormat", "sba").toString());
    ui->mtrNumDigitsSB->setValue(settings->value("numDigitsMtr", 3).toInt());
    ui->numDigitsNodeSB->setValue(settings->value("numDigitsNode", 4).toInt());
    ui->numDigitsBundleSB->setValue(settings->value("numDigitsBundle", 4).toInt());
    ui->mtrNameLE->setEnabled(false);
    connect(ui->mtrBaseNameLE, SIGNAL(textChanged(QString)), this, SLOT(updateMtrName()));
    connect(ui->mtrNumDigitsSB, SIGNAL(valueChanged(int)), this, SLOT(updateMtrName()));
    connect(ui->sbaExtentionLE, SIGNAL(textChanged(QString)), this, SLOT(updateMtrName()));
    connect(ui->catalogCB, SIGNAL(highlighted(QString)), this, SLOT(setCatalog(QString)));

    stdNamesNode << "uuu";
    stdNamesNode = settings->value("stdNamesNode", stdNamesNode).toStringList();
    ui->stdNodeLW->addItems(stdNamesNode);

    stdNamesBundle << "s01" << "s02" << "s03";
    stdNamesBundle = settings->value("stdNamesBundle", stdNamesBundle).toStringList();
    ui->stdBundleLW->addItems(stdNamesBundle);

    //updateCatalogContents();
    //updateMtrName();
    setCatalog(catalog);
}

SbfToVtkDialog::~SbfToVtkDialog()
{
    QStringList knownDirs;
    for(int ct = 0; ct < ui->catalogCB->count() && ct < numLastCatalogs; ct++){
        if(!knownDirs.contains(ui->catalogCB->itemText(ct)))
            knownDirs << ui->catalogCB->itemText(ct);
    }
    settings->setValue("catalogs", knownDirs);
    settings->setValue("stdNamesNode", stdNamesNode);
    settings->setValue("stdNamesBundle", stdNamesBundle);
    settings->setValue("sbaFormat", ui->sbaExtentionLE->text());
    settings->setValue("numDigitsMtr", ui->mtrNumDigitsSB->value());
    settings->setValue("numDigitsNode", ui->numDigitsNodeSB->value());
    settings->setValue("numDigitsBundle", ui->numDigitsBundleSB->value());
    settings->sync();
    delete settings;
    delete ui;
}

void SbfToVtkDialog::on_catalogB_clicked()
{
    catalog = QFileDialog::getExistingDirectory(this, "Choose model root directory");
    if(!catalog.isEmpty()){
        ui->catalogCB->insertItem(0, catalog);
        catalog += "/";
        ui->catalogCB->setCurrentIndex(0);
    }
    updateCatalogContents();
}

void SbfToVtkDialog::updateMtrName()
{
    std::stringstream sstr;
    sstr << ui->mtrBaseNameLE->text().toStdString();
    sstr << std::setw(ui->mtrNumDigitsSB->value()) << std::setfill('0') << 1 << "." << ui->sbaExtentionLE->text().toStdString();
    ui->mtrNameLE->setText(QString::fromStdString(sstr.str()));
}

void SbfToVtkDialog::updateCatalogContents()
{
    //Check standard names and populate input widgets
    QStringList stdNames;
    ui->indNameLE->setText("");
    stdNames << "ind" << "iopt" << "indopt" << "oind";
    stdNames = settings->value("stdIndNames", stdNames).toStringList();
    foreach(const QString &str, stdNames){
        QFileInfo fi;
        fi.setFile(catalog+str+"."+ui->sbaExtentionLE->text());
        if(fi.exists())
            ui->indNameLE->setText(str+"."+ui->sbaExtentionLE->text());
    }
    stdNames.clear();
    ui->crdNameLE->setText("");
    stdNames << "crd" << "copt" << "crdopt" << "ocrd";
    stdNames = settings->value("stdCrdNames", stdNames).toStringList();
    foreach(const QString &str, stdNames){
        QFileInfo fi;
        fi.setFile(catalog+str+"."+ui->sbaExtentionLE->text());
        if(fi.exists())
            ui->crdNameLE->setText(str+"."+ui->sbaExtentionLE->text());
    }
    stdNames.clear();
    ui->mtrNameLE->setText("");
    ui->mtrBaseNameLE->setText("");
    stdNames << "mtr" << "mopt" << "omtr";
    stdNames = settings->value("stdMtrNames", stdNames).toStringList();
    foreach(const QString &str, stdNames){
        QFileInfo fi;
        SbaNameParts name;
        name.base = str.toStdString();
        name.numDigits = ui->mtrNumDigitsSB->value();
        name.ext = ui->sbaExtentionLE->text().toStdString();
        fi.setFile(catalog+QString::fromStdString(name.construct()));
        if(fi.exists()){
            ui->mtrBaseNameLE->setText(QString::fromStdString(name.base));
            updateMtrName();
        }
    }

    QStringList stdNodeNames, existNodeNames;
    QStringList stdBundleNames, existBundleNames;
    stdNodeNames << "uuu";
    stdBundleNames << "s01" << "s02" << "s03" << "t01" << "t02" << "t03";
    stdNodeNames = settings->value("stdNodeNames", stdNodeNames).toStringList();
    stdBundleNames = settings->value("stdBundleNames", stdBundleNames).toStringList();

    ui->namesNodeLW->clear();
    foreach(const QString &str, stdNodeNames){
        QFileInfo fi;
        SbaNameParts name;
        name.base = str.toStdString();
        name.numDigits = ui->numDigitsNodeSB->value();
        name.ext = ui->sbaExtentionLE->text().toStdString();
        fi.setFile(catalog+QString::fromStdString(name.construct()));
        if(fi.exists()){
            ui->namesNodeLW->addItem(QString::fromStdString(name.base));
        }
    }
    ui->namesBundleLW->clear();
    foreach(const QString &str, stdBundleNames){
        QFileInfo fi;
        SbaNameParts name;
        name.base = str.toStdString();
        name.numDigits = ui->numDigitsBundleSB->value();
        name.ext = ui->sbaExtentionLE->text().toStdString();
        fi.setFile(catalog+QString::fromStdString(name.construct()));
        if(fi.exists()){
            ui->namesBundleLW->addItem(QString::fromStdString(name.base));
        }
    }
}

void SbfToVtkDialog::on_chooseInd_clicked()
{
    QString newIndName = QFileDialog::getOpenFileName(this, "Choose indexes file", catalog, "SBA files (*.sba)");
    if(!newIndName.isEmpty()){
        QFileInfo fInfo(newIndName);
        indFileName = fInfo.fileName();
    }
    ui->indNameLE->setText(indFileName);
}

void SbfToVtkDialog::on_chooseCrd_clicked()
{
    QString newCrdName = QFileDialog::getOpenFileName(this, "Choose coordinates file", catalog, "SBA files (*.sba)");
    if(!newCrdName.isEmpty()){
        QFileInfo fInfo(newCrdName);
        crdFileName = fInfo.fileName();
    }
    ui->crdNameLE->setText(crdFileName);
}

void SbfToVtkDialog::on_chooseMtr_clicked()
{
    QString newMtrName = QFileDialog::getOpenFileName(this, "Choose indexes file", catalog, "SBA files (*.sba)");
    if(!newMtrName.isEmpty()){
        QFileInfo fInfo(newMtrName);
        QRegExp re(patternRE);
        re.indexIn(fInfo.fileName());
        if(re.captureCount() != 3){
            QMessageBox box(QMessageBox::NoIcon, QString("Error"), QString("There is error in file name recognition. Please setup file parts manualy."));
            box.exec();
            return;
        }
        ui->mtrBaseNameLE->setText(re.cap(1));
        ui->mtrNumDigitsSB->setValue(re.cap(2).length());
        ui->sbaExtentionLE->setText(re.cap(3));
    }
    updateMtrName();
}

void SbfToVtkDialog::on_chooseAll_clicked()
{
    on_chooseInd_clicked();
    on_chooseCrd_clicked();
    on_chooseMtr_clicked();
}

void SbfToVtkDialog::on_chooseNodePB_clicked()
{
    QStringList nodeNames = QFileDialog::getOpenFileNames(this, "Choose nodes data files", catalog, "SBA files (*.sba)");
    for(int ct = 0; ct < nodeNames.length(); ct++){
        QFileInfo fi(nodeNames[ct]);
        QRegExp re(patternRE);
        re.indexIn(fi.fileName());
        nodeNames[ct] = re.cap(1);
        ui->numDigitsNodeSB->setValue(re.cap(2).length());
    }
    ui->namesNodeLW->addItems(nodeNames);
}

void SbfToVtkDialog::on_chooseBundlePB_clicked()
{
    QStringList bundleNames = QFileDialog::getOpenFileNames(this, "Choose data bundle files", catalog, "SBA files (*.sba)");
    for(int ct = 0; ct < bundleNames.length(); ct++){
        QFileInfo fi(bundleNames[ct]);
        QRegExp re(patternRE);
        re.indexIn(fi.fileName());
        bundleNames[ct] = re.cap(1);
        ui->numDigitsBundleSB->setValue(re.cap(2).length());
    }
    ui->namesBundleLW->addItems(bundleNames);
}

void SbfToVtkDialog::setCatalog(QString newCatalog)
{
    if (catalog == newCatalog || catalog == (newCatalog + "/")) return;
    catalog = newCatalog;
    if(!catalog.isEmpty())
        catalog += "/";
    updateCatalogContents();
}

void SbfToVtkDialog::on_convertB_clicked()
{
    indFileName = ui->indNameLE->text();
    crdFileName = ui->crdNameLE->text();
    mtrFileName = ui->mtrNameLE->text();
    vtkFileName = ui->vtkNameLE->text();
    if(!catalog.isEmpty()){
        //this->setEnabled(false);
        sbfToVTKWriter * writer = new sbfToVTKWriter ();

        writer->catalog() = catalog.toStdString();
        writer->indName() = indFileName.toStdString();
        writer->crdName() = crdFileName.toStdString();
        //writer->mtrName() = mtrFileName.toStdString();
        writer->mtrBaseName() = ui->mtrBaseNameLE->text().toStdString();
        writer->mtrNumDigits() = ui->mtrNumDigitsSB->value();
        writer->sbaExtention() = ui->sbaExtentionLE->text().toStdString();

        writer->nodesDataNames().clear();
        SbaNameParts nameParts;
        nameParts.ext = ui->sbaExtentionLE->text().toStdString();
        nameParts.numDigits = ui->numDigitsNodeSB->value();
        for(int ct = 0; ct < ui->namesNodeLW->count(); ct++) {
            nameParts.base = ui->namesNodeLW->item(ct)->text().toStdString();
            writer->nodesDataNames().push_back(nameParts);
        }

        writer->solutionBundleNames().clear();
        nameParts.ext = ui->sbaExtentionLE->text().toStdString();
        nameParts.numDigits = ui->numDigitsBundleSB->value();
        for(int ct = 0; ct < ui->namesBundleLW->count(); ct++) {
            nameParts.base = ui->namesBundleLW->item(ct)->text().toStdString();
            writer->solutionBundleNames().push_back(nameParts);
        }

        writer->vtkName() = vtkFileName.toStdString();
        writer->setUseCompression(ui->useCompressionChB->isChecked());

        writer->write();

        if(ui->closeOnDoneChB->isChecked())
            this->close();
        //this->setEnabled(false);
    }
}

void SbfToVtkDialog::on_cancelB_clicked()
{
    this->close();
}
