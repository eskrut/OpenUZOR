#include "sbf.h"
#include <boost/program_options.hpp>
#include <vector>
#include "sebuilder.h"

namespace po = boost::program_options;

int main(int argc, char ** argv)
{
    //int numTargetSE;
    std::vector<int> numTargetByLayers;
    std::string numTargetByLayersStr;
    std::string catalog, indName, crdName, mtrName, levelBase;
    po::options_description desc("Program options");
    bool verbouse = false;
    std::vector<double> maxImbalance;
    std::string maxImbalanceStr;
    bool targetCut = true;
    int numIterations;
    bool useSeed = false;
    int numCut;
    desc.add_options()
                    ("help,h", "print help message")
                    ("num-se,n", po::value<std::string>(&numTargetByLayersStr)->default_value("16"), "target numbers of SE by layers i.e. '64,8,2'")
                    ("work-dir,w", po::value<std::string>(&catalog)->default_value(""), "work catalog")
                    ("ind-file,i", po::value<std::string>(&indName)->default_value("ind.sba"), "ind file")
                    ("crd-file,c", po::value<std::string>(&crdName)->default_value("crd.sba"), "crd file")
                    ("mtr-file,m", po::value<std::string>(&mtrName)->default_value("mtr001.sba"), "mtr file")
                    ("level-base,l", po::value<std::string>(&levelBase)->default_value("level"), "level files base name")
                    ("verbouse,v", "make verbouse output")
                    ("max-imbalance", po::value<std::string>(&maxImbalanceStr)->default_value("1.3"), "target maximum imbalance in SE layers i.e. '1.4,1.3,1.3'. Low values of this parameters can couse program to fail.")
                    ("vol", "Minimize number of elements diabalance instead of cut minimization")
                    ("max-iterations", po::value<int>(&numIterations)->default_value(1000), "Maximum iterations in graph partitioning algorythm")
                    ("seed", "Use random seed")
                    ("num-cuts", po::value<int>(&numCut)->default_value(3), "Number of cuts to try partitioning")
                    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help") || vm.count("h")) { std::cout << desc << "\n"; return 1; }

    if (vm.count("verbouse") || vm.count("v")) verbouse = true;

    if (vm.count("vol")) targetCut = false;

    if (vm.count("seed")) useSeed = true;

    //extract number of SE by layers from cmd string
    std::stringstream sstr(numTargetByLayersStr);
    std::string entry;
    while(getline(sstr, entry, ','))
        numTargetByLayers.push_back(std::stoi(entry));

    //extract max imbalance of SE by layers from cmd string
    {
        std::stringstream sstr(maxImbalanceStr);
        while(getline(sstr, entry, ','))
            maxImbalance.push_back(std::stod(entry));
    }

    if(catalog.size() && (catalog.back() != '/' || catalog.back() != '\\')) catalog += "/";

    std::stringstream iName, cName, mName, lName/*, oiName, ocName, omName*/;
    iName << catalog << indName;
    cName << catalog << crdName;
    mName << catalog << mtrName;
    lName << catalog << levelBase;
    //Creating mesh
    std::unique_ptr<sbfMesh> pMesh(new sbfMesh());
    if(pMesh->readMeshFromFiles(iName.str().c_str(), cName.str().c_str(), mName.str().c_str()))
    {std::cout << "error while mesh reading" << std::endl; return 1;}

    std::unique_ptr<SEBuilder> pBuilder(new SEBuilder(pMesh.get(), targetCut));

    pBuilder->setUseSeed(useSeed);
    pBuilder->setVerbouse(verbouse);
    pBuilder->setNumIterations(numIterations);
    pBuilder->setNumCuts(numCut);
    pBuilder->make(numTargetByLayers, maxImbalance);
    pBuilder->write(lName.str().c_str());

    std::cout << "DONE" << std::endl;
    return 0;
}
