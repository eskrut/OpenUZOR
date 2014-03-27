#include "sbf.h"
#include "sbfAdditions.h"
#include <boost/program_options.hpp>
#include <scotch/scotch.h>

#include <vector>
#include <list>
#include <set>
#include <unordered_set>

namespace po = boost::program_options;

int generateLevels(std::vector< std::vector<sbfSElement *> >  & selements, std::vector<int> & numTargetByLayers);

int main(int argc, char ** argv)
{
	//int numTargetSE;
	vector<int> numTargetByLayers;
	string numTargetByLayersStr;
	string catalog, indName, crdName, mtrName, levelBase;
	po::options_description desc("Program options");
	desc.add_options()
					("help,h", "print help message")
					("num-se,n", po::value<string>(&numTargetByLayersStr)->default_value("16"), "target numbers of SE by layers i.e. '64,8,2'")
					("work-dir,w", po::value<string>(&catalog)->default_value(""), "work catalog")
					("ind-file,i", po::value<string>(&indName)->default_value("ind.sba"), "ind file")
					("crd-file,c", po::value<string>(&crdName)->default_value("crd.sba"), "crd file")
					("mtr-file,m", po::value<string>(&mtrName)->default_value("mtr001.sba"), "mtr file")
					("level-base,l", po::value<string>(&levelBase)->default_value("level"), "level files base name")
					;
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help") || vm.count("h")) { cout << desc << "\n"; return 1; }

	//extract number of SE by layers from cmd string
	stringstream sstr(numTargetByLayersStr);
	string entry;
	while(getline(sstr, entry, ',')) {
		stringstream ss(entry);
		int numTarget;
		ss >> numTarget;
		numTargetByLayers.push_back(numTarget);
	}

	stringstream iName, cName, mName, lName, oiName, ocName, omName;
	iName << catalog << indName;
	cName << catalog << crdName;
	mName << catalog << mtrName;
	lName << catalog << levelBase;
	//Creating mesh
	std::auto_ptr<sbfMesh> pMesh(new sbfMesh());
	if(pMesh->readMeshFromFiles(iName.str().c_str(), cName.str().c_str(), mName.str().c_str()))
	//if(pMesh->readIndFromFile(iName.str().c_str()))
	{cout << "error while mesh reading" << endl; return 1;}
	//pMesh->printInfo();
	//pMesh->optimizeNodesNumbering();

	int numElems = pMesh->numElements();

	//Prepare zero level of SE - each SE contains only one regular element
	vector<int> regIndex;
	regIndex.resize(1);
	vector< vector<sbfSElement *> > selevels;
	selevels.resize(numTargetByLayers.size() + 1);
	selevels[0].reserve(numElems*10);
	for(int ct = 0; ct < numElems; ct++)
		selevels[0].push_back( new sbfSElement(pMesh.get(), ct));
	for(size_t ct = 0; ct < numTargetByLayers.size(); ct++){
		selevels[ct+1].reserve(numTargetByLayers[ct]*10);
		for(int ct1 = 0; ct1 < numTargetByLayers[ct]; ct1++)
			selevels[ct+1].push_back( new sbfSElement(pMesh.get(), ct1));
	}
	for(int ct = 0; ct < numElems; ct++){
		regIndex[0] = ct;
		selevels[0][ct]->setRegElemIndexes(regIndex);
	}

	generateLevels(selevels, numTargetByLayers);

	for(size_t ct = 0; ct < numTargetByLayers.size(); ct++){
		cout << "Level " << ct+1 << " contains " << selevels[ct+1].size() << endl;
		sbfSELevel level;
		level.setSize(selevels[ct].size());
		level.setLevelIndex(ct+1);
		for(int ctSE = 0; ctSE < selevels[ct].size(); ctSE++) level.setIndex(ctSE, selevels[ct][ctSE]->parent()->index());

		level.writeToFile(lName.str().c_str(), ct+1);
	}

//	vector <double> facesWeigth;
//	vector <int> facesOwners;
//	vector <double> randoms;
//	randoms.resize(50);
//	srand (time(NULL));
//	for(size_t ct = 0; ct < randoms.size(); ct++) randoms[ct] = ((double)rand())/RAND_MAX;
//
//	facesWeigth.reserve(numElems*20);
//	facesOwners.reserve(numElems*20);
//
//	for(int elemID = 0; elemID < numElems; elemID++){//Loop on elements
//		sbfElement *elem = pMesh->elemPtr(elemID);
//		vector<int> indexes = elem->indexes();
//
//		double faceWeigth;
//		int facesOwner = elemID;
//		list<int> inds;
//		int count;
//
//		vector< vector<int> > facesNodesIndexes = elem->facesNodesIndexes();
//		for(vector< vector<int> >::iterator itFace = facesNodesIndexes.begin(); itFace != facesNodesIndexes.end(); itFace++){//Loop on faces
//			inds.clear();
//			for(vector<int>::iterator itIndex = (*itFace).begin(); itIndex != (*itFace).end(); itIndex++)
//				inds.push_back(*itIndex);
//			inds.sort();
//			count = 0; faceWeigth = 0; for(list<int>::iterator it = inds.begin(); it != inds.end(); it++) {faceWeigth += *it*randoms[count++];}
//			facesWeigth.push_back(faceWeigth);
//			facesOwners.push_back(facesOwner);
//		}//Loop on faces
//
//	}//Loop on elements
//
//	quickAssociatedSortUp<double, int>(&facesWeigth[0], &facesOwners[0], 0, facesWeigth.size()-1);
//
//	cout << "sort done" << endl;
//
//	vector< list <int> > elemNeibour;
//	elemNeibour.resize(numElems);
//
//	int vertnbr, edgenbr;
//	vertnbr = numElems;
//	edgenbr = 0;
//
//	int founded = 0, unfounded = 0;
//
//	vector <double>::iterator facesWeigthIt = facesWeigth.begin();
//	vector <int>::iterator facesOwnersIt = facesOwners.begin();
//	vector <double>::iterator facesWeigthEndM1 = facesWeigth.end() - 1;
//	for(; facesWeigthIt < facesWeigthEndM1; facesWeigthIt++, facesOwnersIt++){
//		if(*facesWeigthIt == *(facesWeigthIt+1)){
//			elemNeibour[*facesOwnersIt].push_back(*(facesOwnersIt+1));
//			elemNeibour[*(facesOwnersIt+1)].push_back(*facesOwnersIt);
//			facesWeigthIt++; facesOwnersIt++; edgenbr += 2;
//			founded++;
//		}
//		else unfounded++;
//	}
//
//	vector <int> verttab, edgetab, parttab;
//	verttab.resize(vertnbr+1);
//	edgetab.resize(edgenbr);
//	parttab.resize(numElems);
//	int count = 0;
//	verttab[0] = count;
//	for(size_t ct = 0; ct < elemNeibour.size(); ct++){
//		for(list <int>::iterator it = elemNeibour[ct].begin(); it != elemNeibour[ct].end(); it++) edgetab[count++] = *it;
//		verttab[ct+1] = count;
//	}
//
//	SCOTCH_Strat stradat;
//	SCOTCH_stratInit(&stradat);
//	//SCOTCH_stratGraphMapBuild(&stradat,  SCOTCH_STRATSPEED, numTargetByLayers[0], 0.1);
//	SCOTCH_Graph grafdat;
//	SCOTCH_graphInit(&grafdat);
//	SCOTCH_graphBuild(&grafdat, 0, vertnbr, &verttab[0], &verttab[1], NULL, NULL, edgenbr, &edgetab[0], NULL);
//	SCOTCH_graphCheck(&grafdat);
//	SCOTCH_Arch archdat;
//	SCOTCH_archInit(&archdat);
//	SCOTCH_archCmplt(&archdat, numTargetByLayers[0]);
//	SCOTCH_graphMap(&grafdat, &archdat, &stradat, &parttab[0]);
//
//	sbfSELevel level;
//	level.setSize(numElems);
//	level.setLevelIndex(1);
//	for(int ct = 0; ct < numElems; ct++) level.setIndex(ct, parttab[ct]);
//
//	level.writeToFile(lName.str().c_str(), 1);
////	pMesh->addElementGroup();
////	for(int ct = 0; ct < numElems; ct++) pMesh->group(0)->addElement(ct, false);
////	pMesh->writeMeshToFiles(oiName.str().c_str(), ocName.str().c_str(), omName.str().c_str());

	cout << "DONE" << endl;

	return 0;
}

