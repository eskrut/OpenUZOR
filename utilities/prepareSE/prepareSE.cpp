#include "sbfMesh.h"
#include <boost/program_options.hpp>
#include <scotch/scotch.h>

namespace po = boost::program_options;

int main(int argc, char ** argv)
{
	using namespace std;
	using namespace sbf;
	int numTargetSE;
	string catalog, indName, crdName, levelBase;
	po::options_description desc("Program options");
	desc.add_options()
					("help,h", "print help message")
					("num-se,n", po::value<int>(&numTargetSE)->default_value(2), "target number of SE")
					("work-dir,w", po::value<string>(&catalog)->default_value(""), "work catalog")
					("ind-file,i", po::value<string>(&indName)->default_value("ind.sba"), "ind file")
					("crd-file,c", po::value<string>(&crdName)->default_value("crd.sba"), "crd file")
					("level-base,l", po::value<string>(&levelBase)->default_value("level"), "level files base name")
					;
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help") || vm.count("h")) { cout << desc << "\n"; return 1; }

	stringstream iName, cName, mName, lName, oiName, ocName, omName;
	iName << catalog << indName;
	cName << catalog << crdName;
	lName << catalog << levelBase;
	//Creating mesh
	std::auto_ptr<sbfMesh> pMesh(new sbfMesh());
	//if(pMesh->readMeshFromFiles(iName.str().c_str(), cName.str().c_str(), mName.str().c_str()))
	if(pMesh->readIndFromFile(iName.str().c_str()))
	{cout << "error while mesh reading" << endl; return 1;}
	pMesh->printInfo();
	//pMesh->optimizeNodesNumbering();

	int numElems = pMesh->numElements();

	vector <double> facesWeigth;
	vector <int> facesOwners;
	vector <double> randoms;
	randoms.resize(50);
	for(size_t ct = 0; ct < randoms.size(); ct++) randoms[ct] = ((double)rand())/RAND_MAX;

	facesWeigth.reserve(numElems*20);
	facesOwners.reserve(numElems*20);

	for(int elemID = 0; elemID < numElems; elemID++){//Loop on elements
		sbfElement *elem = pMesh->elemPtr(elemID);
		vector<int> indexes = elem->indexes();

		double faceWeigth;
		int facesOwner = elemID;
		list<int> inds;
		int count;

		vector< vector<int> > facesNodesIndexes = elem->facesNodesIndexes();
		for(vector< vector<int> >::iterator itFace = facesNodesIndexes.begin(); itFace != facesNodesIndexes.end(); itFace++){//Loop on faces
			inds.clear();
			for(vector<int>::iterator itIndex = (*itFace).begin(); itIndex != (*itFace).end(); itIndex++)
				inds.push_back(*itIndex);
			inds.sort();
			count = 0; faceWeigth = 0; for(list<int>::iterator it = inds.begin(); it != inds.end(); it++) {faceWeigth += *it*randoms[count++];}
			facesWeigth.push_back(faceWeigth);
			facesOwners.push_back(facesOwner);
		}//Loop on faces

	}//Loop on elements

	quickAssociatedSortUp<double, int>(&facesWeigth[0], &facesOwners[0], 0, facesWeigth.size()-1);

	cout << "sort done" << endl;

	vector< list <int> > elemNeibour;
	elemNeibour.resize(numElems);

	int vertnbr, edgenbr;
	vertnbr = numElems;
	edgenbr = 0;

	int founded = 0, unfounded = 0;

	vector <double>::iterator facesWeigthIt = facesWeigth.begin();
	vector <int>::iterator facesOwnersIt = facesOwners.begin();
	vector <double>::iterator facesWeigthEndM1 = facesWeigth.end() - 1;
	for(; facesWeigthIt < facesWeigthEndM1; facesWeigthIt++, facesOwnersIt++){
		if(*facesWeigthIt == *(facesWeigthIt+1)){
			elemNeibour[*facesOwnersIt].push_back(*(facesOwnersIt+1));
			elemNeibour[*(facesOwnersIt+1)].push_back(*facesOwnersIt);
			facesWeigthIt++; facesOwnersIt++; edgenbr += 2;
			founded++;
		}
		else unfounded++;
	}

	vector <int> verttab, edgetab, parttab;
	verttab.resize(vertnbr+1);
	edgetab.resize(edgenbr);
	parttab.resize(numElems);
	int count = 0;
	verttab[0] = count;
	for(size_t ct = 0; ct < elemNeibour.size(); ct++){
		for(list <int>::iterator it = elemNeibour[ct].begin(); it != elemNeibour[ct].end(); it++) edgetab[count++] = *it;
		verttab[ct+1] = count;
	}

	SCOTCH_Strat stradat;
	SCOTCH_stratInit(&stradat);
	SCOTCH_Graph grafdat;
	SCOTCH_graphInit(&grafdat);
	SCOTCH_graphBuild(&grafdat, 0, vertnbr, &verttab[0], &verttab[1], NULL, NULL, edgenbr, &edgetab[0], NULL);
	SCOTCH_graphCheck(&grafdat);
	SCOTCH_Arch archdat;
	SCOTCH_archInit(&archdat);
	SCOTCH_archCmplt(&archdat, numTargetSE);
	SCOTCH_graphMap(&grafdat, &archdat, &stradat, &parttab[0]);

	sbfSELevel level;
	level.setSize(numElems);
	level.setLevelIndex(1);
	for(int ct = 0; ct < numElems; ct++) level.setIndex(ct, parttab[ct]);

	level.writeToFile(lName.str().c_str(), 1);
//	pMesh->addElementGroup();
//	for(int ct = 0; ct < numElems; ct++) pMesh->group(0)->addElement(ct, false);
//	pMesh->writeMeshToFiles(oiName.str().c_str(), ocName.str().c_str(), omName.str().c_str());

	cout << "DONE" << endl;

	return 0;
}
