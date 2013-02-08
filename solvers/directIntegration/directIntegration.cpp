/*
 * directIntegration.cpp
 *
 *  Created on: Oct 4, 2012
 *      Author: eugen
 */

#include "sbf.h"

#include <time.h>
#include <cstdlib>
#include <memory>
#include <string>
#include <sstream>

using namespace std;
using namespace sbf;

int readDisplFile(const char * name, double *displ[2], int & numRecords);

int main(int argc, char ** argv){
	/*
	 * Solution of direct integration problem with central difference method
	 */

	string catalog = "/media/E/Work/models/RDS/";
	string indName = "ind.sba";
	string crdName = "crd.sba";
	string mtrName = "mtr001.sba";
	stringstream iName, cName, mName;
	iName << catalog << indName;
	cName << catalog << crdName;
	mName << catalog << mtrName;

	string displXName = "displX.dat";
	string displYName = "displY.dat";
	string displZName = "displV.dat";

	//TODO make command line parsing
	//Place command line parser here

	auto_ptr<sbfMesh> mesh(new sbfMesh());
	mesh->readMeshFromFiles(iName.str().c_str(), cName.str().c_str(), mName.str().c_str());
	mesh->printInfo();

	//try to optimize numbering
	mesh->optimizeNodesNumbering();

	//Create node data storages

	NodesData<double, 3> displ_m1("displ_m1", mesh.get());
	NodesData<double, 3> displ("displ", mesh.get());
	NodesData<double, 3> displ_p1("displ_p1", mesh.get());
	NodesData<double, 3> mass("mass", mesh.get());
	NodesData<double, 3> force("force", mesh.get());
	NodesData<double, 3> demp("demp", mesh.get());
	NodesData<double, 3> rezKU("rezKU", mesh.get());

	mass.null();
	force.null();
	demp.null();
	displ_m1.null();
	displ.null();

	//Create displacement data storages - time may be not uniform and equal between displacements
	double *displX[2], *displY[2], *displZ[2];
	int numRecordsDisplX, numRecordsDisplY, numRecordsDisplZ;
	//Read displ files
	if(readDisplFile((catalog+displXName).c_str(), displX, numRecordsDisplX)) return 1;
	if(readDisplFile((catalog+displYName).c_str(), displY, numRecordsDisplY)) return 1;
	if(readDisplFile((catalog+displZName).c_str(), displZ, numRecordsDisplZ)) return 1;

	//Creating properties set
	auto_ptr<sbfPropertiesSet> propSet(new sbfPropertiesSet);
	propSet->addMaterial(MaterialProperties::makeMPropertiesSteel());

	//Create stiffness matrix

	auto_ptr<StiffMatrixBlock3x3> stiff(new StiffMatrixBlock3x3());
	stiff->setType(UP_TREANGLE_MATRIX);
	stiff->setMesh(mesh.get());
	stiff->setPropSet(propSet.get());
	stiff->updateIndexesFromMesh(UP_TREANGLE_MATRIX);// <- doubled information !!
	stiff->null();
	stiff->compute();

	//TODO implement universal function
	//Compute mass
	sbfElement *elem;
	ElemStiffMatrixHexa8 *stiffHexa8 = new ElemStiffMatrixHexa8();
	stiffHexa8->setPropSet(propSet.get());
	vector<int> elemNodeIndexes;
	for(int ctElem = 0; ctElem < mesh->numElements(); ctElem++){//Loop on elements
		elem = mesh->elemPtr(ctElem);
		stiffHexa8->setElem(elem);
		double elemMass = stiffHexa8->computeMass();
		elemNodeIndexes = elem->indexes();
		int numNodes = (int)elemNodeIndexes.size();
		for(vector<int>::iterator it = elemNodeIndexes.begin(); it != elemNodeIndexes.end(); it++){
			mass.data(*it, 0) += elemMass/numNodes;
			mass.data(*it, 1) += elemMass/numNodes;
			mass.data(*it, 2) += elemMass/numNodes;
		}
	}//Loop on elements
	{
		double allMass = 0;
		for(int ctNode = 0; ctNode < mesh->numNodes(); ctNode++)
			allMass += mass.data(ctNode, 0);
		cout << "All mass is: " << allMass << endl;
	}

	double curT, startT, endT, deltaT, dPlotTime, plotTime;
	curT = startT = plotTime = 0.0;
	endT = max(max(displX[0][numRecordsDisplX], displY[0][numRecordsDisplY]), displZ[0][numRecordsDisplZ]);
	deltaT = 1.0e-5;
	dPlotTime = (endT - startT) / 1000;

	//HARD CODE !!!!!!!!!!!!!!!!!!
	vector<int> kinemLoadIndexes;
	sbfGroupFilter filt;
	double minZ = mesh->minZ();
	filt.setCrdZF(minZ*1.0001, minZ*0.9999);
	sbfNodeGroup *gr = mesh->addNodeGroup();
	gr->addFilter(filt);
	mesh->processNodeGroups();
	kinemLoadIndexes = gr->nodeIndList();
	cout << "number of found nodes for kinematic load is " << kinemLoadIndexes.size() << endl;

	getchar();

	return 0;
}

int readDisplFile(const char * name, double *displ[2], int & numRecords){
	ifstream in;
	int recordCount = 0;
	numRecords = recordCount;
	double curT, curD;
	in.open(name);
	if(!in.good()){
		cerr << "Error while reading file " << name << ". Exit." << endl;
		return 1;
	}
	while(!in.eof()){
		in >> curT >> curD;
		recordCount++;
	}
	in.close();
	in.open(name);
	displ[0] = new double [recordCount]; //time
	displ[1] = new double [recordCount]; //displ
	recordCount = 0;
	while(!in.eof()){
		in >> displ[0][recordCount] >> displ[1][recordCount];
		recordCount++;
	}
	in.close();
	numRecords = recordCount;
	cout << "File \"" << name << "\": " << numRecords << " records read." << endl;
	return 0;
}