int generateLevels(std::vector< std::vector<sbfSElement *> >  & selements, std::vector<int> & numTargetByLayers){

	sbfMesh * mesh = selements[0][0]->mesh();
	if(!mesh) return 1;
	int numRegElems = selements[0].size();
	vector <double> facesWeigth;
	vector <int> facesOwners;
	vector <double> randoms;
	randoms.resize(50);
	for(size_t ct = 0; ct < randoms.size(); ct++) randoms[ct] = ((double)rand())/RAND_MAX;

	facesWeigth.reserve(numRegElems*50);
	facesOwners.reserve(numRegElems*50);

	for(int elemID = 0; elemID < numRegElems; elemID++){//Loop on elements
		sbfElement *elem = mesh->elemPtr(elemID);
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

	quickAssociatedSort<double, int>(&facesWeigth[0], &facesOwners[0], 0, facesWeigth.size()-1);

	cout << "sort done" << endl;

	for(size_t ctLevel = 0; ctLevel < numTargetByLayers.size(); ctLevel++){//Loop on levels
		int numTargetSE = numTargetByLayers[ctLevel];
		int numSElems = selements[ctLevel].size();

		int vertnbr, edgenbr;
		vertnbr = numSElems;
		edgenbr = 0;

		vector< vector <int> > regElemIndexes;
		regElemIndexes.reserve(numSElems);
		for(size_t ct = 0; ct < numSElems; ct++)
			regElemIndexes.push_back(selements[ctLevel][ct]->regElemIndexes());

		vector< list <int> > elemNeibour;
		elemNeibour.resize(numSElems);

		int founded = 0, unfounded = 0;

		vector <double> facesWeigthNextLevel;
		vector <int> facesOwnersNextLevel;
		facesWeigthNextLevel.reserve(facesWeigth.size());
		facesOwnersNextLevel.reserve(facesOwners.size());
		vector <double>::iterator facesWeigthIt = facesWeigth.begin();
		vector <int>::iterator facesOwnersIt = facesOwners.begin();
		vector <double>::iterator facesWeigthEndM1 = facesWeigth.end() - 1;
		for(; facesWeigthIt < facesWeigthEndM1; facesWeigthIt++, facesOwnersIt++){
			if(*facesWeigthIt == *(facesWeigthIt+1)){
				//Check if *facesOwnersIt and *(facesOwnersIt+1) are in one SE
				int owner0, owner1;
				owner0 = *facesOwnersIt; owner1 = *(facesOwnersIt+1);
				int ownerSE0 = -1, ownerSE1 = -1;
				bool inSame = false;
				if(ctLevel == 0){ownerSE0 = owner0; ownerSE1 = owner1;}
				else{
//					for(int ct = 0; ct < numSElems; ct++){
//						int count = 0;
//						for(size_t ctElem = 0; ctElem < regElemIndexes[ct].size(); ctElem++){
//							if( regElemIndexes[ct][ctElem] == owner0){ ownerSE0 = ct; count++; }
//							else if( regElemIndexes[ct][ctElem] == owner1){ ownerSE1 = ct; count++; }
//							if(count > 1){inSame = true; break;}
//						}
//						if(inSame) break;
//					}
					sbfSElement * se = selements[0][owner0];
					for(int ct = 0; ct < ctLevel; ct++) se = se->parent();
					ownerSE0 = se->index();
					se = selements[0][owner1];
					for(int ct = 0; ct < ctLevel; ct++) se = se->parent();
					ownerSE1 = se->index();
					if(owner0 == owner1) inSame = true;
				}
				if (inSame){ facesWeigthIt++; facesOwnersIt++; continue; }
				elemNeibour[ownerSE1].push_back(ownerSE0);
				elemNeibour[ownerSE0].push_back(ownerSE1);
				facesWeigthNextLevel.push_back(*facesWeigthIt);
				facesWeigthNextLevel.push_back(*facesWeigthIt);
				facesOwnersNextLevel.push_back(*facesOwnersIt);
				facesOwnersNextLevel.push_back(*(facesOwnersIt+1));
				facesWeigthIt++; facesOwnersIt++;
				founded++;
			}
			else unfounded++;
		}

		vector <int> verttab, edgetab, edlotab, parttab;
		verttab.resize(vertnbr+1);
		parttab.resize(vertnbr);
		//edgetab.resize(edgenbr);
		//edgetab.resize(edgenbr);
		int count = 0;
		verttab[0] = count;
		for(size_t ct = 0; ct < elemNeibour.size(); ct++){
			list<int> elemNeibourAll, elemNeibourUnique, elemNeibourUniqueCount;
			for(list <int>::iterator it = elemNeibour[ct].begin(); it != elemNeibour[ct].end(); it++)
				elemNeibourAll.push_back(*it);
			elemNeibourAll.sort();
			elemNeibourUnique = elemNeibourAll;
			elemNeibourUnique.unique();
			list <int>::iterator elemNeibourUniqueBegin = elemNeibourUnique.begin();
			list <int>::iterator elemNeibourUniqueEnd = elemNeibourUnique.end();
			list <int>::iterator elemNeibourAllBegin = elemNeibourAll.begin();
			list <int>::iterator elemNeibourAllEnd = elemNeibourAll.end();
			list <int>::iterator itA = elemNeibourAllBegin;
			for(list <int>::iterator itU = elemNeibourUniqueBegin; itU != elemNeibourUniqueEnd; itU++)
			{
				int numAcuarence = 0;
				while(*itA == *itU && itA != elemNeibourAllEnd){
					numAcuarence++;
					itA++;
				}
				edgetab.push_back(*itU);
				edlotab.push_back(numAcuarence);
				count++;
				edgenbr++;
			}
			verttab[ct+1] = count;
		}

		SCOTCH_Strat stradat;
		SCOTCH_stratInit(&stradat);
		SCOTCH_Graph grafdat;
		SCOTCH_graphInit(&grafdat);
		SCOTCH_graphBuild(&grafdat, 0, vertnbr, &verttab[0], &verttab[1], NULL, NULL, edgenbr, &edgetab[0], &edlotab[0]);
		SCOTCH_graphCheck(&grafdat);
		SCOTCH_Arch archdat;
		SCOTCH_archInit(&archdat);
		SCOTCH_archCmplt(&archdat, numTargetSE);
		SCOTCH_graphMap(&grafdat, &archdat, &stradat, &parttab[0]);

		for(int ct = 0; ct < numSElems; ct++){
			selements[ctLevel][ct]->setParent(selements[ctLevel+1][parttab[ct]]);
			selements[ctLevel+1][parttab[ct]]->addChildren(selements[ctLevel][ct]);
			//selements[ctLevel][ct]->setIndex(ct);
		}

		//There are SElements with some disconnected clusters of elements.
		//For such SE split them to several SEs

		for(std::vector<sbfSElement *>::iterator seIT = selements[ctLevel+1].begin(); seIT != selements[ctLevel+1].end(); seIT++){//Loop on SElements including new ones
			set<int> allElemIndexes;//Indexes of all elements in this SE
			for(int ct = 0; ct < (*seIT)->numSElements(); ct++) allElemIndexes.insert((*seIT)->children(ct)->index());
			set<int> inOneSE;
			inOneSE.insert(*(allElemIndexes.begin()));
			bool flagChanges = true;
			while(flagChanges){
				flagChanges = false;
				for(set<int>::iterator it = inOneSE.begin(); it != inOneSE.end(); it++){
					for(list<int>::iterator itN = elemNeibour[*it].begin(); itN != elemNeibour[*it].end(); itN++){
						if(allElemIndexes.count(*itN) && !inOneSE.count(*itN)){
							inOneSE.insert(*itN);
							flagChanges = true;
						}
					}
				}
			}
			if(allElemIndexes.size() != inOneSE.size()){
				set<int> inOtherSE;
				(*seIT)->setChildrens(vector<sbfSElement *>{});
				(*seIT)->numSElements();
				for(set<int>::iterator it = inOneSE.begin(); it != inOneSE.end(); it++){
					(*seIT)->addChildren(selements[ctLevel][*it]);
					selements[ctLevel][*it]->setParent(*seIT);
				}
				for(set<int>::iterator it = allElemIndexes.begin(); it != allElemIndexes.end(); it++){
					if(!inOneSE.count(*it))
						inOtherSE.insert(*it);
				}
				selements[ctLevel+1].push_back(new sbfSElement((*seIT)->mesh(), selements[ctLevel+1].size()));
				//numTargetByLayers[ctLevel]++;
				for(set<int>::iterator it = inOtherSE.begin(); it != inOtherSE.end(); it++){
					selements[ctLevel+1].back()->addChildren(selements[ctLevel][*it]);
					selements[ctLevel][*it]->setParent(selements[ctLevel+1].back());
				}
			}
		}//Loop on SElements including new ones

		facesWeigth = facesWeigthNextLevel;
		facesOwners = facesOwnersNextLevel;

		cout << "level " << ctLevel << " done" << endl;
	}//Loop on levels

	return 0;
}
