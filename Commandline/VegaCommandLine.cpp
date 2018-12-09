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
#include "../Abstract/Utility.h"
#include "build_properties.h"
#include "../Nastran/NastranParser.h"
#include "../Optistruct/OptistructParser.h"
#include "../Nastran/NastranWriter.h"
#include "../Nastran/NastranRunner.h"

#if ENABLE_ASTER
#include "../Aster/AsterWriter.h"
#include "../Aster/AsterRunner.h"
#endif
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
using boost::to_upper;
using namespace std;


// A helper function to simplify the main part.
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v) {
    copy(v.begin(), v.end(), ostream_iterator<T>(cout, " "));
    return os;
}

namespace vega {

unordered_map<VegaCommandLine::ExitCode, string, EnumClassHash> VegaCommandLine::failureReason_by_ExitCode = {
        {ExitCode::OK, "OK"},
        {ExitCode::GENERIC_EXCEPTION, "An unexpected exception was thrown during translation."},
		{ExitCode::PARSING_EXCEPTION, "While reading the input file, VEGA encountered an error and quit."},
		{ExitCode::WRITING_EXCEPTION, "While writing the output file, VEGA encountered an error and quit."},
        {ExitCode::NO_INPUT_FILE, "No input file was specified."},
        {ExitCode::OUTPUT_DIR_NOT_CREATED, "Output dir can't be created."},
        {ExitCode::INVALID_COMMAND_LINE, "Invalid command line argument."},
        {ExitCode::MODEL_VALIDATION_ERROR, "Validation of model failed."},
        {ExitCode::SOLVER_NOT_FOUND, "Problem launching the solver (solver not found?)."},
        {ExitCode::SOLVER_KILLED, "Solver got a signal (killed?)."},
        {ExitCode::SOLVER_EXIT_NOT_ZERO, "Generic Solver problem {exit code !=0}."},
        {ExitCode::SOLVER_RESULT_NOT_FOUND, "Solver did not produce the expected result file."},
        {ExitCode::SOLVER_SYNTAX_ERROR, "Solver found a syntax error in vega++ translated file."},
        {ExitCode::SOLVER_TEST_FAIL, "internal solver test fail (TEST_RESU)."}
};

VegaCommandLine::VegaCommandLine() {
    parserBySolverName[SolverName::NASTRAN] = make_shared<nastran::NastranParser>();
    parserBySolverName[SolverName::OPTISTRUCT] = make_shared<optistruct::OptistructParser>();
#if ENABLE_ASTER
    writersBySolverName[SolverName::CODE_ASTER] = make_shared<aster::AsterWriter>();
    runnerBySolverType[SolverName::CODE_ASTER] = make_shared<aster::AsterRunner>();
#endif
    writersBySolverName[SolverName::NASTRAN] = make_shared<nastran::NastranWriter>();
    writersBySolverName[SolverName::SYSTUS] = make_shared<systus::SystusWriter>();
    runnerBySolverType[SolverName::NASTRAN] = make_shared<nastran::NastranRunner>();
    runnerBySolverType[SolverName::SYSTUS] = make_shared<systus::SystusRunner>();
}

VegaCommandLine::ExitCode VegaCommandLine::convertStudy(
        const ConfigurationParameters& configuration, string& modelFileOut,
        const Solver& inputSolver) {

    vega::SolverName outputSolver = configuration.outputSolver.getSolverName();
    const auto& writerIterator = writersBySolverName.find(outputSolver);
    if (writerIterator == writersBySolverName.end()) {
        cerr << "Output format " << configuration.outputSolver << "not supported." << endl;
        return ExitCode::INVALID_COMMAND_LINE;
    }
    shared_ptr<Writer> writer = writerIterator->second;

    const auto& parserIterator = parserBySolverName.find(inputSolver.getSolverName());
    if (parserIterator == parserBySolverName.end()) {
        cerr << "Input format " << inputSolver << "not supported." << endl;
        return ExitCode::INVALID_COMMAND_LINE;
    }

    if (configuration.logLevel >= LogLevel::TRACE) {
        cout << "Selected writer: " << *writer << endl;
    }

    // Parsing the input file
    shared_ptr<Parser> parser = parserIterator->second;
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
            && configuration.translationMode == ConfigurationParameters::TranslationMode::MODE_STRICT) {
        cerr << "Errors validating model. EXIT" << endl;
        return ExitCode::MODEL_VALIDATION_ERROR;
    }

