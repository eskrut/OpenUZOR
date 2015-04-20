#include "sbfToVTK.h"
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <list>

namespace po = boost::program_options;

int main (int argc, char ** argv)
{
    std::string catalog, indName, crdName, mtrBaseName, levelBase, prefix;
    std::string nodesDataNamesStr, solBundleNamesStr;
    int mtrNumDigits, nodeDataNumDigits, solBundleNumDigits;
    std::string sbaExtantion, vtkFileName;
    bool useCompression;
    po::options_description desc("Program options");
    desc.add_options()
    ("help,h", "print help message")
    ("work-dir,w", po::value<std::string>(&catalog)->default_value("./"), "work catalog")
    ("ind-file,i", po::value<std::string>(&indName)->default_value("ind.sba"), "ind file")
    ("crd-file,c", po::value<std::string>(&crdName)->default_value("crd.sba"), "crd file")
    ("mtr-file,m", po::value<std::string>(&mtrBaseName)->default_value("mtr"), "mtr file base name")
    ("prefix,p", po::value<std::string>(&prefix)->default_value(""), "prefix to add to standard names")
    ("mtr-digits", po::value<int>(&mtrNumDigits)->default_value(3), "mtr file digits number")
    ("level-base,l", po::value<std::string>(&levelBase)->default_value("level"), "level files base name")
    ("nodes-data,n", po::value<std::string>(&nodesDataNamesStr)->default_value(""), "base names for nodes data files separated with comma i.e. 'uuu,displ'")
    ("nodes-digits", po::value<int>(&nodeDataNumDigits)->default_value(4), "nodes data file digits number")
    ("sol-bundle,s", po::value<std::string>(&solBundleNamesStr)->default_value(""), "base names for solution bundle files separated with comma i.e. 's01,s02'")
    ("sol-digits", po::value<int>(&solBundleNumDigits)->default_value(4), "solution bundle file digits number")
    ("ext", po::value<std::string>(&sbaExtantion)->default_value("sba"), "extantion")
    ("output,o", po::value<std::string>(&vtkFileName)->default_value("vtk.vtu"), "output file name")
    ("compression", po::value<bool>(&useCompression)->default_value(true), "use compression")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    std::list<std::string> nodesDataNames, solBundleNames;
    std::stringstream sstr(nodesDataNamesStr);
    std::string entry;
    while(getline(sstr, entry, ','))
        nodesDataNames.push_back(entry);
    sstr.str(solBundleNamesStr);
    while(getline(sstr, entry, ','))
        solBundleNames.push_back(entry);

    if (vm.count("help") || vm.count("h")) { std::cout << desc << "\n"; return 1; }

    sbfToVTKWriter * writer = new sbfToVTKWriter ();

    writer->catalog() = catalog;
    writer->indName() = indName;
    writer->crdName() = crdName;
    writer->namePrefix() = prefix;
    writer->mtrBaseName() = mtrBaseName;
    writer->mtrNumDigits() = mtrNumDigits;
    writer->levelBaseName() = levelBase;
    writer->sbaExtention() = sbaExtantion;

    writer->nodesDataNames().clear();
    SbaNameParts nameParts;
    nameParts.ext = sbaExtantion;
    nameParts.numDigits = nodeDataNumDigits;
    for(auto entry : nodesDataNames) {
        nameParts.base = entry;
        writer->nodesDataNames().push_back(nameParts);
    }

    writer->solutionBundleNames().clear();
    nameParts.ext = sbaExtantion;
    nameParts.numDigits = solBundleNumDigits;
    for(auto entry : solBundleNames) {
        nameParts.base = entry;
        writer->solutionBundleNames().push_back(nameParts);
    }

    writer->vtkName() = vtkFileName;
    writer->setUseCompression(useCompression);

    writer->write();

    delete writer;
    return 0;
}
