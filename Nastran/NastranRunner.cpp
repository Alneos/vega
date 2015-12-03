/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
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
