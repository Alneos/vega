/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * VegaCommandLine.h
 *
 *  Created on: Sep 1, 2013
 *      Author: devel
 */

#ifndef VEGACOMMANDLINE_H_
#define VEGACOMMANDLINE_H_

#include "../Abstract/ConfigurationParameters.h"
#include "../Abstract/SolverInterfaces.h"
#include <string>
#include <unordered_map>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace vega {
namespace po = boost::program_options;
namespace fs = boost::filesystem;

/**
  * Helper function to simplify the main part.
  */
template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " "));
    return os;
}

class VegaCommandLine {
public:
    /**
     * This enumeration contains all the possible exit codes.
     *
     * Remember to update failureReason_by_ExitCode if you add an entry here
     */
    enum class ExitCode {
        OK = 0, //
        GENERIC_EXCEPTION = 1, //An unexpected exception was thrown
        NO_INPUT_FILE = 2, //No input file was specified
        OUTPUT_DIR_NOT_CREATED = 3, //Output dir can't be created.
        INVALID_COMMAND_LINE = 4, //Invalid command line argument
        MODEL_VALIDATION_ERROR = 5, //Validation of model failed.
		PARSING_EXCEPTION = 6, // Error in the Parser.
		WRITING_EXCEPTION = 7, // Error in the Writer.
		CHILD_CRASHED = 8, // Child not providing status
		FORK_FAILED = 9, // Fork in main could not be done by the system
        SOLVER_NOT_FOUND = static_cast<int>(Runner::ExitCode::SOLVER_NOT_FOUND), //solver was not found
        SOLVER_KILLED = static_cast<int>(Runner::ExitCode::SOLVER_KILLED),
        SOLVER_EXIT_NOT_ZERO = static_cast<int>(Runner::ExitCode::SOLVER_EXIT_NOT_ZERO),
        SOLVER_RESULT_NOT_FOUND = static_cast<int>(Runner::ExitCode::SOLVER_RESULT_NOT_FOUND),
        SOLVER_SYNTAX_ERROR = static_cast<int>(Runner::ExitCode::TRANSLATION_SYNTAX_ERROR),
        SOLVER_TEST_FAIL = static_cast<int>(Runner::ExitCode::TEST_FAIL) //internal solver test fail (TEST_RESU)
    };

private:
    static std::unordered_map<ExitCode, std::string, EnumClassHash> failureReason_by_ExitCode;

    ConfigurationParameters readCommandLineParameters(const po::variables_map& vm);
    ExitCode convertStudy(const ConfigurationParameters& configuration, std::string& modelFileOut,
            const Solver& inputSolver);
    ExitCode runSolver(const ConfigurationParameters& configuration, std::string modelFile);
    static void printHelp(const po::options_description& visible);
    static void printHeader();
    std::string expand_user(std::string path);
    static fs::path normalize_path(std::string path);
    std::unordered_map<SolverName, std::unique_ptr<Parser>, EnumClassHash> parserBySolverName;
    std::unordered_map<SolverName, std::unique_ptr<Writer>, EnumClassHash> writersBySolverName;
    std::unordered_map<SolverName, std::unique_ptr<Runner>, EnumClassHash> runnerBySolverType;
public:
    VegaCommandLine();
    VegaCommandLine(const VegaCommandLine& that) = delete;
    ExitCode process(int ac, const char* av[]);
    static const std::string exitCodeToString(ExitCode);
};

} /* namespace vega */
#endif /* VEGACOMMANDLINE_H_ */
