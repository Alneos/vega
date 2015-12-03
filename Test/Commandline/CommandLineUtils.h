/*
 * CommandLineUtils.h
 *
 *  Created on: Nov 25, 2014
 *      Author: devel
 */

#ifndef COMMANDLINEUTILS_H_
#define COMMANDLINEUTILS_H_

#include "../../Abstract/ConfigurationParameters.h"
namespace vega {
namespace tests {

using std::string;

class CommandLineUtils {
private:
    CommandLineUtils();
    static bool containsWord(const string &fname, const string &word);
    static std::vector<bool> containWords(const string &fname,
            const std::vector<string> &words);
    public:
    static void run(string inputFname, SolverName inputSolver, SolverName outputSolver,
            bool runSolver = false, bool strict = false, double tolerance = 0.02);
    static void nastranStudy2Aster(string fname, bool runAster = false, bool strict = true,
            double tolerance = 0.02);
    static void nastranStudy2Systus(string fname, bool runSystus = false, bool strict = true,
            double tolerance = 0.02);
    static void nastranStudy2Nastran(string fname, bool runAster = false, bool strict = true,
            double tolerance = 0.02);
    virtual ~CommandLineUtils();
};
}
} /* namespace vega */

#endif /* COMMANDLINEUTILS_H_ */
