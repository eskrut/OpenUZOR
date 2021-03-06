#ifndef _SBFTOVTK_H_
#define _SBFTOVTK_H_

#include "sbf.h"

typedef float sbfReadWriteType;

struct SbaNameParts{
    SbaNameParts() = default;
    SbaNameParts(const std::string &base, int numDigits = 4, const std::string &ext = "sba") : base(base), numDigits(numDigits), ext(ext) {}
    std::string base;
    int numDigits;
    std::string ext;
    std::string construct(int curID = 1){
        std::stringstream fullName;
        fullName << base << std::setw(numDigits) << std::setfill('0') << curID << "." << ext;
        return fullName.str();
    }
};

class sbfToVTKWriter{
public:
    sbfToVTKWriter();
    ~sbfToVTKWriter();
private:
    sbfToVTKWriter(const sbfToVTKWriter & );
    sbfToVTKWriter & operator= (const sbfToVTKWriter & );
private:
    std::string indName_;
    std::string crdName_;
    std::string mtrName_;
    std::string namePrefix_;
    std::string catalog_;
    std::string vtkName_;
    std::string mtrBaseName_;
    int mtrNumDigits_;
    std::string levelBaseName_;
    int levelNumDigits_;
    std::string sbaExtention_;
    std::list<SbaNameParts> solutionBundleNames_;
    std::list<SbaNameParts> nodesDataNames_;
    bool flagValidNames_;
    bool flagSaveLevels_;
    int stepsToWrite_;
    bool flagUseCompression_;

    void check();
public:
    std::string & indName() {return indName_;}
    std::string & crdName() {return crdName_;}
    std::string & namePrefix() {return namePrefix_;}
    std::string & mtrName() {return mtrName_;}
    std::string & catalog() {return catalog_;}
    std::string & vtkName() {return vtkName_;}
    std::string & mtrBaseName() {return mtrBaseName_;}
    int & mtrNumDigits() {return mtrNumDigits_;}
    std::string & levelBaseName() {return levelBaseName_;}
    int & levelNumDigits() {return levelNumDigits_;}
    std::string & sbaExtention() {return sbaExtention_;}
    std::list<SbaNameParts> & solutionBundleNames() { return solutionBundleNames_; }
    std::list<SbaNameParts> & nodesDataNames() { return nodesDataNames_; }
    void setUseCompression(bool use) { flagUseCompression_ = use; }

    //TODO eliminate report output flag after resolving report objects issue
    int write(bool doReportOutput = true); // main write function
};

#endif// _SBFTOVTK_H_
