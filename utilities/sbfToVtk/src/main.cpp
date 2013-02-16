#include "sbfToVTK.h"

using namespace std;
using namespace sbf;

int main (int argc, char * argv[])
{
	sbfToVTKWriter * writer = new sbfToVTKWriter ();

//	writer->mtrName() = "mtr001.sba";
//	writer->nodesDataNames().push_back("uuu");
//	writer->solutionBundleNames().push_back("s01");
//	writer->solutionBundleNames().push_back("s02");
//	writer->solutionBundleNames().push_back("s03");
//	writer->solutionBundleNames().push_back("s04");

	writer->write();

	cout << "done" << endl;
	return 0;
}
