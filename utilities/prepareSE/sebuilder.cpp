#include "sebuilder.h"
#include <metis.h>
#include <functional>
#include <cassert>

SEBuilder::SEBuilder(const sbfMesh *mesh, const bool targetCut) :
    mesh_(mesh),
    verbouse_(false),
    targetCut_(targetCut),
    numIterations_(100),
    numCuts_(3),
    seed_(false),
    inversePartition_(false)
{
    seLevels_.setMesh(mesh_);
}

SEBuilder::~SEBuilder()
{
}

void SEBuilder::make(const std::vector<int> &numTargetByLayers, const std::vector<double> &maxImbalanceByLayer)
{
    const int numElems = mesh_->numElements();

    //Prepare zero level of SE - each SE contains only one regular element
    selevels_.resize(numTargetByLayers.size() + 1);
    selevels_[0].reserve(numElems*10);
    for(int ct = 0; ct < numElems; ct++)
        selevels_[0].push_back( new sbfSElement(mesh_, ct));
    for(size_t ct = 0; ct < numTargetByLayers.size(); ct++){
        selevels_[ct+1].reserve(numTargetByLayers[ct]*100);
        for(int ct1 = 0; ct1 < numTargetByLayers[ct]; ct1++)
            selevels_[ct+1].push_back( new sbfSElement(mesh_, ct1));
    }
    for(int ct = 0; ct < numElems; ct++)
        selevels_[0][ct]->setRegElemIndexes({ct});

    if( !inversePartition_ )
        generateLevels(numTargetByLayers, maxImbalanceByLayer);
    else
        generateLevelsInverce(numTargetByLayers, maxImbalanceByLayer);
}

void SEBuilder::write(const char *levelBaseName)
{
    const int numElems = mesh_->numElements();
    NodesData<int, 1> levelMtrs((levelBaseName + std::string("Mtr")).c_str(), numElems);
    for(size_t ct = 0; ct < selevels_.size()-1; ct++){
        std::cout << "Level " << ct+1 << " contains " << selevels_[ct+1].size() << std::endl;
        sbfSELevel level;
        level.setSize(selevels_[ct].size());
        level.setLevelIndex(ct+1);
        for(size_t ctSE = 0; ctSE < selevels_[ct].size(); ctSE++) level.setIndex(ctSE, selevels_[ct][ctSE]->parent()->index());

        level.writeToFile(levelBaseName, ct+1);

        for(int ctElem = 0; ctElem < mesh_->numElements(); ++ctElem) {
            auto se = selevels_[0][ctElem];
            for(int ctLevel = 0; ctLevel <= ct; ++ctLevel)
                se = se->parent();
            levelMtrs.data()[ctElem] = se->index() + 1;
        }
        levelMtrs.writeToFile<int>();
    }
}

void SEBuilder::processFacesWeigthOwners(int numRegElems, std::vector<int> &facesOwners, std::vector<double> &facesWeigth)
{
    std::vector<int> regElems;
    regElems.reserve(numRegElems);
    for(int elemID = 0; elemID < numRegElems; elemID++)
        regElems.push_back(elemID);
    processFacesWeigthOwners(regElems, facesOwners, facesWeigth);
}

void SEBuilder::processFacesWeigthOwners(const std::vector<int> &regElems, std::vector<int> &facesOwners, std::vector<double> &facesWeigth)
{
    std::vector <double> randoms;
    randoms.resize(50);
    for(size_t ct = 0; ct < randoms.size(); ct++) randoms[ct] = ((double)rand())/RAND_MAX;

    facesWeigth.reserve(regElems.size()*50);
    facesOwners.reserve(regElems.size()*50);

    for(auto elemID : regElems){//Loop on elements
        sbfElement *elem = const_cast<sbfMesh*>(mesh_)->elemPtr(elemID);

        double faceWeigth;
        int facesOwner = elemID;
        std::list<int> inds;
        int count;

        std::vector< std::vector<int> > facesNodesIndexes = elem->facesNodesIndexes();
        for(auto itFace = facesNodesIndexes.begin(); itFace != facesNodesIndexes.end(); itFace++){//Loop on faces
            inds.clear();
            for(auto itIndex = (*itFace).begin(); itIndex != (*itFace).end(); itIndex++)
                inds.push_back(*itIndex);
            inds.sort();
            count = 0; faceWeigth = 0; for(auto it = inds.begin(); it != inds.end(); it++) {faceWeigth += *it*randoms[count++];}
            facesWeigth.push_back(faceWeigth);
            facesOwners.push_back(facesOwner);
        }//Loop on faces

    }//Loop on elements

    quickAssociatedSort<double, int>(facesWeigth.data(), facesOwners.data(), 0, facesWeigth.size()-1);
}