    string modelFile = writer->writeModel(model, configuration);
    modelFileOut.append(modelFile);
    return ExitCode::OK;
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
    if (vm.count("verbosity")){
        string verbosity= vm["verbosity"].as<string>();
        boost::to_upper(verbosity);
        if (verbosity=="ERROR") logLevel = LogLevel::ERROR;
        if (verbosity=="WARN") logLevel = LogLevel::WARN;
        if (verbosity=="INFO") logLevel = LogLevel::INFO;
        if (verbosity=="DEBUG") logLevel = LogLevel::DEBUG;
        if (verbosity=="TRACE") logLevel = LogLevel::TRACE;
    }

    if (vm.count("debug") > 0) {
        cout << "Debug output enabled. " << endl;
        logLevel = max(logLevel,LogLevel::DEBUG);
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
    ConfigurationParameters::TranslationMode translationMode = ConfigurationParameters::TranslationMode::BEST_EFFORT;
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
        translationMode = ConfigurationParameters::TranslationMode::MESH_AT_LEAST;
    }
    if (vm.count("strict")) {
        if (hasParamBestEffort || hasParamMeshAtLeast) {
            throw invalid_argument("only one between mesh-at-least, best-effort or strict"
                    " parameter can be specified at once.");
        }
        translationMode = ConfigurationParameters::TranslationMode::MODE_STRICT;
    }


    // Choice of output solver, and options related to it
    #if ENABLE_ASTER
    Solver solver(SolverName::CODE_ASTER);
    #else
    Solver solver(SolverName::NASTRAN);
    #endif
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

    string solverVersion;
    if (vm.count("solver-version")) {
        solverVersion = vm["solver-version"].as<string>();
    } else {
        if (solver.getSolverName()== SolverName::SYSTUS){
            solverVersion = "2016";
        }else{
            solverVersion = "";
        }
    }

    // Option for Systus Conversion
    string systusRBE2TranslationMode="penalty";
    if (vm.count("systus.RBE2TranslationMode")){
        systusRBE2TranslationMode = vm["systus.RBE2TranslationMode"].as<string>();
        set<string> availableTranlation { "lagrangian", "penalty" };
        set<string>::iterator it = availableTranlation.find(systusRBE2TranslationMode);
        if (it == availableTranlation.end()) {
            throw invalid_argument("Systus RBE2 Translation Mode must be either lagrangian or penalty (default).");
        }
    }
    double systusRBE2Rigidity=Globals::UNAVAILABLE_DOUBLE;
    if ((vm.count("systus.RBE2Rigidity")) && (systusRBE2TranslationMode=="penalty")){
        systusRBE2Rigidity = vm["systus.RBE2Rigidity"].as<double>();
        if ((!is_equal(systusRBE2Rigidity,Globals::UNAVAILABLE_DOUBLE)) && (systusRBE2Rigidity<=0.0)){
            throw invalid_argument("Systus RBE2 Rigidity must be positive.");
        }
    }
    double systusRBELagrangian=vm["systus.RBELagrangian"].as<double>();


    string systusOptionAnalysis="auto";
    if (vm.count("systus.OptionAnalysis")){
    	systusOptionAnalysis = vm["systus.OptionAnalysis"].as<string>();
    	set<string> availableTranlation { "auto", "3D", "shell", "shell-multi" };
    	set<string>::iterator it = availableTranlation.find(systusOptionAnalysis);
    	if (it == availableTranlation.end()) {
    		throw invalid_argument("Systus OPTION analysis must be either auto (default), 3D, shell or shell-multi");
    	}
    }
    string systusOutputProduct="systus";
    if (vm.count("systus.OutputProduct")){
        systusOutputProduct = vm["systus.OutputProduct"].as<string>();
        set<string> availableTranlation { "systus", "topaze" };
        set<string>::iterator it = availableTranlation.find(systusOutputProduct);
        if (it == availableTranlation.end()) {
            throw invalid_argument("Systus output product must be either systus (default) or topaze");
        }
    }

