#ifndef COMMAND_OPTIONS_HPP
#define COMMAND_OPTIONS_HPP

#include "boost/program_options.hpp"
#include <fstream>

namespace po = boost::program_options;

#include "sbfReporter.h"

class sbfCmdOptions {
public:
    sbfCmdOptions(const std::string &name = std::string("Options")) :
        desc(po::options_description(name))
    {
        desc.add_options()
                ("help,h", "print help message");
    }

    sbfCmdOptions &makeMeshFilesOps(const std::string &iName = "ind.sba",
                                    const std::string &cName = "crd.sba",
                                    const std::string &mName = "mtr0001.sba",
                                    const std::string &prefix = "",
                                    const std::string &wDir = "./")
    {
        desc.add_options()
                ("work-dir,w", po::value<std::string>(&workDir)->default_value(wDir), "work catalog")
                ("ind-file,i", po::value<std::string>(&indName)->default_value(iName), "ind file")
                ("crd-file,c", po::value<std::string>(&crdName)->default_value(cName), "crd file")
                ("mtr-file,m", po::value<std::string>(&mtrName)->default_value(mName), "mtr file")
                ("name-prefix", po::value<std::string>(&namePrefix)->default_value(prefix), "prefix for mesh files names");
        return *this;
    }

    template<class T>
    sbfCmdOptions &add(const std::string &option,
                       T &bindValue,
                       const std::string &description = std::string(),
                       const T &defaultValue = T()
                       )
    {
        desc.add_options()
                (option.c_str(), po::value<T>(&bindValue)->default_value(defaultValue), description.c_str());
        return *this;
    }

    template<class T>
    sbfCmdOptions &add_required(const std::string &option,
                                T &bindValue,
                                const std::string &description = std::string()
            )
    {
        desc.add_options()
                (option.c_str(), po::value<T>(&bindValue)->required(), description.c_str());
        return *this;
    }

    sbfCmdOptions &add(const std::string &option,
                       std::string &bindValue,
                       const std::string &description = std::string(),
                       const std::string &defaultValue = std::string()
                       )
    {
        desc.add_options()
                (option.c_str(), po::value<std::string>(&bindValue)->default_value(defaultValue), description.c_str());
        return *this;
    }

    sbfCmdOptions &add(const std::string &option,
                       bool &bindValue,
                       const std::string &description = std::string(),
                       const bool &defaultValue = false
                       )
    {
        desc.add_options()
                (option.c_str(), po::bool_switch(&bindValue)->default_value(defaultValue), description.c_str());
        return *this;
    }

    void parceCmd(int argc, char **argv)
    {
        try {
            po::store(po::parse_command_line(argc, argv, desc), vm);
            po::notify( vm );
        }
        catch (std::exception &e) {
            report.raiseError("sbfCmdOptions::parceCmd:", e.what());
            throw e;
        }

        if(workDir.size() == 0)
            workDir = "./";
        else if( workDir.back() != '/')
            workDir += "/";
    }

    void parceIni(const std::string &iniFile)
    {
        std::ifstream settingsFile(iniFile, std::ifstream::in);
        if(settingsFile.good()) {
            try {
                po::store( po::parse_config_file( settingsFile , desc ), vm );
                settingsFile.close();
                po::notify( vm );
            }
            catch (std::exception &e) {
                report.raiseError("sbfCmdOptions::parceIni:", e.what());
                throw e;
            }

            if(workDir.size() == 0)
                workDir = "./";
            else if( workDir.back() != '/')
                workDir += "/";
        }
    }

    void parce(int argc, char **argv, const std::string &iniKey = std::string())
    {
        try {
            po::store(po::parse_command_line(argc, argv, desc), vm);
            if(iniKey.size()) {
                bool ok;
                auto w = getValue<std::string>("work-dir", &ok);
                auto i = getValue<std::string>(iniKey, &ok);
                if(!ok) report.raiseError("Cant parce ini file name from options");
                std::ifstream settingsFile(w+i, std::ifstream::in);
                if(settingsFile.good()) {
                    po::store( po::parse_config_file( settingsFile , desc ), vm );
                    settingsFile.close();
                }
                else
                    report.raiseError("Cant read ini file", w+i);
            }
            po::notify( vm );
        }
        catch (std::exception &e) {
            report.raiseError("sbfCmdOptions::parceCmd:", e.what());
            throw e;
        }

        if(workDir.size() == 0)
            workDir = "./";
        else if( workDir.back() != '/')
            workDir += "/";
    }

//    //template<class ...B, typename std::enable_if<sizeof... (B) == 0>::type>
//    //typename std::enable_if<sizeof... (B) == 0>::type
//    bool haveInOptoions(/*B ...b*/){ return true;}

//    template<class A, class ...B>
//    bool haveInOptoions(A a, B ...b){
//        return (vm.count(a) > 0 ? true : false) || haveInOptoions(&b...);
//    }
    bool haveInOptions(const std::string &option) {
        return vm.count(option);
    }

    template<typename T>
    T getValue(const std::string & option, bool *ok) {
        if(vm.count(option)) {
            if(ok) *ok = true;
            return vm[option].as<T>();
        }
        if(ok) *ok = false;
        return T();
    }

    std::string getIndName() const;

    std::string getCrdName() const;

    std::string getMtrName() const;

    std::string getPrefix() const;

    std::string getWorkDir() const;

    std::string getIndPath() const;

    std::string getCrdPath() const;

    std::string getMtrPath() const;

private:
    friend std::ostream & operator<<(std::ostream &o, const sbfCmdOptions &ops) {
        o << ops.desc;
        return o;
    }

private:
    po::options_description desc;
    po::variables_map vm;
    std::string workDir;
    std::string indName, crdName, mtrName, namePrefix;
};

#endif // COMMAND_OPTIONS_HPP

std::string sbfCmdOptions::getIndName() const
{
return namePrefix + indName;
}

std::string sbfCmdOptions::getCrdName() const
{
return namePrefix + crdName;
}

std::string sbfCmdOptions::getMtrName() const
{
    return namePrefix + mtrName;
}

std::string sbfCmdOptions::getPrefix() const
{
    return namePrefix;
}

std::string sbfCmdOptions::getWorkDir() const
{
    return workDir;
}

std::string sbfCmdOptions::getIndPath() const
{
    return workDir + namePrefix + indName;
}

std::string sbfCmdOptions::getCrdPath() const
{
    return workDir + namePrefix + crdName;
}

std::string sbfCmdOptions::getMtrPath() const
{
    return workDir + namePrefix + mtrName;
}
