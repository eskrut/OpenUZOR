#ifndef SEBUILDER_H
#define SEBUILDER_H

#include "sbf.h"
#include "sbfAdditions.h"

#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_set>

class SEBuilder
{
public:
    SEBuilder(const sbfMesh *mesh, const bool targetCut);
    ~SEBuilder();
private:
    const sbfMesh *mesh_;
    std::vector< std::vector<sbfSElement *> > selevels_;
    sbfSELevelList seLevels_;
    bool verbouse_;
    bool targetCut_;
    int numIterations_;
    int numCuts_;
    bool seed_;
    bool inversePartition_;
public:
    void setVerbouse(bool verbose) { verbouse_ = verbose; }
    void setUseSeed(bool seed) { seed_ = seed; }
    void setUseInversePartition(bool inversePartition) { inversePartition_ = inversePartition; }
    void make(const std::vector<int> &numByLayers, const std::vector<double> &maxImbalanceByLayer = std::vector<double>());
    void write(const char *levelBaseName = "level");
    void setNumIterations(int numIterations) { numIterations_ = numIterations; }
    void setNumCuts(int numCuts) { numCuts_ = numCuts; }
private:
    void processFacesWeigthOwners(int numRegElems, std::vector<int> &facesOwners, std::vector<double> &facesWeigth);
    void processFacesWeigthOwners(const std::vector<int> &regElems, std::vector<int> &facesOwners, std::vector<double> &facesWeigth);
    void partSE(sbfSElement *sElem, int nParts, double maxImbalance, const std::vector<double> &globalFacesWeigth, const std::vector<int> &globalFacesOwners);
    int generateLevels(const std::vector<int> &numTargetByLayers, const std::vector<double> &maxImbalanceByLayer);
    int generateLevelsInverce(const std::vector<int> &numTargetByLayers, const std::vector<double> &maxImbalanceByLayer);
    //TODO add get partitioning statistics
};

#endif // SEBUILDER_H