    string systusDynamicMethod="direct";
    if (vm.count("systus.DynamicMethod")){
        systusDynamicMethod = vm["systus.DynamicMethod"].as<string>();
        set<string> availableTranlation { "direct", "modal" };
        set<string>::iterator it = availableTranlation.find(systusDynamicMethod);
        if (it == availableTranlation.end()) {
            throw invalid_argument("Systus dynamic method must be either direct (default) or modal");
        }
    }


    string systusOutputMatrix="table";
    int systusSizeMatrix=9;
    if (vm.count("systus.OutputMatrix")){
        systusOutputMatrix = vm["systus.OutputMatrix"].as<string>();
        set<string> availableTranlation { "table", "file" };
        set<string>::iterator it = availableTranlation.find(systusOutputMatrix);
        if (it == availableTranlation.end()) {
            throw invalid_argument("Systus output matrix must be either 'table' (default) or 'file'");
        }
        if (systusOutputMatrix=="file"){
            systusSizeMatrix=20;
        }
    }
    if (vm.count("systus.SizeMatrix")){
        systusSizeMatrix = vm["systus.SizeMatrix"].as<int>();
        if (systusSizeMatrix < 2) {
            throw invalid_argument("Systus Size of Matrix must be greater than 1.");
        }
    }



    // Default value is "auto", characterized by a void systusSubcases
    vector< vector<int> > systusSubcases;
    if (vm.count("systus.Subcase")){
        vector<string> subcases = vm["systus.Subcase"].as< vector<string>>();
        std::string delimiter=",";
        std::string allowedchars="0123456789"+delimiter;
        for (size_t i = 0; i < subcases.size(); ++i){
            vector<int> vec;
            string soptions = subcases[i];
            // Single: every analysis in a separate file, is characterized by
            // systusSubcases = < <-1> >
            // TODO: Not very elegant. Do better.
            if (soptions=="single"){
                systusSubcases.clear();
                systusSubcases.push_back({-1});
                break;
            }
            if (soptions=="auto"){
                systusSubcases.clear();
                break;
            }

            if (strspn(soptions.c_str(), allowedchars.c_str()) != soptions.size() ){
                throw invalid_argument("Systus Subcase list must verify the syntax 'n1,n2,n3'.");
            }
            size_t pos = 0;
            while ((pos = soptions.find(delimiter)) != string::npos) {
                string token = soptions.substr(0, pos);
                soptions.erase(0, pos + delimiter.length());
                vec.push_back(  atoi(token.c_str()) );
            }
            vec.push_back(atoi(soptions.c_str()));
            systusSubcases.push_back(vec);
        }
    }


    if (vm.count("listOptions")){
        cout << "VEGA options for this translation are: "<< endl;
        cout << "\t Output directory: "<< outputDir << endl;
        cout << "\t Verbosity: "<< static_cast<int>(logLevel) << endl;
        cout << "\t Systus RBE2 Translation Mode: "<< systusRBE2TranslationMode << endl;
        cout << "\t Systus RBE2 Rigidity (for penalty mode only): " << (is_equal(systusRBE2Rigidity, Globals::UNAVAILABLE_DOUBLE) ? "auto" : to_string(systusRBE2Rigidity)) << endl;
        cout << "\t Systus RBE Lagrangian (for RBE2 lagrangian mode and RBE3): " << systusRBELagrangian << endl;
        cout << "\t Systus OPTION analysis: " << systusOptionAnalysis << endl;
        cout << "\t Systus Dynamic method: " << systusDynamicMethod << endl;
        cout << "\t Systus Output product: " << systusOutputProduct << endl;
        cout << "\t Systus Output Matrix: " << systusOutputMatrix << endl;
        cout << "\t Systus Size Matrix: " << systusSizeMatrix << endl;
        cout << "\t Systus Version: " << solverVersion << endl;
        for (size_t i = 0; i < systusSubcases.size(); ++i) {
           cout <<"\t Systus Subcase "<<(i+1)<<": ";
           for (size_t j = 0; j < systusSubcases[i].size(); ++j)
              cout << systusSubcases[i][j] << ' ';
           cout << endl;
        }
    }

