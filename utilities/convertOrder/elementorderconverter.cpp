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

