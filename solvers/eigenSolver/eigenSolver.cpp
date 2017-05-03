#include "eigenSolver.h"

#include <Eigen/Eigenvalues>
#include <Eigen/Cholesky>

#include "sbfMatrixIterator.h"
#include "sbfStiffMatrix.h"
#include "sbfAdditions.h"

auto mul = [](sbfMatrixIterator *iterator, double *d, double *f){
    const int numNodes = iterator->matrix()->mesh()->numNodes();
    const int numDof = iterator->matrix()->numDof();
    const int numAllDof = numDof*numNodes;
    for(int ct = 0; ct < numAllDof; ct++) f[ct] = 0;
    for(int ctNode = 0; ctNode < numNodes; ++ctNode) {
        iterator->setToRow(ctNode);
        while(iterator->isValid()) {
            double *data = iterator->data();
            double *dataF = f+numDof*ctNode;
            int columnID = iterator->column();
            for(int ct1 = 0; ct1 < numDof; ++ct1)
                for(int ct2 = 0; ct2 < numDof; ++ct2)
                    dataF[ct1] += data[ct1*numDof+ct2]*d[numDof*columnID+ct2];
            iterator->next();
        }
    }
};
auto solve = [](sbfMatrixIterator *stiffIterator, sbfMatrixIterator *iCholIterator, double *force, double *disp, double rNormTarget){
    auto iChol = const_cast<sbfStiffMatrix*>(iCholIterator->matrix());
    auto stiff = const_cast<sbfStiffMatrix*>(stiffIterator->matrix());
    auto mesh = stiff->mesh();
    const int numAllDof = mesh->numNodes()*stiff->numDof();

    //_p1 stands for 'plus one'
    CreateSmartAndRawPtr(double, new double [numAllDof], KU);
    CreateSmartAndRawPtr(double, new double [numAllDof], Kp);
    CreateSmartAndRawPtr(double, new double [numAllDof], r);
    CreateSmartAndRawPtr(double, new double [numAllDof], r_p1);
    CreateSmartAndRawPtr(double, new double [numAllDof], z);
    CreateSmartAndRawPtr(double, new double [numAllDof], z_p1);
    CreateSmartAndRawPtr(double, new double [numAllDof], p);
    CreateSmartAndRawPtr(double, new double [numAllDof], p_p1);
    double alpha, betta;

    iChol->solve_L_LT_u_eq_f(disp, force, iCholIterator);
////    for(int ct = 0; ct < numAllDof; ++ct) disp[ct] = 0.0;
//    mul(stiffIterator, disp, KU);
//    for(int ct = 0; ct < numAllDof; ++ct) r[ct] = force[ct] - KU[ct];

//    //Solve L*L^T*z = r
//    iChol->solve_L_LT_u_eq_f(z, r, iCholIterator);
//    for(int ct = 0; ct < numAllDof; ++ct) p[ct] = z[ct];

//    double rNorm = 0.0;
//    for(int ct = 0; ct < numAllDof; ++ct) if ( rNorm < fabs(r[ct]) ) rNorm = std::fabs(r[ct]);
//    int numIterations = 0;

//    while(rNorm > rNormTarget) {
//        mul(stiffIterator, p, Kp);
//        double scal1 = 0, scal2 = 0;
//        for(int ct = 0; ct < numAllDof; ++ct) {
//            scal1 += z[ct]*r[ct];
//            scal2 += p[ct]*Kp[ct];
//        }
//        alpha = scal1/scal2;
//        for(int ct = 0; ct < numAllDof; ++ct) {
//            disp[ct] += alpha*p[ct];
//            r_p1[ct] = r[ct] - alpha*Kp[ct];
//        }
//        iChol->solve_L_LT_u_eq_f(z_p1, r_p1, iCholIterator);
//        scal1 = 0; scal2 = 0;
//        for(int ct = 0; ct < numAllDof; ++ct) {
//            scal1 += z_p1[ct]*r_p1[ct];
//            scal2 += z[ct]*r[ct];
//        }
//        betta = scal1/scal2;
//        for(int ct = 0; ct < numAllDof; ++ct) {
//            p_p1[ct] = z_p1[ct] + betta*p[ct];
//            z[ct] = z_p1[ct];
//            p[ct] = p_p1[ct];
//            r[ct] = r_p1[ct];
//        }
//        rNorm = 0.0;
//        for(int ct = 0; ct < numAllDof; ++ct) if ( rNorm < fabs(r[ct]) ) rNorm = std::fabs(r[ct]);
//        numIterations++;
//    }
////    report("rNorm=", rNorm, " numIter=", numIterations);
};

EigenSolver::EigenSolver(const sbfStiffMatrix *stiff, const double *diagMass) :
    stiff_(stiff),
    diagMass_(diagMass),
    numEig_(0)
{
}