    ConfigurationParameters configuration = ConfigurationParameters(inputFile.string(), solver,
            solverVersion, modelName, outputDir, logLevel, translationMode, testFnamePath,
            tolerance, runSolver, solverServer, solverCommand,
            systusRBE2TranslationMode, systusRBE2Rigidity, systusRBELagrangian, systusOptionAnalysis, systusOutputProduct,
            systusSubcases, systusOutputMatrix, systusSizeMatrix, systusDynamicMethod);
    return configuration;
}

VegaCommandLine::ExitCode VegaCommandLine::runSolver(const ConfigurationParameters& configuration,
        string modelFile) {
    ExitCode result;
    const auto& runnerIterator = runnerBySolverType.find(configuration.outputSolver.getSolverName());
    if (runnerIterator == runnerBySolverType.end()) {
        cerr << "Output runner " << configuration.outputSolver << "not supported." << endl;
        result = ExitCode::INVALID_COMMAND_LINE;
    }
    shared_ptr<Runner> runner = runnerIterator->second;
    //Little hack: return codes are the same.
    result = static_cast<VegaCommandLine::ExitCode>(runner->execSolver(configuration, modelFile));
    return result;
}

void VegaCommandLine::printHelp(const po::options_description& visible) {
    cout << endl << "vegapp [options] inputFile input-format output-format" << endl;
    cout << visible << endl;
}

