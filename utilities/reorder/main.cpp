#include <boost/program_options.hpp>
#include <iostream>
#include "reorderer.h"
#include "sbfMesh.h"

namespace po = boost::program_options;

int main ( int argc, char **argv )
{
    std::string catalog, indName;
    po::options_description desc ( "Program options" );
    bool verbouse = false;
    desc.add_options()
    ( "help,h", "print help message" )
    ( "work-dir,w", po::value<std::string> ( &catalog )->default_value ( "" ), "work catalog" )
    ( "ind-file,i", po::value<std::string> ( &indName )->default_value ( "ind.sba" ), "ind file" )
    ( "verbouse,v", "make verbouse output" )
    ;
    po::variables_map vm;
    po::store ( po::parse_command_line ( argc, argv, desc ), vm );
    po::notify ( vm );

    if ( vm.count ( "help" ) || vm.count ( "h" ) ) { std::cout << desc << "\n"; return 1; }

    if ( vm.count ( "verbouse" ) || vm.count ( "v" ) ) verbouse = true;

    if ( catalog.size() && ( catalog.back() != '/' || catalog.back() != '\\' ) ) catalog += "/";

    std::stringstream iName;
    iName << catalog << indName;
    //Creating mesh
    std::unique_ptr<sbfMesh> pMesh ( std::make_unique<sbfMesh>() );
    if ( pMesh->readIndFromFile ( iName.str().c_str() ) )
    {std::cout << "error while mesh reading" << std::endl; return 1;}

    Reorderer rord;

    rord.renumber ( pMesh.get() );

    return 0;
}
