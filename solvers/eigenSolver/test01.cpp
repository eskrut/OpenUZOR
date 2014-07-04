#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include <eigen3/Eigen/Eigenvalues>
#include <eigen3/Eigen/Cholesky>

#include "sbf.h"
#include "eigenSolver.h"

BOOST_AUTO_TEST_SUITE(eigenBasics)

BOOST_AUTO_TEST_CASE(eigen)
{
    using namespace Eigen;
    using Eigen::MatrixXd;
    MatrixXd K(3, 3);
    MatrixXd M(3, 3);

    K(0, 0) = 2;    K(0, 1) = -1;   K(0, 2) = 0;
    K(1, 0) = -1;   K(1, 1) = 4;    K(1, 2) = -1;
    K(2, 0) = 0;    K(2, 1) = -1;   K(2, 2) = 2;

    M.fill(0);
    M(0, 0) = 0.5;
    M(1, 1) = 1;
    M(2, 2) = 0.5;

    report(K);
    report(M);

    LLT<MatrixXd> Lfactor(M);
    MatrixXd L = Lfactor.matrixL();
    MatrixXd L_inv = L.inverse();
    MatrixXd LT = L.transpose();
    MatrixXd L_minusT = LT.inverse();
    MatrixXd C = L_inv*K*L_minusT;
    EigenSolver<MatrixXd> ges(C);
    VectorXd l = ges.eigenvalues().transpose().real();
    MatrixXd F = L_minusT*ges.eigenvectors().real();

    report(l);
    report(F);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(simpleTest)

BOOST_AUTO_TEST_CASE(consoleBeam)
{
    CreateSmartAndRawPtr(sbfMesh, mesh);
    int L = 1;
    double M = 1.0;
    double I = 1.0;
    float xPart = 250;
    mesh->addNode(0, 0, 0, false);
    for(int ct = 1; ct <= xPart; ct++)
        mesh->addElement(sbfElement(ElementType::BEAM_LINEAR_6DOF, {ct-1, mesh->addNode(L/xPart*ct, 0, 0, false)}));
    mesh->setMtr(1);

    CreateSmartAndRawPtr(sbfPropertiesSet, new sbfPropertiesSet, propSet);

    auto steel = sbfMaterialProperties::makeMPropertiesSteel();
    steel->addTable(new sbfPropertyTable("shear module"));
    steel->propertyTable("shear module")->addNodeValue(24.0f, 8e10);
    steel->propertyTable("shear module")->setCurParam(24.0f);
    steel->addTable(new sbfPropertyTable("area"));
    steel->propertyTable("area")->addNodeValue(24.0f, 100000000.0f);
    steel->propertyTable("area")->setCurParam(24.0f);
    steel->addTable(new sbfPropertyTable("Ix"));
    steel->propertyTable("Ix")->addNodeValue(24.0f, I*10000);
    steel->propertyTable("Ix")->setCurParam(24.0f);
    steel->addTable(new sbfPropertyTable("Iy"));
    steel->propertyTable("Iy")->addNodeValue(24.0f, I*10000);
    steel->propertyTable("Iy")->setCurParam(24.0f);
    steel->addTable(new sbfPropertyTable("Iz"));
    steel->propertyTable("Iz")->addNodeValue(24.0f, I);
    steel->propertyTable("Iz")->setCurParam(24.0f);
    propSet->addMaterial(steel);

    CreateSmartAndRawPtr(sbfStiffMatrix, sbfStiffMatrix::New(mesh, propSet), stiff);
    stiff->computeSequantially();

    CreateSmartAndRawPtr(sbfMatrixIterator, stiff->createIterator(), iter);
    double *data = iter->diagonal(0);
    for(int ct = 0; ct < 6; ++ct) data[ct*(6+1)] *= 1e6;
    std::vector<double> mass;
    mass.resize(mesh->numNodes()*stiff->numDof(), M/xPart);
    for(int ct = 0; ct < mesh->numNodes(); ++ct) mass[ct*6+3] = mass[ct*6+4] = mass[ct*6+5] = 1e-5;

    CreateSmartAndRawPtr(EigenSolver, new EigenSolver(stiff, mass.data()), solver);
    sbfTimer<> t;
    t.start();
    solver->compute(10, 1e-6);
    t.stop();
    report(t.timeSpanStr());

    auto vvs = solver->valuesVectors();
    for(auto &vv : vvs) std::cout << std::sqrt(vv.first) << "\t";
    std::cout << std::endl;
    NodesData<double, 3> form("form", mesh);
    mesh->writeMeshToFiles();
    for(auto &vv : vvs) {
        for(int ct = 0; ct < mesh->numNodes(); ct++) for(int ct1 = 0; ct1 < 3; ct1++) form(ct, ct1) = vv.second.data()[ct*stiff->numDof()+ct1];
        form.writeToFile();
    }
}

BOOST_AUTO_TEST_SUITE_END()