void VegaCommandLine::printHeader() {
    cout << "Vega, version " << VEGA_VERSION_MAJOR << "." << VEGA_VERSION_MINOR << "."
                        << VEGA_VERSION_PATCH << " " << VEGA_VERSION_EXTRA << endl;
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
    ExitCode result = ExitCode::OK;
#if Backtrace_FOUND
    signal(SIGSEGV, handler);
#endif
    LogLevel logLevel = LogLevel::TRACE;
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
        ("listOptions,l", "Print the options used by current translation.") //
        ("mesh-at-least,m", "If the source study is fully understood it is translated, "
                " otherwise it is translated only the mesh.") //
        ("strict,s", "Stops translation at the first "
                "unrecognized keyword or parameter.")//
        ("verbosity", po::value<string>(), "Verbosity of VEGA. From low to high: ERROR, WARN, INFO, DEBUG, TRACE"); //

        // Systus specific options
        // TODO: Some of these options are not so specific: rename and move them.
        po::options_description systusOptions("Systus specific options");
        systusOptions.add_options() //
        ("systus.DynamicMethod",po::value<string>()->default_value("direct"),
                 "Dynamic method for harmonic analysis: direct (default) or modal.") //
        ("systus.RBE2TranslationMode",po::value<string>()->default_value("penalty"),
                "Translation mode of RBE2 from Nastran To Systus: lagrangian or penalty.") //
        ("systus.RBE2Rigidity", po::value<double>(),
                "Rigidity of RBE2 (if you don't want the automatic penalty translation only).") //
        ("systus.RBELagrangian", po::value<double>()->default_value(1.0),
                "Lagrange coefficients for RBE2 (Lagrange Translation) and RBE3.") //
        ("systus.Subcase", po::value<std::vector<std::string>>(),
                "'auto' (default), 'single' or lists 'n1,n2,...,nN' of analysis numbers belonging to the same subcase.") //
        ("systus.OptionAnalysis",po::value<string>()->default_value("auto"),
                "Type of analysis used by the Systus writer (Systus OPTION command): auto (default), 3D, shell or shell-multi.") //
         ("systus.OutputProduct",po::value<string>()->default_value("systus"),
                "Output format for the Systus writer: systus (default) or topaze.") //
        ("systus.OutputMatrix",po::value<string>()->default_value("table"),
                "Output of Matrix Elements (e.g Super Elements) to 'table' (default) or 'file'") //
        ("systus.SizeMatrix", po::value<int>(),
                "Maximum size of Systus Matrix Elements: default 9 for table, 20 for file."); //


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
        cmdline_options.add(commandLine).add(generic).add(systusOptions).add(hidden);

        po::options_description config_file_options;
        config_file_options.add(generic).add(systusOptions).add(hidden);

        po::positional_options_description p;
        p.add("input-file", 1);
        p.add("input-format", 1);
        p.add("output-format", 1);

        po::options_description visible("Options");
        visible.add(commandLine).add(generic).add(systusOptions);

        po::variables_map vm;
        store(po::command_line_parser(ac, av).options(cmdline_options).positional(p).run(), vm);

        notify(vm);

        // By default, print a header with version number
        printHeader();

        // If we asked for version, nothing to do anymore.
        if (vm.count("version")) {
            //printHeader();
            return ExitCode::OK;
        }

        // Print help message and quit.
        if (vm.count("help")) {
            printHelp(visible);
            return ExitCode::OK;
        }

        // Read the Config File.
        config_file = expand_user(config_file);
        ifstream ifs(config_file.c_str());
        if (!ifs) {
            //TODO: Global Handler for error.
            // We only make a warning if the user provided a config file
            if (!vm["config"].defaulted()){
               cerr << "Warning: cannot open configuration file " << config_file << ". Default behavior used." << endl;
            }
        } else {
            store(parse_config_file(ifs, config_file_options), vm);
            notify(vm);
        }



        if (vm.count("input-file") == 0) {
            cout << "No input files specified." << endl << endl;
            printHelp(visible);
            return ExitCode::NO_INPUT_FILE;
        }


        const ConfigurationParameters configuration = readCommandLineParameters(vm);
        logLevel = configuration.logLevel;
        if (configuration.resultFile.string().size() >= 1) {
            if (!fs::exists(configuration.resultFile)) {
                cerr << "Test file specified " << configuration.resultFile << " can't be found. " << endl;
                return ExitCode::NO_INPUT_FILE;
            }
        }
        if (!fs::exists(configuration.inputFile)) {
            cout << "Input file" << configuration.inputFile << " not found." << endl << endl;
            return ExitCode::NO_INPUT_FILE;
        }

        if (!fs::exists(configuration.outputPath)) {
            bool create = fs::create_directories(configuration.outputPath);
            if (!create) {
                cerr << "Output Directory " + configuration.outputPath + " can't be created."
                        << endl;
                return ExitCode::OUTPUT_DIR_NOT_CREATED;
            }
        }

        Solver inputFormat(SolverName::NASTRAN);
        if (vm.count("input-format")) {
            string inputSolverString = vm["input-format"].as<string>();
            inputFormat = Solver::fromString(inputSolverString);
        }
        string modelFile;
        result = convertStudy(configuration, modelFile, inputFormat);
        if (result == ExitCode::OK && configuration.runSolver) {
            result = runSolver(configuration, modelFile);
        }
    } catch (invalid_argument &e) {
        if (logLevel >= LogLevel::DEBUG) {
            throw;
        } else
            cerr  << endl << "Invalid argument: " << e.what()  << endl;
        return ExitCode::INVALID_COMMAND_LINE;
    } catch (ParsingException & e) {   // A parsing error occurred.
        if (logLevel >= LogLevel::DEBUG) {
            throw;
        } else
            cerr << endl << e.what() << endl;
    	return ExitCode::PARSING_EXCEPTION;
    } catch (WritingException & e) {   // An error occurred in the Writer.
        if (logLevel >= LogLevel::DEBUG) {
            throw;
        } else
            cerr << endl << e.what() << endl;
    	return ExitCode::WRITING_EXCEPTION;
    } catch (logic_error& e) {
        if (logLevel >= LogLevel::DEBUG) {
            throw;
        } else
            cerr << endl << "Logic error: " << e.what() << endl;
        return ExitCode::GENERIC_EXCEPTION;
    } catch (exception& e) {
        if (logLevel >= LogLevel::DEBUG) {
            throw;
        } else
            cerr << endl << "Exception: " << e.what() << endl;
        return ExitCode::GENERIC_EXCEPTION;
    }
    return result;
}

const string VegaCommandLine::exitCodeToString(ExitCode exitCode)
 {
    auto it = failureReason_by_ExitCode.find(exitCode);
    if (it == failureReason_by_ExitCode.end()) {
        return "UNKNOWN FAILURE";
    }
    return it->second.c_str();
}

} /* namespace vega */
