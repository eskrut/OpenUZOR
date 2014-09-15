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
public:
    void setVerbouse(bool verbose) { verbouse_ = verbose; }
    void setUseSeed(bool seed) { seed_ = seed; }
    void make(const std::vector<int> &numByLayers, const std::vector<double> &maxImbalanceByLayer = std::vector<double>());
    void write(const char *levelBaseName = "level");
    void setNumIterations(int numIterations) { numIterations_ = numIterations; }
    void setNumCuts(int numCuts) { numCuts_ = numCuts; }
private:
    int generateLevels(const std::vector<int> &numTargetByLayers, const std::vector<double> &maxImbalanceByLayer);
    //TODO add get partitioning statistics
};

#endif // SEBUILDER_H
