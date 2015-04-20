#include "elementorderconverter.h"

#include "sbfMesh.h"
#include "sbfElement.h"
#include "sbfNode.h"
#include <map>
#include <set>
#include <list>
#include <vector>

ElementOrderConverter::ElementOrderConverter()
{

}

sbfMesh *ElementOrderConverter::convert ( const sbfMesh *originalMesh,
                                          int targetOrder,
                                          bool verbouse,
                                          float mergeTolerance )
{
    sbfMesh *mesh = new sbfMesh;
    mesh->reserveElementsNumber ( originalMesh->numElements() );
    mesh->reserveNodesNumber ( originalMesh->numNodes() );

    std::map<int /*order*/, int /*num elements*/> originalOrders;
    originalMesh->applyToAllElements ( [&] ( const sbfElement & elem ) {
        originalOrders[elemOrder ( elem.type() )]++;
    } );

    if ( verbouse ) {
        originalMesh->printInfo();
        report ( "Original mesh contsins:" );
        for ( const auto &rec : originalOrders )
            report ( rec.second, " elements of order ", rec.first );
    }

    if ( originalOrders.size() > 1 ) {
        std::string err ( "Original mesh conteins elements of different orers" );
        report.error ( err );
        throw std::runtime_error ( err );
    }

    std::multimap<int /*rounded crd*/, int /*index*/> xCrdIndMap, yCrdIndMap, zCrdIndMap;
    //Low performance with threadpool
//    auto findNodeID = [&] ( const sbfNode & n ) {
//        sbfThreadPool pool(3);
//        std::set<int> xIndexes, yIndexes, zIndexes;
//        auto xRangeF = pool.enqueue([&](){return xCrdIndMap.equal_range ( static_cast<int> ( n.x() / mergeTolerance ) );});
//        auto yRangeF = pool.enqueue([&](){return yCrdIndMap.equal_range ( static_cast<int> ( n.y() / mergeTolerance ) );});
//        auto zRangeF = pool.enqueue([&](){return zCrdIndMap.equal_range ( static_cast<int> ( n.z() / mergeTolerance ) );});
//        auto xRange = xRangeF.get();
//        if ( xRange.first == xRange.second )
//            return -1;
//        auto xIndexesF = pool.enqueue([&](){
//        for ( auto xi = xRange.first; xi != xRange.second; ++xi ) {
//            xIndexes.insert ( xi->second );
//        }
//        return 0;
//        });
//        auto yRange = yRangeF.get();
//        if ( yRange.first == yRange.second )
//            return -1;
//        auto yIndexesF = pool.enqueue([&](){
//        for ( auto yi = yRange.first; yi != yRange.second; ++yi ) {
//            yIndexes.insert ( yi->second );
//        }
//        return 0;
//        });
//        auto zRange = zRangeF.get();
//        if ( zRange.first == zRange.second )
//            return -1;
//        auto zIndexesF = pool.enqueue([&](){
//        for ( auto zi = zRange.first; zi != zRange.second; ++zi ) {
//            zIndexes.insert ( zi->second );
//        }
//        return 0;
//        });
//        xIndexesF.get();
//        yIndexesF.get();
//        zIndexesF.get();
//        for ( auto xi : xIndexes ) {
//            if ( yIndexes.find ( xi ) != yIndexes.end() && zIndexes.find ( xi ) != zIndexes.end() )
//                return xi;
//        }
//        return -1;
//    };
    auto findNodeID = [&] ( const sbfNode & n ) {
        auto xRange = xCrdIndMap.equal_range ( static_cast<int> ( n.x() / mergeTolerance ) );
        auto yRange = yCrdIndMap.equal_range ( static_cast<int> ( n.y() / mergeTolerance ) );
        auto zRange = zCrdIndMap.equal_range ( static_cast<int> ( n.z() / mergeTolerance ) );
        if ( xRange.first == xRange.second || yRange.first == yRange.second || zRange.first == zRange.second )
            return -1;
        std::set<int> xIndexes, yIndexes, zIndexes;
        for ( auto xi = xRange.first; xi != xRange.second; ++xi ) {
            xIndexes.insert ( xi->second );
        }
        for ( auto yi = yRange.first; yi != yRange.second; ++yi ) {
            yIndexes.insert ( yi->second );
        }
        for ( auto zi = zRange.first; zi != zRange.second; ++zi ) {
            zIndexes.insert ( zi->second );
        }
        for ( auto xi : xIndexes ) {
            if ( yIndexes.find ( xi ) != yIndexes.end() && zIndexes.find ( xi ) != zIndexes.end() )
                return xi;
        }
        return -1;
    };
    auto addToCache = [&] ( const sbfNode & n, int index ) {
        xCrdIndMap.insert(std::make_pair(static_cast<int> ( n.x() / mergeTolerance ), index));
        yCrdIndMap.insert(std::make_pair(static_cast<int> ( n.y() / mergeTolerance ), index));
        zCrdIndMap.insert(std::make_pair(static_cast<int> ( n.z() / mergeTolerance ), index));
    };

    report.createNewProgress ( "Converting elements" );
    for ( int ctElem = 0; ctElem < originalMesh->numElements(); ++ctElem ) {
        if ( ctElem % 1000 == 0 ) report.updateProgress ( 0, originalMesh->numElements(), ctElem );
        const sbfElement &el = originalMesh->elem ( ctElem );
        switch ( el.type() ) {
        case ElementType::HEXAHEDRON_LINEAR:
            if ( targetOrder == 2 ) {
                auto indexes = el.indexes();
                std::vector<int> newIndexes;
                for ( const auto &ind : indexes ) {
                    const sbfNode &n = originalMesh->node ( ind );
//                    int id = findNodeID ( n );
//                    if ( id == -1 ) {
//                        id = mesh->addNode ( n, false );
//                        addToCache ( n, id );
//                    }
//                    newIndexes.push_back ( id );
                    newIndexes.push_back ( mesh->addNode ( n, true, mergeTolerance ) );
                }
                auto middleNode = [] ( const sbfNode & n0, const sbfNode & n1 ) {
                    return sbfNode ( ( n0.x() + n1.x() ) / 2,
                                     ( n0.y() + n1.y() ) / 2,
                                     ( n0.z() + n1.z() ) / 2 );
                };
                for ( std::pair<int, int> p : std::list<std::pair<int, int>> {
                {0, 1}, {1, 2}, {2, 3}, {3, 0},
                {0, 4}, {1, 5}, {2, 6}, {3, 7},
                {4, 5}, {5, 6}, {6, 7}, {7, 4}
            } ) {
                    const sbfNode &n = middleNode ( originalMesh->node ( indexes[p.first] ),
                                                    originalMesh->node ( indexes[p.second] ) );
//                    int id = findNodeID ( n );
//                    if ( id == -1 ) {
//                        id = mesh->addNode ( n, false );
//                        addToCache ( n, id );
//                    }
//                    newIndexes.push_back ( id );
                    newIndexes.push_back ( mesh->addNode ( middleNode ( originalMesh->node ( indexes[p.first] ),
                                                                                           originalMesh->node ( indexes[p.second] ) ),
                                                                                           true, mergeTolerance ) );
                }
                mesh->addElement ( sbfElement ( ElementType::HEXAHEDRON_QUADRATIC, newIndexes ) );
            }
            else {
                std::string err ( "Unimplemented branch in order convertion" );
                report.error ( err );
                throw std::runtime_error ( err );
            }
            break;
        default: {
            std::string err ( "Unimplemented branch in order convertion" );
            report.error ( err );
            throw std::runtime_error ( err );
        }
        break;
        }
    }
    report.finalizeProgress();

    return mesh;
}

