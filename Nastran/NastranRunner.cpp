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
 * NastranRunner.cpp
 *
 *  Created on: Nov 8, 2013
 *      Author: devel
 */

#include "NastranRunner.h"
#include "../Abstract/ConfigurationParameters.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <sstream>

namespace vega {
namespace nastran {

namespace fs = boost::filesystem;
using namespace std;

NastranRunnerImpl::NastranRunnerImpl() {

}

Runner::ExitCode NastranRunnerImpl::execSolver(const ConfigurationParameters &configuration,
        string modelFile) {
    fs::path modelFilePath(modelFile);
    string fname = modelFilePath.stem().string();
    string command = "nastran " + modelFile + " > " + fname + ".stdout 2> " + fname
            + ".stderr";
    fs::path outputFsPath(configuration.outputPath);
    cout << configuration.outputPath << endl;
    if (!fs::is_directory(outputFsPath))
        return SOLVER_NOT_FOUND;
    if (configuration.outputPath != ".")
        command = "cd " + configuration.outputPath + " && " + command;

    vector<string> fileList = { ".DBALL", ".f04", ".f06", ".IFPDAT", ".log", ".MASTER", ".stdout",
            ".stderr" };
    deletePreviousResultFiles(modelFile, fileList);

    //run command
    int i = system(command.c_str());
    if (i != 0) {
        cerr << "Command " << command << " exit code:" << i << endl;
    } else if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Command " << command << " ended with exit code " << i << endl;
    }

    ExitCode exitCode = convertExecResult(i);

    return exitCode;
}

NastranRunnerImpl::~NastranRunnerImpl() {

}

} /* namespace nastran */
} /* namespace vega */
