#include "reorderer.h"

#include "sbfMesh.h"
#include "sbfReporter.h"

#include <set>

#include "metis.h"

Reorderer::Reorderer()
{

}

Reorderer::~Reorderer()
{

}

std::pair<std::vector<int>, std::vector<int> > Reorderer::renumber(const std::vector<int> &rowShifts,
                                                                   const std::vector<int> &columnIDs,
                                                                   bool makeReport)
{
    std::pair<std::vector<int>, std::vector<int> > reorderIndexes;
    reorderIndexes.first.resize(rowShifts.size()-1, -1);
    reorderIndexes.second.resize(rowShifts.size()-1, -1);

    //METIS related code
    //TODO try parallel version
    idx_t numNodes = rowShifts.size()-1;
    std::vector<idx_t> xadj;
    std::vector<idx_t> adjncy;
    std::vector<idx_t> perm;
    std::vector<idx_t> iperm;
    xadj.reserve(rowShifts.size());
    adjncy.reserve(columnIDs.size());
    perm.resize(rowShifts.size()-1, -1);
    iperm.resize(rowShifts.size()-1, -1);
    for(auto p : rowShifts) xadj.push_back(p);
    for(auto p : columnIDs) adjncy.push_back(p);
    idx_t options[METIS_NOPTIONS];
    METIS_SetDefaultOptions(options);

    auto getBandwidth = [&](){
        size_t bandwidth = 0;
        for(idx_t ct = 0; ct < numNodes; ++ct){
            bandwidth += std::max(ct, adjncy[xadj[ct]]) - std::min(ct, adjncy[xadj[ct+1]-1]) + 1;
        }
        return bandwidth;
    };

    size_t bandwidth0 = getBandwidth();

    //TODO add options modification
    int rez = METIS_NodeND(&numNodes, xadj.data(), adjncy.data(), nullptr, options, perm.data(), iperm.data());

    if(rez != METIS_OK) {
        std::string err;
        switch (rez) {
        case METIS_ERROR_INPUT:
            err = "METIS error input";
            break;
        case METIS_ERROR_MEMORY:
            err = "METIS error memory";
            break;
        case METIS_ERROR:
            err = "METIS unrecognized error";
            break;
        default:
            err = "Some unregistered error from METIS";
            break;
        }
        report.error(err);
        throw std::runtime_error(err);
    }

    for(size_t ct = 0; ct < numNodes; ++ct){
        std::set<int> ids;
        for(int shift = xadj[ct]; shift < xadj[ct+1]; ++shift)
            ids.insert(iperm[adjncy[shift]]);
        int shift = xadj[ct];
        for(auto i : ids)
            adjncy[shift++] = i;
    }

    size_t bandwidth1 = getBandwidth();

    return reorderIndexes;
}

std::pair<std::vector<int>, std::vector<int> > Reorderer::renumber(const sbfMesh *mesh, bool makeReport)
{
    std::pair<std::vector<int>, std::vector<int> > reorderIndexes;

    return reorderIndexes;
}