void EigenSolver::compute(int numTarget, double lambdaConvFactor, double formConvFactor, bool makeReport)
{

    //Some assertions
    if( !diagMass_ ) throw std::runtime_error("Non-diagonal matrixes not implemented");
    if( !(stiff_->type() & MatrixType::FULL_MATRIX) ) throw std::runtime_error("Currently only full stiffness matryx implemented");

    //Sub-space iteration method - Duc T.Nguyen - Finite Element Methods - Parallel-sparce static and eigen solutions. p.286
    numTargetEig_ = numTarget;
    numEig_ = std::min(2*(numTarget+1), (numTarget+1)+8);
    values_.resize(numEig_, 0);
    forms_.resize(numEig_);
    M_forms_.resize(numEig_);
    const int numDof = stiff_->numDof();
    const int numNodes = stiff_->mesh()->numNodes();
    const int numAllDofs = numDof*numNodes;
    for(auto &f : forms_)
        f.resize(numAllDofs, 0);
    for(auto &f : M_forms_)
        f.resize(numAllDofs, 0);
    //Generate initial gess
    for(int ct = 0; ct < numEig_; ++ct) forms_[ct][ct] = 1.0;

    CreateSmartAndRawPtr(sbfStiffMatrix, const_cast<sbfStiffMatrix*>(stiff_)->createChol(makeReport), iChol);
    CreateSmartAndRawPtr(sbfMatrixIterator, const_cast<sbfStiffMatrix*>(stiff_)->createIterator(), stiffIterator);
    CreateSmartAndRawPtr(sbfMatrixIterator, iChol->createIterator(), iCholIterator);
    CreateSmartAndRawPtr(double, new double [numEig_*numEig_], K_reduced);
    CreateSmartAndRawPtr(double, new double [numEig_*numEig_], M_reduced);
    CreateSmartAndRawPtr(double, new double [numEig_], lambda_reduced);
    CreateSmartAndRawPtr(double, new double [numEig_], lambda_reduced_prev);
    CreateSmartAndRawPtr(double, new double [numEig_*numEig_], Phi_reduced);

    for(int ct = 0; ct < numEig_; ++ct) lambda_reduced_prev[ct] = 0.0;
    bool converged = false;
    while ( !converged ) {
        //Compute M * X_k
        for(auto &col : M_forms_) for(auto &val : col) val = 0;
        if(diagMass_) {
            for(int ctCol = 0; ctCol < numEig_; ++ctCol) {
                for(int ctRow = 0; ctRow < numAllDofs; ++ctRow) {
                    M_forms_[ctCol][ctRow] = diagMass_[ctRow]*forms_[ctCol][ctRow];
                }
            }
        }
        else {
            //TODO Implement multiplication for non diagonal mass matrix
        }
        //Solve for new approximation
        // K * X_{k+1} = M * X_k
        //TODO this could be done in parallel
        for(int ct = 0; ct < numEig_; ++ct)
            solve(stiffIterator, iCholIterator, M_forms_[ct].data(), forms_[ct].data(), formConvFactor);

        //Compute reduced matrixes
        //  K * X_{k+1}
        //Use M_forms_ as temporary storage
        for(auto &f : M_forms_) for(auto &v : f) v = 0;
        for(int ctRow = 0; ctRow < numNodes; ++ctRow) {
            stiffIterator->setToRow(ctRow);
            while(stiffIterator->isValid()) {
                double *stiffData = stiffIterator->data();
                int colID = stiffIterator->column();
                //TODO this could be done in parallel
                for(int ctForm = 0; ctForm < numEig_; ++ctForm) {
                    double *formData = forms_[ctForm].data() + colID*numDof;
                    double *rezData = M_forms_[ctForm].data() + ctRow*numDof;
                    for(int ctBlockRow = 0; ctBlockRow < numDof; ++ctBlockRow)
                        for(int ctBlockColumn = 0; ctBlockColumn < numDof; ++ctBlockColumn)
                            rezData[ctBlockRow] += stiffData[ctBlockRow*numDof + ctBlockColumn] * formData[ctBlockColumn];
                }
                stiffIterator->next();
            }
        }
        //  X_{k+1}^T * result_of( K * X_{k+1} )
        for(int ct = 0; ct < numEig_*numEig_; ++ct) K_reduced[ct] = 0.0;
        for(int ctRow = 0; ctRow < numEig_; ++ctRow)
            for(int ctColumn = 0; ctColumn < numEig_; ++ctColumn)
                for(int ct = 0; ct < numAllDofs; ++ct)
                    K_reduced[ctRow*numEig_ + ctColumn] += forms_[ctRow][ct] * M_forms_[ctColumn][ct];
        //  M * X_{k+1}
        //Use M_forms_ as temporary storage
        for(auto &f : M_forms_) for(auto &v : f) v = 0;
        for(int ctRow = 0; ctRow < numNodes; ++ctRow) {
            const double *massData = diagMass_ + ctRow*numDof;
            int colID = ctRow;
            for(int ctForm = 0; ctForm < numEig_; ++ctForm) {
                double *formData = forms_[ctForm].data() + colID*numDof;
                double *rezData = M_forms_[ctForm].data() + ctRow*numDof;
                for(int ctBlockRow = 0; ctBlockRow < numDof; ++ctBlockRow)
                    rezData[ctBlockRow] += massData[ctBlockRow] * formData[ctBlockRow];
            }
        }
        //  X_{k+1}^T * result_of( M * X_{k+1} )
        for(int ct = 0; ct < numEig_*numEig_; ++ct) M_reduced[ct] = 0.0;
        for(int ctRow = 0; ctRow < numEig_; ++ctRow)
            for(int ctColumn = 0; ctColumn < numEig_; ++ctColumn)
                for(int ct = 0; ct < numAllDofs; ++ct)
                    M_reduced[ctRow*numEig_ + ctColumn] += forms_[ctRow][ct] * M_forms_[ctColumn][ct];

        //Solve reduced eigen problem
        solveReduced(K_reduced, M_reduced, lambda_reduced, Phi_reduced);

        //Find improved approximation
        for(int ctForm = 0; ctForm < numEig_; ++ctForm) for(int ct = 0; ct < numAllDofs; ct++) {
            M_forms_[ctForm][ct] = forms_[ctForm][ct];
            forms_[ctForm][ct] = 0.0;
        }
        for(int ctRow = 0; ctRow < numAllDofs; ++ctRow)
            for(int ctColumn = 0; ctColumn < numEig_; ++ctColumn)
                for(int ct = 0; ct < numEig_; ++ct)
                    forms_[ctColumn][ctRow] += M_forms_[ct][ctRow] * Phi_reduced[ct*numEig_ + ctColumn];

        double convSum = 0.0;
        for(int ct = 0; ct < numEig_; ++ct) convSum += std::fabs((lambda_reduced[ct] - lambda_reduced_prev[ct])/lambda_reduced[ct]);
        if(convSum < lambdaConvFactor) converged = true;
        for(int ct = 0; ct < numEig_; ++ct) lambda_reduced_prev[ct] = lambda_reduced[ct];
        double minLambda = lambda_reduced[0];
        for(int ct = 0; ct < numEig_; ++ct) if( minLambda > lambda_reduced[ct] ) minLambda = lambda_reduced[ct];
        if(makeReport)
            report("convSum=", convSum, "first freq", std::sqrt(minLambda)/2.0/(std::atan(1)*4));
    }

    std::vector<double> tmpRez(lambda_reduced, lambda_reduced+numEig_);
    auto tmpForms = forms_;
    for(int ct = 0; ct < numTargetEig_; ct++){
        int minID = 0; double minVal = tmpRez[0];
        for(int ct = 0; ct < tmpRez.size(); ++ct) if(minVal > tmpRez[ct]) {minVal = tmpRez[ct]; minID = ct;}
        rez_.push_back(std::make_pair(minVal, tmpForms[minID]));
        tmpRez.erase(tmpRez.begin() + minID);
        tmpForms.erase(tmpForms.begin() + minID);
    }
}

