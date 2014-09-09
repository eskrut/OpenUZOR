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
    SEBuilder(const sbfMesh *mesh);
    ~SEBuilder();
private:
    const sbfMesh *mesh_;
    std::vector< std::vector<sbfSElement *> > selevels_;
    sbfSELevelList seLevels_;
    bool verbouse_;
public:
    void setVerbouse(bool verbose) { verbouse_ = verbose; }
    void make(const std::vector<int> &numByLayers, const std::vector<double> &maxImbalanceByLayer = std::vector<double>());
    void write(const char *levelBaseName = "level");
    //TODO add get partitioning statistics
};

#endif // SEBUILDER_H
