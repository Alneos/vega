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

class VegaCommandLine {
public:
    /**
     * This enumeration contains all the possible exit codes.
     *
     * Remember to update failureReason_by_ExitCode if you add an entry here
     */
    enum ExitCode {
        OK = 0, //
        GENERIC_EXCEPTION = 1, //An unexpected exception was thrown
        NO_INPUT_FILE = 2, //No input file was specified
        OUTPUT_DIR_NOT_CREATED = 3, //Output dir can't be created.
        INVALID_COMMAND_LINE = 4, //Invalid command line argument
        MODEL_VALIDATION_ERROR = 5, //Validation of model failed.
		PARSING_EXCEPTION = 6, // Error in the Parser.
		WRITING_EXCEPTION = 7, // Error in the Writer.
        SOLVER_NOT_FOUND = Runner::SOLVER_NOT_FOUND, //solver was not found
        SOLVER_KILLED = Runner::SOLVER_KILLED,
        SOLVER_EXIT_NOT_ZERO = Runner::SOLVER_EXIT_NOT_ZERO,
        SOLVER_RESULT_NOT_FOUND = Runner::SOLVER_RESULT_NOT_FOUND,
        SOLVER_SYNTAX_ERROR = Runner::TRANSLATION_SYNTAX_ERROR,
        SOLVER_TEST_FAIL = Runner::TEST_FAIL //internal solver test fail (TEST_RESU)
    };

private:
    static std::unordered_map<ExitCode, std::string, std::hash<int>> failureReason_by_ExitCode;

    ConfigurationParameters readCommandLineParameters(const po::variables_map& vm);
    ExitCode convertStudy(const ConfigurationParameters& configuration, std::string& modelFileOut,
            const Solver& inputSolver);
    ExitCode runSolver(const ConfigurationParameters& configuration, std::string modelFile);
    static void printHelp(const po::options_description& visible);
    static void printHeader();
    std::string expand_user(std::string path);
    static fs::path normalize_path(std::string path);
    std::unordered_map<SolverName, Parser *, std::hash<int>> parserBySolverName;
    std::unordered_map<SolverName, Writer *, std::hash<int>> writersBySolverName;
    std::unordered_map<SolverName, Runner *, std::hash<int>> runnerBySolverType;
public:
    VegaCommandLine();
    ExitCode process(int ac, const char* av[]);
    virtual ~VegaCommandLine();
    static const char * exitCodeToString(ExitCode);
};

} /* namespace vega */
#endif /* VEGACOMMANDLINE_H_ */