std::vector<std::pair<double, std::vector<double> > > EigenSolver::valuesVectors() const
{
    return rez_;
}

void EigenSolver::solveReduced(double *K, double *M, double *lambda, double *phi)
{
    using namespace Eigen;
    using Eigen::MatrixXd;
    MatrixXd K_reduced(numEig_, numEig_);
    MatrixXd M_reduced(numEig_, numEig_);

    for(int ctRow = 0; ctRow < numEig_; ++ctRow)
        for(int ctCol = 0; ctCol < numEig_; ++ctCol) {
            K_reduced(ctRow, ctCol) = K[ctRow*numEig_ + ctCol];
            M_reduced(ctRow, ctCol) = M[ctRow*numEig_ + ctCol];
        }
    LLT<MatrixXd> Lfactor(M_reduced);
    MatrixXd L = Lfactor.matrixL();
    MatrixXd L_inv = L.inverse();
    MatrixXd LT = L.transpose();
    MatrixXd L_minusT = LT.inverse();
    MatrixXd C = L_inv*K_reduced*L_minusT;
    Eigen::EigenSolver<MatrixXd> ges(C);
    VectorXd l = ges.eigenvalues().transpose().real();
    MatrixXd F = L_minusT*ges.eigenvectors().real();

    for(int ctRow = 0; ctRow < numEig_; ++ctRow) {
        lambda[ctRow] = l(ctRow);
        for(int ctCol = 0; ctCol < numEig_; ++ctCol)
            phi[ctRow*numEig_ + ctCol] = F(ctRow, ctCol);
    }
}
