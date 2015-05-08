#include "reorderer.h"

#include "sbfMesh.h"
#include "sbfElement.h"
#include "sbfReporter.h"

#include <set>

#include "metis.h"

Reorderer::Reorderer()
{

}

Reorderer::~Reorderer()
{

}

std::pair<std::vector<int>, std::vector<int> > Reorderer::renumber ( const std::vector<int> &rowShifts,
                                                                     const std::vector<int> &columnIDs,
                                                                     bool makeReport )
{
    std::pair<std::vector<int>, std::vector<int> > reorderIndexes;
    reorderIndexes.first.resize ( rowShifts.size() - 1, -1 );
    reorderIndexes.second.resize ( rowShifts.size() - 1, -1 );

    //METIS related code
    //TODO try parallel version
    idx_t numNodes = rowShifts.size() - 1;
    std::vector<idx_t> xadj;
    std::vector<idx_t> adjncy;
    std::vector<idx_t> perm;
    std::vector<idx_t> iperm;
    xadj.reserve ( rowShifts.size() );
    adjncy.reserve ( columnIDs.size() );
    perm.resize ( rowShifts.size() - 1, -1 );
    iperm.resize ( rowShifts.size() - 1, -1 );
    for ( auto p : rowShifts ) xadj.push_back ( p );
    for ( auto p : columnIDs ) adjncy.push_back ( p );
    idx_t options[METIS_NOPTIONS];
    METIS_SetDefaultOptions ( options );

    auto getBandwidth = [&] ( const std::vector<idx_t> &xadj, const std::vector<idx_t> &adjncy ) {
        size_t bandwidth = 0;
        idx_t numNodes = xadj.size() - 1;
        for ( idx_t ct = 0; ct < numNodes; ++ct ) {
            bandwidth += std::max ( ct, adjncy[xadj[ct + 1] - 1] ) - std::min ( ct, adjncy[xadj[ct]] ) + 1;
        }
        return bandwidth;
    };

    size_t bandwidth0 = getBandwidth ( xadj, adjncy );
    if ( makeReport ) report ( "Before reordering full bandwidth is", bandwidth0 );

    //TODO add options modification
    int rez = METIS_NodeND ( &numNodes, xadj.data(), adjncy.data(), nullptr, options, perm.data(), iperm.data() );

    if ( rez != METIS_OK ) {
        std::string err;
        switch ( rez ) {
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
        report.error ( err );
        throw std::runtime_error ( err );
    }

    size_t newShift = 0;
    std::vector<idx_t> newXadj;
    std::vector<idx_t> newAdjncy;
    newXadj.reserve ( xadj.size() );
    newAdjncy.reserve ( adjncy.size() );
    newXadj.push_back ( newShift );
    for ( idx_t ct = 0; ct < numNodes; ++ct ) {
        int nodeToProcess = perm[ct];
        std::set<int> ids;
        for ( int shift = xadj[nodeToProcess]; shift < xadj[nodeToProcess + 1]; ++shift )
            ids.insert ( iperm[adjncy[shift]] );
        for ( auto i : ids )
            newAdjncy[newShift++] = i;
        newXadj.push_back ( newShift );
    }

    size_t bandwidth1 = getBandwidth ( newXadj, newAdjncy );
    if ( makeReport ) report ( "After reordering full bandwidth is", bandwidth1,
                                   "\nCompression is", static_cast<float> ( bandwidth0 ) / bandwidth1 );

    return reorderIndexes;
}

std::pair<std::vector<int>, std::vector<int> > Reorderer::renumber ( const sbfMesh *mesh, bool makeReport )
{
    std::vector<std::set<int>> connections;
    int maxNodeID = 0;
    mesh->applyToAllElements ( [&] ( const sbfElement & elem ) {
        auto indexes = elem.indexes();
        for ( size_t ct1 = 0; ct1 < indexes.size(); ++ct1 )
            maxNodeID = std::max ( maxNodeID, indexes[ct1] );
    } );
    connections.resize ( maxNodeID + 1 );
    mesh->applyToAllElements ( [&] ( const sbfElement & elem ) {
        auto indexes = elem.indexes();
        for ( size_t ct1 = 0; ct1 < indexes.size(); ++ct1 ) for ( size_t ct2 = 0; ct2 < indexes.size(); ++ct2 )
                if ( ct1 != ct2 )
                    connections[indexes[ct1]].insert ( indexes[ct2] );
    } );
    std::vector<int> rowShifts;
    std::vector<int> columnIDs;
    rowShifts.reserve ( mesh->numNodes() + 1 );
    size_t colLength = 0;
    for ( const auto &s : connections ) colLength += s.size();
    columnIDs.reserve ( colLength );
    colLength = 0;
    rowShifts.push_back ( colLength );
    for ( const auto &s : connections ) {
        columnIDs.insert ( columnIDs.end(), s.begin(), s.end() );
        colLength += s.size();
        rowShifts.push_back ( colLength );
    }

    return renumber ( rowShifts, columnIDs, makeReport );
}

