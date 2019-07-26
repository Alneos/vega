/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * This file is part of Vega.
 *
 *   Vega is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Vega is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Vega.  If not, see <http://www.gnu.org/licenses/>.
 *
 * AsterRunner.cpp
 *
 *  Created on: Nov 8, 2013
 *      Author: devel
 */

#include "AsterRunner.h"

#include "../Abstract/ConfigurationParameters.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <sstream>

namespace vega {
namespace aster {

namespace fs = boost::filesystem;
using namespace std;

Runner::ExitCode AsterRunner::execSolver(const ConfigurationParameters &configuration,
        string modelFile) {
    bool local = configuration.solverServer.empty() || configuration.solverServer == "localhost"
            || configuration.solverServer == "127.0.0.1";
    string command = configuration.solverCommand.empty() ? "as_run" : configuration.solverCommand;
    fs::path modelFilePath(modelFile);
    string fname = modelFilePath.stem().string();
    command = command + " " + modelFile + " > " + fname + ".stdout 2> " + fname + ".stderr";

    if (configuration.outputPath != ".") {
        command = "cd " + configuration.outputPath + " && " + command;
    }
    vector<string> fileList = { ".mess", ".resu", ".rmed", ".stdout", ".stderr" };
    deletePreviousResultFiles(modelFile, fileList);
    fs::path repeout = modelFilePath.remove_filename();
    //repeout /= "repe_out";
    repeout /= fname + "_repe_out";
    if (fs::exists(repeout)) {
        uintmax_t removed = fs::remove_all(repeout);
        if (removed == 0) {
            cerr << "Repe Output Directory " + repeout.string() + " can't be removed."
                    << endl;
        }
    }
    bool created = fs::create_directories(repeout);
    if (!created) {
        cerr << "Repe Output Directory " + repeout.string() + " can't be created."
                << endl;
    }
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "About to launch " << command << endl;
    }

    int i = 0;
    if (local) {
        i = system(command.c_str());
    } else {
        throw logic_error("Remote server not implemented");
    }
    if (i != 0) {
        cerr << "Command " << command << " exit code:" << i << endl;
    } else if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Command " << command << " ended with exit code " << i << endl;
    }

    ExitCode exitCode = convertExecResult(i);

    if (exitCode == ExitCode::OK) {
        //check if resu file exist
        string resuFileStr = stripExtension(modelFile) + ".resu";
        ifstream resuFile(resuFileStr);
        if (!(resuFile.is_open() && resuFile.good())) {
            ostringstream str;
            str << "Error executing Code Aster: " << resuFileStr << " not found.";
            perror(str.str().c_str());
            exitCode = ExitCode::SOLVER_RESULT_NOT_FOUND;
        } else {
            //check if it contains nook
            string line;
            int lineNumber = 0;
            while (getline(resuFile, line)) {
                lineNumber += 1;
                if (line.find("NOOK") != string::npos) {
                    cerr << "Test fail: line " << lineNumber << " file: " << resuFileStr << endl;
                    exitCode = ExitCode::TEST_FAIL;
                }
            }
            if (exitCode == ExitCode::OK && configuration.logLevel >= LogLevel::DEBUG
                    && !configuration.resultFile.empty()) {
                cout << "Tests OK." << endl;
            }
        }
    } else if (i == 4) { //handle the case of code_aster exit code 4
        exitCode = ExitCode::TRANSLATION_SYNTAX_ERROR;
    }
    return exitCode;
}

}
} /* namespace vega */