void SEBuilder::partSE(sbfSElement *sElem, int nParts, double maxImbalance)
{
    std::vector<double> localFacesWeigth;
    std::vector<int> localFacesOwners;
    auto regElements = sElem->regElemIndexes();
    localFacesWeigth.reserve(regElements.size()*20);
    localFacesOwners.reserve(regElements.size()*20);
    processFacesWeigthOwners(regElements, localFacesOwners, localFacesWeigth);
    std::map<int, int> ownerMap, regElemMap;
    for(size_t ct = 0; ct < regElements.size(); ++ct) {
        ownerMap[regElements[ct]] = ct;
        regElemMap[ct] = regElements[ct];
    }

    int vertnbr, edgenbr;
    vertnbr = regElements.size();
    edgenbr = 0;

    std::vector< std::list <int> > elemNeibour;
    elemNeibour.resize(vertnbr);

    auto facesWeigthIt = localFacesWeigth.begin();
    auto facesOwnersIt = localFacesOwners.begin();
    auto facesWeigthEndM1 = localFacesWeigth.end() - 1;
    for(; facesWeigthIt < facesWeigthEndM1; facesWeigthIt++, facesOwnersIt++){
        if(*facesWeigthIt == *(facesWeigthIt+1)){
            auto owner0 = ownerMap[*facesOwnersIt]; auto owner1 = ownerMap[*(facesOwnersIt+1)];
            elemNeibour[owner1].push_back(owner0);
            elemNeibour[owner0].push_back(owner1);
            facesWeigthIt++; facesOwnersIt++;
        }
    }
    for(auto &rec : elemNeibour) if(rec.size() == 0) throw std::runtime_error("fail to construct neibours");

    std::vector<idx_t> verttab, edgetab, edlotab, parttab;
    verttab.resize(vertnbr+1);
    parttab.resize(vertnbr, 0);
    int count = 0;
    verttab[0] = count;
    for(size_t ct = 0; ct < elemNeibour.size(); ct++){
        std::list<int> elemNeibourAll, elemNeibourUnique;
        for(auto it = elemNeibour[ct].begin(); it != elemNeibour[ct].end(); it++)
            elemNeibourAll.push_back(*it);
        elemNeibourAll.sort();
        elemNeibourUnique = elemNeibourAll;
        elemNeibourUnique.unique();
        auto elemNeibourUniqueBegin = elemNeibourUnique.begin();
        auto elemNeibourUniqueEnd = elemNeibourUnique.end();
        auto elemNeibourAllBegin = elemNeibourAll.begin();
        auto elemNeibourAllEnd = elemNeibourAll.end();
        auto itA = elemNeibourAllBegin;
        for(auto itU = elemNeibourUniqueBegin; itU != elemNeibourUniqueEnd; itU++)
        {
            if(static_cast<int>(ct) != *itU){
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
        }
        verttab[ct+1] = count;
    }

    if(nParts > 1) {
        idx_t nvtxs = vertnbr, ncon = 1, nparts = nParts, objval;
        idx_t options[METIS_NOPTIONS];
        METIS_SetDefaultOptions(options);
        if (targetCut_)
            options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;
        else
            options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_VOL;
        options[METIS_OPTION_NUMBERING] = 0;
        options[METIS_OPTION_NITER] = numIterations_;
        options[METIS_OPTION_NCUTS] = numCuts_;
        options[METIS_OPTION_MINCONN] = 1;
        if(maxImbalance > 0)
            options[METIS_OPTION_UFACTOR] = static_cast<int>((maxImbalance-1)*1000);
        options[METIS_OPTION_CONTIG] = 1; //Force contiguous partitions
        if (seed_)
            options[METIS_OPTION_SEED] = time(nullptr);
        int rez = METIS_PartGraphKway(&nvtxs,
                                      &ncon,
                                      verttab.data(),
                                      edgetab.data(),
                                      /*idx_t *vwgt*/nullptr,
                                      /*idx_t *vsize*/nullptr,
                                      /*idx_t *adjwgt*/nullptr,
                                      &nparts,
                                      /*real_t *tpwgts*/nullptr,
                                      /*real_t *ubvec*/nullptr,
                                      options,
                                      &objval,
                                      parttab.data()
                                      );
        if ( rez != METIS_OK ) throw std::runtime_error("Metis runtime failed :(");
    }

    std::vector<std::vector<int>> childsRegElements;
    childsRegElements.resize(nParts);
    for(size_t ct = 0; ct < parttab.size(); ++ct)
        childsRegElements[parttab[ct]].push_back(regElements[ct]);

    std::vector<sbfSElement*> childSEs;
    for(int ct = 0; ct < nParts; ++ct){
        childSEs.push_back(new sbfSElement(sElem->mesh()));
        childSEs.back()->setRegElemIndexes(childsRegElements[ct]);
        childSEs.back()->setParent(sElem);
    }
    sElem->setRegElemIndexes(std::vector<int>());
    sElem->setChildrens(childSEs);
}

int SEBuilder::generateLevels(const std::vector<int> &numTargetByLayers, const std::vector<double> &maxImbalanceByLayer){
    int numRegElems = selevels_[0].size();
    std::vector <double> facesWeigth;
    std::vector <int> facesOwners;
    processFacesWeigthOwners(numRegElems, facesOwners, facesWeigth);

    if ( verbouse_ ) std::cout << "sort done" << std::endl;

    for(size_t ctLevel = 0; ctLevel < numTargetByLayers.size(); ctLevel++){//Loop on levels
        int numTargetSE = numTargetByLayers[ctLevel];
        int numSElems = selevels_[ctLevel].size();

        int vertnbr, edgenbr;
        vertnbr = numSElems;
        edgenbr = 0;

        std::vector< std::list <int> > elemNeibour;
        elemNeibour.resize(vertnbr);

        int founded = 0, unfounded = 0;

        std::vector <double> facesWeigthNextLevel;
        std::vector <int> facesOwnersNextLevel;
        facesWeigthNextLevel.reserve(facesWeigth.size());
        facesOwnersNextLevel.reserve(facesOwners.size());
        auto facesWeigthIt = facesWeigth.begin();
        auto facesOwnersIt = facesOwners.begin();
        auto facesWeigthEndM1 = facesWeigth.end() - 1;
        for(; facesWeigthIt < facesWeigthEndM1; facesWeigthIt++, facesOwnersIt++){
            if(*facesWeigthIt == *(facesWeigthIt+1)){
                //Check if *facesOwnersIt and *(facesOwnersIt+1) are in one SE
                int owner0, owner1;
                owner0 = *facesOwnersIt; owner1 = *(facesOwnersIt+1);
                int ownerSE0 = -1, ownerSE1 = -1;
                bool inSame = false;
                if(ctLevel == 0){ownerSE0 = owner0; ownerSE1 = owner1;}
                else{
                    sbfSElement * se = selevels_[0][owner0];
                    for(size_t ct = 0; ct < ctLevel; ct++) se = se->parent();
                    ownerSE0 = se->index();
                    se = selevels_[0][owner1];
                    for(size_t ct = 0; ct < ctLevel; ct++) se = se->parent();
                    ownerSE1 = se->index();
                    if(ownerSE0 == ownerSE1) inSame = true;
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

        std::vector<idx_t> verttab, edgetab, edlotab, parttab;
        verttab.resize(vertnbr+1);
        parttab.resize(vertnbr);
        int count = 0;
        verttab[0] = count;
        for(size_t ct = 0; ct < elemNeibour.size(); ct++){
            std::list<int> elemNeibourAll, elemNeibourUnique;
            for(auto it = elemNeibour[ct].begin(); it != elemNeibour[ct].end(); it++)
                elemNeibourAll.push_back(*it);
            elemNeibourAll.sort();
            elemNeibourUnique = elemNeibourAll;
            elemNeibourUnique.unique();
            auto elemNeibourUniqueBegin = elemNeibourUnique.begin();
            auto elemNeibourUniqueEnd = elemNeibourUnique.end();
            auto elemNeibourAllBegin = elemNeibourAll.begin();
            auto elemNeibourAllEnd = elemNeibourAll.end();
            auto itA = elemNeibourAllBegin;
            for(auto itU = elemNeibourUniqueBegin; itU != elemNeibourUniqueEnd; itU++)
            {
                if(static_cast<int>(ct) != *itU){
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
            }
            verttab[ct+1] = count;
        }

        idx_t nvtxs = vertnbr, ncon = 1, nparts = numTargetSE, objval;
        idx_t options[METIS_NOPTIONS];
        METIS_SetDefaultOptions(options);
        if (targetCut_)
            options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;
        else
            options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_VOL;
        options[METIS_OPTION_NUMBERING] = 0;
        options[METIS_OPTION_NITER] = numIterations_;
        options[METIS_OPTION_NCUTS] = numCuts_;
        options[METIS_OPTION_MINCONN] = 1;
        double maxLayerImbalance = maxImbalanceByLayer.back();
        if( ctLevel < maxImbalanceByLayer.size() ) maxLayerImbalance = maxImbalanceByLayer.at(ctLevel);
        options[METIS_OPTION_UFACTOR] = static_cast<int>((maxLayerImbalance-1)*1000);
        options[METIS_OPTION_CONTIG] = 1; //Force contiguous partitions
        if (seed_)
            options[METIS_OPTION_SEED] = time(nullptr);
//        options[METIS_OPTION_DBGLVL] = 1;
        int rez = METIS_PartGraphKway(&nvtxs,
                                      &ncon,
                                      verttab.data(),
                                      edgetab.data(),
                                      /*idx_t *vwgt*/nullptr,
                                      /*idx_t *vsize*/nullptr,
                                      /*idx_t *adjwgt*/nullptr,
                                      &nparts,
                                      /*real_t *tpwgts*/nullptr,
                                      /*real_t *ubvec*/nullptr,
                                      options,
                                      &objval,
                                      parttab.data()
                                      );
        if ( rez != METIS_OK ) throw std::runtime_error("Metis runtime failed :(");

        //BUGFIX sometimes returned parttab is strange - it is not continious and not based with 0
        {
            std::set<int> parttabEntries;
            for(auto p : parttab) parttabEntries.insert(p);
            if ( parttabEntries.size() != nparts )
                throw std::runtime_error("Metis failed to make partitioning. This is sad :( You may try to change amounts of SE in failed level. Targeting number of super elements of half previous level size often leads to this behavior.");
//            std::map<int, int> partMap;
//            int count = 0;
//            for(auto p : parttabEntries) partMap.insert(std::make_pair(p, count++));
//            for(auto &p : parttab) p = partMap[p];
//            assert(parttabEntries.size() == nparts);
        }


        for(int ct = 0; ct < numSElems; ct++){
            selevels_[ctLevel][ct]->setParent(selevels_[ctLevel+1][parttab[ct]]);
            selevels_[ctLevel+1][parttab[ct]]->addChildren(selevels_[ctLevel][ct]);
        }

        //Metis solves following problem, but still it should be checked
        //There are SElements with some disconnected clusters of elements.
        //For such SE split them to several SEs

        for(auto seIT = selevels_[ctLevel+1].begin(); seIT != selevels_[ctLevel+1].end(); seIT++){//Loop on SElements including new ones
            std::set<int> allElemIndexes;//Indexes of all elements in this SE
            for(int ct = 0; ct < (*seIT)->numSElements(); ct++) allElemIndexes.insert((*seIT)->children(ct)->index());
            std::set<int> inOneSE;
            inOneSE.insert(*(allElemIndexes.begin()));
            bool flagChanges = true;
            while(flagChanges){
                flagChanges = false;
                for(auto it = inOneSE.begin(); it != inOneSE.end(); it++){
                    for(auto itN = elemNeibour[*it].begin(); itN != elemNeibour[*it].end(); itN++){
                        if(allElemIndexes.count(*itN) && !inOneSE.count(*itN)){
                            inOneSE.insert(*itN);
                            flagChanges = true;
                        }
                    }
                }
            }
            if(allElemIndexes.size() != inOneSE.size()){
                std::set<int> inOtherSE;
                (*seIT)->setChildrens(std::vector<sbfSElement *>{});
                (*seIT)->numSElements();
                for(auto it = inOneSE.begin(); it != inOneSE.end(); it++){
                    (*seIT)->addChildren(selevels_[ctLevel][*it]);
                    selevels_[ctLevel][*it]->setParent(*seIT);
                }
                for(auto it = allElemIndexes.begin(); it != allElemIndexes.end(); it++){
                    if(!inOneSE.count(*it))
                        inOtherSE.insert(*it);
                }
                sbfSElement *additionalSE = new sbfSElement((*seIT)->mesh(), selevels_[ctLevel+1].size());
                for(auto it = inOtherSE.begin(); it != inOtherSE.end(); it++){
                    additionalSE->addChildren(selevels_[ctLevel][*it]);
                    selevels_[ctLevel][*it]->setParent(additionalSE);
                }
                selevels_[ctLevel+1].push_back(additionalSE);
                if ( verbouse_ ) std::cout << "Split disconnected SE to two of sizes: " << inOneSE.size() << ", " << inOtherSE.size() << std::endl;
            }
        }//Loop on SElements including new ones

        facesWeigth = facesWeigthNextLevel;
        facesOwners = facesOwnersNextLevel;

        std::cout << "level " << ctLevel << " done" << std::endl;

        //get statistics
        std::list<int> selems;
        int numAll = 0;
        for(auto se : selevels_[ctLevel+1]){
            selems.push_back(se->numSElements());
            numAll += se->numSElements();
        }
        if(numAll != numSElems)
            throw std::runtime_error("SElements contain not all elements of previous layer");
        selems.sort();
        selems.reverse();
        if ( verbouse_ ) { for (auto se : selems ) std::cout << se << "\t"; std::cout << "Imbalance is " << (1.0*selems.front())/selems.back() << std::endl; }
    }//Loop on levels
    if(verbouse_) {
        using SEID = std::pair<int, int>;
        auto comp = [](const SEID &left, const SEID &right){
            if (left.first < right.first) return true;
            else if(left.first == right.first) return left.second < right.second;
            else return false;
        };
        std::map<SEID, sbfSElement::SEStat, std::function<bool(const SEID &left, const SEID &right)>> SEStats(comp);
        sbfSElement::update(selevels_.back().begin(), selevels_.back().end());
//        for(auto se : selevels_.back())
//            se->updateStat();
        for(int ctLevel = 1; ctLevel < selevels_.size(); ++ctLevel)
            for(int ct = 0; ct < selevels_[ctLevel].size(); ++ct)
                SEStats.insert(std::make_pair(std::make_pair(ctLevel, ct+1), selevels_[ctLevel][ct]->stat()));
        report("Level ID", "SE ID", "Nodes inner", "Nodes outer", "Num elements");
        for(const auto &rec : SEStats)
            report(rec.first.first, rec.first.second, rec.second.numInnerNodes, rec.second.numOuterNodes, rec.second.numSEelements);
    }
    return 0;
}

int SEBuilder::generateLevelsInverce(const std::vector<int> &numTargetByLayers, const std::vector<double> &maxImbalanceByLayer)
{
    int numRegElems = selevels_[0].size();

    //Construct top-level fake SElement
    auto fakeSE = new sbfSElement(mesh_);
    std::vector<int> elemsIDs;
    elemsIDs.resize(numRegElems);
    for(size_t ct = 0; ct < numRegElems; ++ct) elemsIDs[ct] = ct;
    fakeSE->setRegElemIndexes(elemsIDs);

    std::vector<sbfSElement*> curSElements;
    curSElements.push_back(fakeSE);

    for(int ctLevel = numTargetByLayers.size()-1; ctLevel >= 0; --ctLevel) {
        int numTargetInThisLevel = numTargetByLayers[ctLevel];
        int numAttached = 0;
        std::vector<int> numParts;
        std::vector<sbfSElement*> nextSElements;
        double maxLayerImbalance = maxImbalanceByLayer.back();
        if( ctLevel < maxImbalanceByLayer.size() ) maxLayerImbalance = maxImbalanceByLayer.at(ctLevel);
        for(int ct = 0; ct < curSElements.size(); ++ct) {
            numParts.push_back((numTargetInThisLevel - numAttached)/(curSElements.size()-ct));
            numAttached += numParts.back();
            partSE(curSElements[ct], numParts.back(), maxLayerImbalance);
            for(int ctChild = 0; ctChild < curSElements[ct]->numSElements(); ++ctChild )
                nextSElements.push_back(curSElements[ct]->children(ctChild));
        }
        for (int ctSE = 0; ctSE < nextSElements.size(); ++ctSE) {
            selevels_[ctLevel+1][ctSE] = nextSElements[ctSE];
            nextSElements[ctSE]->setIndex(ctSE);
        }
        curSElements = nextSElements;
        if ( verbouse_ ) std::cout << "Level " << ctLevel+1 << " done" << std::endl;
    }
    for(auto se : curSElements) {
        auto regEls = se->regElemIndexes();
        for(auto el : regEls) selevels_[0][el]->setParent(se);
    }
}
