#include "sbf.h"
#include <boost/program_options.hpp>
#include "elementorderconverter.h"

namespace po = boost::program_options;

int main ( int argc, char **argv )
{
    std::string catalog, indName, crdName;
    std::string outSuf, indOutName, crdOutName;
    po::options_description desc ( "Program options" );
    int targetOrder;
    bool verbouse = false;
    desc.add_options()
    ( "help,h", "print help message" )
    ( "work-dir,w", po::value<std::string> ( &catalog )->default_value ( "" ), "work catalog" )
    ( "ind-file,i", po::value<std::string> ( &indName )->default_value ( "ind.sba" ), "ind file" )
    ( "crd-file,c", po::value<std::string> ( &crdName )->default_value ( "crd.sba" ), "crd file" )
    ( "suffix,s", po::value<std::string> ( &outSuf )->default_value ( "_mod" ),
      "suffix which will be added to output files" )
    ( "order,o", po::value<int> ( &targetOrder )->default_value ( 2 ), "target order of elements in mesh" )
    ( "verbouse,v", "make verbouse output" )
    ;
    po::variables_map vm;
    po::store ( po::parse_command_line ( argc, argv, desc ), vm );
    po::notify ( vm );

    if ( vm.count ( "help" ) || vm.count ( "h" ) ) { std::cout << desc << "\n"; return 1; }

    if ( vm.count ( "verbouse" ) || vm.count ( "v" ) ) verbouse = true;

    if ( catalog.size() && ( catalog.back() != '/' || catalog.back() != '\\' ) ) catalog += "/";

    std::stringstream iName, cName;
    iName << catalog << indName;
    cName << catalog << crdName;
    //Creating mesh
    std::unique_ptr<sbfMesh> pMesh ( std::make_unique<sbfMesh>() );
    if ( pMesh->readIndFromFile ( iName.str().c_str() ) )
    {std::cout << "error while reading mesh indexes file " << indName << std::endl; return 1;}
    if ( pMesh->readCrdFromFile ( cName.str().c_str() ) )
    {std::cout << "error while reading mesh coordinates file " << crdName << std::endl; return 1;}

    ElementOrderConverter conv;

    std::unique_ptr<sbfMesh> newMesh ( conv.convert ( pMesh.get(), targetOrder, verbouse ) );

    size_t pos;
    for ( auto &p : std::list<std::pair<const std::string &, std::string &>> {{indName, indOutName}, {crdName, crdOutName}} ) {
        if ( ( pos = p.first.find ( ".sba" ) ) != std::string::npos ) {
            p.second = catalog + p.first;
            p.second.insert ( pos, outSuf );
        }
        else
            p.second = catalog + p.first + outSuf + ".sba";
    }

    newMesh->writeIndToFile ( indOutName.c_str() );
    newMesh->writeCrdToFile ( crdOutName.c_str() );

    std::cout << "DONE" << std::endl;
    return 0;
}
