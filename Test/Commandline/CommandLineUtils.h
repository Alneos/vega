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

class CommandLineUtils {
private:
    static bool containsWord(const std::string &fname, const std::string &word);
    static std::vector<bool> containWords(const std::string &fname,
            const std::vector<std::string> &words);
    public:
    static void run(std::string inputFname, SolverName inputSolver, SolverName outputSolver,
            bool runSolver = false, bool strict = false, double tolerance = 0.02);
    static void nastranStudy2Aster(std::string fname, bool runAster = false, bool strict = true,
            double tolerance = 0.02);
    static void nastranStudy2Systus(std::string fname, bool runSystus = false, bool strict = true,
            double tolerance = 0.02);
    static void nastranStudy2Nastran(std::string fname, bool runAster = false, bool strict = true,
            double tolerance = 0.02);
    static void optistructStudy2Aster(std::string fname, bool runAster = false, bool strict = true,
            double tolerance = 0.02);
};
} /* namespace tests */
} /* namespace vega */

#endif /* COMMANDLINEUTILS_H_ */
