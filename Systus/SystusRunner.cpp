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
 * SystusRunner.cpp
 *
 *  Created on: Nov 8, 2013
 *      Author: devel
 */

#include "SystusRunner.h"
#include "../Abstract/ConfigurationParameters.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>

namespace vega {
namespace systus {

namespace fs = boost::filesystem;
using namespace std;

Runner::ExitCode SystusRunner::execSolver(const ConfigurationParameters &configuration,
        string modelFile) {

    fs::path modelFilePath(modelFile);
    string fname = modelFilePath.stem().string();
    string command = "systus -batch -exec " + modelFile + " > " + fname + ".stdout 2> " + fname
            + ".stderr";
    fs::path outputFsPath(configuration.outputPath);
    cout << configuration.outputPath << endl;
    if (!fs::is_directory(outputFsPath))
        return Runner::ExitCode::SOLVER_NOT_FOUND;
    if (configuration.outputPath != ".")
        command = "cd " + configuration.outputPath + " && " + command;

    // delete previous results
    for (fs::directory_iterator it(outputFsPath); it != fs::directory_iterator(); it++) {
        if (fs::is_regular_file(it->status())) {
            string filename = it->path().filename().string();
            if (filename.find(fname) == 0) {
                if (filename.rfind(".TIT") + 4 == filename.size()
                        || filename.rfind(".fdb") + 4 == filename.size()) {
                    remove(*it);
                }
            }
            if (filename.find("SYSTUS") == 0 && filename.rfind(".DAT") + 4 == filename.size())
                remove(*it);
        }
    }

    //run command
    int i = system(command.c_str());
    if (i != 0) {
        cerr << "Command " << command << " exit code:" << i << endl;
    } else if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Command " << command << " ended with exit code " << i << endl;
    }

    ExitCode exitCode = convertExecResult(i);

    // test if any error occurred during computation
    string outputFileStr = (outputFsPath / fs::path(fname + ".stdout")).string();
    ifstream outputFile(outputFileStr);
    string line;
    int lineNumber = 0;
    while (getline(outputFile, line)) {
        lineNumber += 1;
        if (line.find("ERROR") != string::npos && line.find("NO ERROR") == string::npos){
            cerr << "Error found : line " << lineNumber << " file: " << outputFileStr << endl;
            exitCode = Runner::ExitCode::SOLVER_EXIT_NOT_ZERO;
        }
    }

    // test if result files exist
    if (exitCode == Runner::ExitCode::OK) {
        for (fs::directory_iterator it(outputFsPath); it != fs::directory_iterator(); it++) {
            if (fs::is_regular_file(it->status())) {
                string filename = it->path().filename().string();
                if (filename.find(fname + "_") == 0 && filename.rfind(".DAT") + 4 == filename.size()) {
                    string titFilename = fname + "_DATA" +
                            filename.substr(fname.size() + 1, filename.size() - (fname.size() + 1) - 4) + ".TIT";
                    string fdbFilename = fname + "_POST" +
                            filename.substr(fname.size() + 1, filename.size() - (fname.size() + 1) - 4) + ".fdb";
                    string resuFilename = fname + "_" +
                            filename.substr(fname.size() + 1, filename.size() - (fname.size() + 1) - 4) + ".RESU";
                    if (!(fs::exists(outputFsPath / fs::path(titFilename)) &&
                            fs::exists(outputFsPath / fs::path(fdbFilename)) ))
                        return Runner::ExitCode::SOLVER_RESULT_NOT_FOUND;
                    if (fs::exists(outputFsPath / fs::path(resuFilename))){
                        //check if it contains nook
                        string resuFileStr = (outputFsPath / fs::path(resuFilename)).string();
                        string line2;
                        int lineNumber2 = 0;
                        ifstream resuFile(resuFileStr);
                        while (getline(resuFile, line2)) {
                            lineNumber2 += 1;
                            if (line2.find("NOOK") != string::npos) {
                                cerr << "Test fail: line " << lineNumber2 << " file: " << resuFilename << endl;
                                exitCode = Runner::ExitCode::TEST_FAIL;
                            }
                        }

                    }
                }
            }

        }
    }
    return exitCode;
}

} /* namespace systus */
} /* namespace vega */

