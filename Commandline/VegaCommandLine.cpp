/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * VegaCommandLine.cpp
 *
 *  Created on: Sep 1, 2013
 *      Author: devel
 */

#include "VegaCommandLine.h"
#include "build_properties.h"
#include "../Nastran/NastranFacade.h"
#include "../Aster/AsterFacade.h"
#include "../Systus/SystusWriter.h"
#include "../Systus/SystusRunner.h"
#include "../ResultReaders/ResultReadersFacade.h"
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <iterator>
#include <ciso646>

namespace fs = boost::filesystem;
namespace po = boost::program_options;
using namespace std;

#ifdef __linux__
#include <execinfo.h>
#include <signal.h>

void handler(int sig) {
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, static_cast<int>(size), STDERR_FILENO);
    exit(1);
}

#endif

// A helper function to simplify the main part.
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v) {
    copy(v.begin(), v.end(), ostream_iterator<T>(cout, " "));
    return os;
}

namespace vega {

unordered_map<VegaCommandLine::ExitCode, string, hash<int>> VegaCommandLine::failureReason_by_ExitCode = {
        {OK, "OK"},
        {GENERIC_EXCEPTION, "An unexpected exception was thrown during translation."},
        {NO_INPUT_FILE, "No input file was specified."},
        {OUTPUT_DIR_NOT_CREATED, "Output dir can't be created."},
        {INVALID_COMMAND_LINE, "Invalid command line argument."},
        {MODEL_VALIDATION_ERROR, "Validation of model failed."},
        {SOLVER_NOT_FOUND, "Problem launching the solver (solver not found?)."},
        {SOLVER_KILLED, "Solver got a signal (killed?)."},
        {SOLVER_EXIT_NOT_ZERO, "Generic Solver problem {exit code !=0}."},
        {SOLVER_RESULT_NOT_FOUND, "Solver did not produce the expected result file."},
        {SOLVER_SYNTAX_ERROR, "Solver found a syntax error in vega++ translated file."},
        {SOLVER_TEST_FAIL, "internal solver test fail (TEST_RESU)."}
};

VegaCommandLine::VegaCommandLine() {
    parserBySolverName[NASTRAN] = new nastran::NastranParser();
    writersBySolverName[CODE_ASTER] = new aster::AsterWriter();
    writersBySolverName[NASTRAN] = new nastran::NastranWriter();
    writersBySolverName[SYSTUS] = new SystusWriter();
    runnerBySolverType[CODE_ASTER] = new aster::AsterRunner();
    runnerBySolverType[NASTRAN] = new nastran::NastranRunner();
    runnerBySolverType[SYSTUS] = new SystusRunner();
}

VegaCommandLine::ExitCode VegaCommandLine::convertStudy(
        const ConfigurationParameters& configuration, string& modelFileOut,
        const Solver& inputSolver) {

    vega::SolverName outputSolver = configuration.outputSolver.getSolverName();
    unordered_map<SolverName, Writer *, hash<int>>::const_iterator writerIterator =
            writersBySolverName.find(outputSolver);
    if (writerIterator == writersBySolverName.end()) {
        cerr << "Output format " << configuration.outputSolver << "not supported." << endl;
        return INVALID_COMMAND_LINE;
    }
    Writer* writer = writerIterator->second;

    auto parserIterator = parserBySolverName.find(inputSolver.getSolverName());
    if (parserIterator == parserBySolverName.end()) {
        cerr << "Input format " << inputSolver << "not supported." << endl;
        return INVALID_COMMAND_LINE;
    }

    if (configuration.logLevel >= LogLevel::TRACE) {
        cout << "Selected writer: " << *writer << endl;
    }

    Parser* parser = parserIterator->second;
    shared_ptr<Model> model = parser->parse(configuration);

    //adding assertions if result file is set in the model
    shared_ptr<ResultReader> resultReader = result::ResultReadersFacade::getResultReader(
            configuration);
    if (resultReader) {
        resultReader->add_assertions(configuration, model);
    }

    model->finish();
    bool validationResult = model->validate();
    if (!validationResult
            && configuration.translationMode == ConfigurationParameters::MODE_STRICT) {
        cerr << "Errors validating model. EXIT" << endl;
        return MODEL_VALIDATION_ERROR;
    }

    string modelFile = writer->writeModel(model, configuration);
    modelFileOut.append(modelFile);
    return OK;
}

VegaCommandLine::~VegaCommandLine() {
    for (auto parserNamePair : parserBySolverName) {
        delete (parserNamePair.second);
    }
    for (auto writerNamePair : writersBySolverName) {
        delete (writerNamePair.second);
    }
    for (auto writerNamePair : runnerBySolverType) {
        delete (writerNamePair.second);
    }
}

fs::path VegaCommandLine::normalize_path(string strpath) {
    if (strpath.front() == '"' || strpath.front() == '\'') {
        strpath.erase(0, 1); // erase the first character
        strpath.erase(strpath.size() - 1); // erase the last character
    }
    return fs::path(strpath).make_preferred();
}

ConfigurationParameters VegaCommandLine::readCommandLineParameters(const po::variables_map& vm) {
    LogLevel logLevel = LogLevel::INFO;
    if (vm.count("debug") > 0) {
        cout << "Debug output enabled. " << endl;
        logLevel = LogLevel::DEBUG;
    }
    const string inputFileStr = (vm["input-file"].as<string>());
    fs::path inputFile = normalize_path(inputFileStr);

    const string modelName(inputFile.filename().string());
    string outputDir;
    if (vm.count("output-dir")) {
        outputDir = normalize_path(vm["output-dir"].as<string>()).string();
    } else {
        outputDir = ".";
    }

    string solverVersion;
    if (vm.count("solver-version")) {
        solverVersion = vm["solver-version"].as<string>();
    } else {
        solverVersion = "";
    }
    string testFileName;
    if (vm.count("test-file")) {
        testFileName = normalize_path(vm["test-file"].as<string>()).string();
    } else {
        testFileName = "";
    }
    fs::path testFnamePath(testFileName);

    double tolerance;
    if (vm.count("tolerance")) {
        tolerance = vm["tolerance"].as<double>();
    } else {
        tolerance = 0.02;
    }
    ConfigurationParameters::TranslationMode translationMode = ConfigurationParameters::BEST_EFFORT;
    bool hasParamBestEffort = false;
    if (vm.count("best-effort")) {
        hasParamBestEffort = true;
    }
    bool hasParamMeshAtLeast = false;
    if (vm.count("mesh-at-least")) {
        if (hasParamBestEffort) {
            throw invalid_argument("only one between mesh-at-least or best-effort "
                    " parameter can be specified at once.");
        }
        translationMode = ConfigurationParameters::MESH_AT_LEAST;
    }
    if (vm.count("strict")) {
        if (hasParamBestEffort || hasParamMeshAtLeast) {
            throw invalid_argument("only one between mesh-at-least, best-effort or strict"
                    " parameter can be specified at once.");
        }
        translationMode = ConfigurationParameters::MODE_STRICT;
    }
    if (vm.count("strict")) {
        if (hasParamBestEffort || hasParamMeshAtLeast) {
            throw invalid_argument("only one between mesh-at-least, best-effort or strict"
                    " parameter can be specified at once.");
        }
        translationMode = ConfigurationParameters::MODE_STRICT;
    }
    if (vm.count("strict")) {
        if (hasParamBestEffort || hasParamMeshAtLeast) {
            throw invalid_argument("only one between mesh-at-least, best-effort or strict"
                    " parameter can be specified at once.");
        }
        translationMode = ConfigurationParameters::MODE_STRICT;
    }
    Solver solver(CODE_ASTER);
    if (vm.count("output-format")) {
        string outputSolver = vm["output-format"].as<string>();
        solver = Solver::fromString(outputSolver);
    }

    bool runSolver = false;
    if (vm.count("run-solver")) {
        runSolver = true;
    }
    string solverCommand;
    if (vm.count("solver-command")) {
        solverCommand = vm["solver-command"].as<string>();
        boost::algorithm::trim(solverCommand);
    }
    string solverServer;
    if (vm.count("solver-server")) {
        solverServer = vm["solver-server"].as<string>();
        boost::algorithm::trim(solverServer);
    }

    // Option for Systus Conversion
    string systusRBE2TranslationMode="lagrangian";
    if (vm.count("systus.RBE2TranslationMode")){
    	systusRBE2TranslationMode = vm["systus.RBE2TranslationMode"].as<string>();
    	set<string> availableTranlation { "lagrangian", "penalty" };
    	set<string>::iterator it = availableTranlation.find(systusRBE2TranslationMode);
    	if (it == availableTranlation.end()) {
    		throw invalid_argument("Systus RBE2 Translation Mode must be either lagrangian (default) or penalty.");
    	}
    }
    double systusRBE2Penalty=10.0;
    if (vm.count("systus.RBE2PenaltyFactor")){
    	systusRBE2Penalty = vm["systus.RBE2PenaltyFactor"].as<double>();
    	if (systusRBE2Penalty<= 0.0){
    		throw invalid_argument("Systus RBE2 Penalty Factor must be positive.");
    	}
    }


    ConfigurationParameters configuration = ConfigurationParameters(inputFile.string(), solver,
            solverVersion, modelName, outputDir, logLevel, translationMode, testFnamePath,
            tolerance, runSolver, solverServer, solverCommand,
			systusRBE2TranslationMode,systusRBE2Penalty);
    return configuration;
}

VegaCommandLine::ExitCode VegaCommandLine::runSolver(const ConfigurationParameters& configuration,
        string modelFile) {
    ExitCode result;
    unordered_map<SolverName, Runner*, hash<int>>::const_iterator runnerIterator =
            runnerBySolverType.find(configuration.outputSolver.getSolverName());
    if (runnerIterator == runnerBySolverType.end()) {
        cerr << "Output runner " << configuration.outputSolver << "not supported." << endl;
        result = ExitCode::INVALID_COMMAND_LINE;
    }
    Runner* runner = runnerIterator->second;
    //Little hack: return codes are the same.
    result = (VegaCommandLine::ExitCode) runner->execSolver(configuration, modelFile);
    return result;
}

void VegaCommandLine::printHelp(const po::options_description& visible) {
    cout << endl << "vegapp [options] inputFile input-format output-format" << endl;
    cout << visible << "\n";
}

string VegaCommandLine::expand_user(string path) {
    // http://stackoverflow.com/a/4891126
    if (not path.empty() and path[0] == '~') {
        assert(path.size() == 1 or path[1] == '/');  // or other error handling
        char const* home = getenv("HOME");
        if (home || (home = getenv("USERPROFILE"))) {
            path.replace(0, 1, home);
        } else {
            char const *hdrive = getenv("HOMEDRIVE"), *hpath = getenv("HOMEPATH");
            assert(hdrive);  // or other error handling
            assert(hpath);
            path.replace(0, 1, string(hdrive) + hpath);
        }
    }
    return path;
}

VegaCommandLine::ExitCode VegaCommandLine::process(int ac, const char* av[]) {
    ExitCode result = VegaCommandLine::OK;
#ifdef __linux__
    signal(SIGSEGV, handler);
#endif
    try {
        string config_file;

        // Declare a group of options that will be
        // allowed only on command line
        po::options_description commandLine("Command line options");
        commandLine.add_options() //
        ("version,v", "print version string and exit.") //
        ("help,h", "produce help message and exit.") //
        ("config,c", po::value<string>(&config_file)->default_value("~/vegamain.cfg"),
                "name of a configuration file.") //
        ("output-dir,o", po::value<string>()->default_value("."),
                "Output directory where results will be stored. If not "
                        "specified files will be put in the current directory.") //
        ("run-solver,R", "run solver after successful translation") //
        ("test-file,t", po::value<string>(), "add tests found in TESTFILE");

        // Declare a group of options that will be
        // allowed both on command line and in
        // config file
        po::options_description generic("Generic options");
        generic.add_options() //
        ("solver-command", po::value<string>(),
                "Solver command. Default 'as_run' if outputSolver is CODEASTER") //
        ("solver-server", po::value<string>()->default_value("localhost"),
                "Solver server for remote solver execution.") //
        ("debug,d", "set debug options in solvers, verbose output") //
        ("solver-version", po::value<string>(), "output solver specific version") //
        ("tolerance,x", po::value<double>(), "use TOLERANCE during tests.") //
        ("best-effort,b", "All the recognized keywords in the source file are "
                "translated, unknown keywords are skipped.") //
        ("mesh-at-least,m", "If the source study is fully understood it is translated, "
                " otherwise it is translated only the mesh.") //
		("systus.RBE2TranslationMode",po::value<string>()->default_value("lagrangian"), 
		        "Translation mode of RBE2 from Nastran To Systus: lagrangian or penalty.") //
	    ("systus.RBE2PenaltyFactor", po::value<double>()->default_value(10.0),
	            "Penalty RBE2 will have a rigidity of max rigidity*this value.") //
		("strict,s", "Stops translation at the first "
                "unrecognized keyword or parameter.");

        // Hidden options, will be allowed both on command line and
        // in config file, but will not be shown to the user.
        po::options_description hidden("Hidden options");
        hidden.add_options() //
        ("input-file", po::value<string>(), "input file") //
        ("input-format", po::value<string>()->default_value("NASTRAN"),
                "input format. Allowed formats are NASTRAN, ")("output-format",
                po::value<string>()->default_value("ASTER"),
                "output format. Allowed formats are ASTER, SYSTUS");

        po::options_description cmdline_options;
        cmdline_options.add(commandLine).add(generic).add(hidden);

        po::options_description config_file_options;
        config_file_options.add(generic).add(hidden);

        po::positional_options_description p;
        p.add("input-file", 1);
        p.add("input-format", 1);
        p.add("output-format", 1);

        po::options_description visible("Options");
        visible.add(commandLine).add(generic);

        po::variables_map vm;
        store(po::command_line_parser(ac, av).options(cmdline_options).positional(p).run(), vm);

        notify(vm);

        config_file = expand_user(config_file);
        ifstream ifs(config_file.c_str());
        if (!ifs) {
            cerr << "cannot open config file: " << config_file << "\n";
            // return 0;
        } else {
            store(parse_config_file(ifs, config_file_options), vm);
            notify(vm);
        }

        if (vm.count("help")) {
            printHelp(visible);
            return OK;
        }

        if (vm.count("version")) {
            cout << "Vega, version " << VEGA_VERSION_MAJOR << "." << VEGA_VERSION_MINOR << "."
                    << VEGA_VERSION_PATCH << " " << VEGA_VERSION_EXTRA << endl;
            return OK;
        }

        if (vm.count("input-file") == 0) {
            cout << "No input files specified." << endl << endl;
            printHelp(visible);
            return NO_INPUT_FILE;
        }



        const ConfigurationParameters configuration = readCommandLineParameters(vm);
        if (configuration.resultFile.string().size() >= 1) {
            if (!fs::exists(configuration.resultFile)) {
                cerr << "Test file specified " << configuration.resultFile << " can't be found. \n";
                return NO_INPUT_FILE;
            }
        }
        if (!fs::exists(configuration.inputFile)) {
            cout << "Input file" << configuration.inputFile << " not found." << endl << endl;
            return NO_INPUT_FILE;
        }

        if (!fs::exists(configuration.outputPath)) {
            bool create = fs::create_directories(configuration.outputPath);
            if (!create) {
                cerr << "Output Directory " + configuration.outputPath + " can't be created."
                        << endl;
                return OUTPUT_DIR_NOT_CREATED;
            }
        }
        Solver inputFormat(NASTRAN);
        if (vm.count("input-format")) {
            string inputSolverString = vm["input-format"].as<string>();
            inputFormat = Solver::fromString(inputSolverString);
        }
        string modelFile;
        result = convertStudy(configuration, modelFile, inputFormat);
        if (result == OK && configuration.runSolver) {
            result = runSolver(configuration, modelFile);
        }
    } catch (invalid_argument &e) {
        cerr << "\n Invalid argument:" << e.what() << "\n";
        return INVALID_COMMAND_LINE;
    } catch (logic_error& e) {
        cerr << "\n :" << e.what() << "\n";
        return GENERIC_EXCEPTION;
    } catch (exception& e) {
        cerr << "\n exception: " << e.what() << "\n";
        return GENERIC_EXCEPTION;
    }
    return result;
}

const char * VegaCommandLine::exitCodeToString(ExitCode exitCode) {
    auto it = failureReason_by_ExitCode.find(exitCode);
    if (it == failureReason_by_ExitCode.end()) {
        return "UNKNOWN FAILURE";
    }
    return it->second.c_str();
}

} /* namespace vega */
