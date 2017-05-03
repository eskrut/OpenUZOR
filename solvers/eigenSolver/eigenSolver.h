#ifndef EIGENSOLVER_H
#define EIGENSOLVER_H

#include "sbfStiffMatrix.h"

class EigenSolver
{
public:
    EigenSolver(const sbfStiffMatrix *stiff, const double *diagMass);
private:
    const sbfStiffMatrix *stiff_;
    const double *diagMass_;
    int numTargetEig_;
    int numEig_;

    std::vector<std::vector<double>> forms_;
    std::vector<std::vector<double>> M_forms_;
    std::vector<double> values_;
    std::vector<std::pair<double, std::vector<double>>> rez_;
public:
    void compute(int numTarget, double lambdaConvFactor = 1e-6, double formConvFactor = 1e-6, bool makeReport = true);
    std::vector<std::pair<double, std::vector<double>>> valuesVectors() const;
private:
    void solveReduced(double *K, double *M, double *lambda, double *phi);
};

#endif // EIGENSOLVER_H
