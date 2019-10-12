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
#include <boost/algorithm/string/predicate.hpp>
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
    string fnamePrefix = fname.substr(0, fname.find("_ALL"));

    fs::path outputFsPath(configuration.outputPath);
    cout << configuration.outputPath << endl;
    if (!fs::is_directory(outputFsPath))
        return Runner::ExitCode::SOLVER_NOT_FOUND;

    // delete previous results
    for (fs::directory_iterator it(outputFsPath); it != fs::directory_iterator(); it++) {
        if (fs::is_regular_file(it->status())) {
            string filename = it->path().filename().string();
            if (filename.find(fnamePrefix) == 0) {
                if (boost::algorithm::ends_with(filename, ".TIT")
                        or boost::algorithm::ends_with(filename, ".fdb")
                        or boost::algorithm::ends_with(filename, ".RESU")) {
                    remove(*it);
                }
            }
            if (filename.find("SYSTUS") == 0 && filename.rfind(".DAT") + 4 == filename.size())
                remove(*it);
        }
    }

    ExitCode exitCode;
    if (configuration.systusOutputProduct=="systus") {
        string command = "systus -batch -exec " + modelFile + " > " + fname + ".stdout 2> " + fname
                + ".stderr";
        if (configuration.outputPath != ".")
            command = "cd " + configuration.outputPath + " && " + command;
        //run command
        int i = system(command.c_str());
        if (i != 0) {
            cerr << "Command " << command << " FAILED with exit code:" << i << endl;
        } else if (configuration.logLevel >= LogLevel::DEBUG) {
            cout << "Command " << command << " ended with exit code " << i << endl;
        }
        exitCode = convertExecResult(i);
    } else {
        for (fs::directory_iterator it2( outputFsPath ); it2 != fs::directory_iterator(); it2++ ) {
            if( !fs::is_regular_file( it2->status() ) )
                continue;
            string datFilename = it2->path().filename().string();
            if (not boost::algorithm::ends_with(datFilename, ".DAT"))
                continue;
            if (not boost::algorithm::starts_with(datFilename, fnamePrefix))
                continue;
            cout << "Found DAT file:" << datFilename << endl;
            //check if it contains nook
            string datFileStr = (outputFsPath / fs::path(datFilename)).string();
            string command = "systus -batch -exec " + datFilename + " > " + fname + ".stdout 2> " + fname
                    + ".stderr";
            if (configuration.outputPath != ".")
                command = "cd " + configuration.outputPath + " && " + command;
            //run command
            int i = system(command.c_str());
            if (i != 0) {
                cerr << "Command " << command << " FAILED with exit code:" << i << endl;
            } else if (configuration.logLevel >= LogLevel::DEBUG) {
                cout << "Command " << command << " ended with exit code " << i << endl;
            }
            exitCode = convertExecResult(i);
            if (exitCode != Runner::ExitCode::OK)
                return exitCode;
        }
    }

    // test if any error occurred during computation
    string outputFileStr = (outputFsPath / fs::path(fname + ".stdout")).string();
    ifstream outputFile(outputFileStr);
    string line;
    int lineNumber = 0;
    while (getline(outputFile, line)) {
        lineNumber += 1;
        if (line.find("ERROR") != string::npos && line.find("NO ERROR") == string::npos) {
            cerr << "Error found : line " << lineNumber << " file: " << outputFileStr << endl;
            exitCode = Runner::ExitCode::SOLVER_EXIT_NOT_ZERO;
        }
    }

    // test if result files exist
    if (exitCode == Runner::ExitCode::OK) {
        if (configuration.systusOutputProduct=="systus") {
            bool foundTIT = false;
            for (fs::directory_iterator it2( outputFsPath ); it2 != fs::directory_iterator(); it2++ ) {
                if (not fs::is_regular_file( it2->status() ) )
                    continue;
                string filename2 = it2->path().filename().string();
                if (not boost::algorithm::ends_with(filename2, ".TIT"))
                    continue;
                if (not boost::algorithm::starts_with(filename2, fnamePrefix))
                    continue;
                foundTIT = true;
                break;
            }
            bool foundFDB = false;
            for (fs::directory_iterator it2( outputFsPath ); it2 != fs::directory_iterator(); it2++ ) {
                if (not fs::is_regular_file( it2->status() ) )
                    continue;
                string filename2 = it2->path().filename().string();
                if (not boost::algorithm::ends_with(filename2, ".fdb"))
                    continue;
                if (not boost::algorithm::starts_with(filename2, fnamePrefix))
                    continue;
                foundFDB = true;
                break;
            }
            if (not foundTIT and not foundFDB) {
                cerr << "Cannot find results after test." << endl;
                return Runner::ExitCode::SOLVER_RESULT_NOT_FOUND;
            }
        }
        for (fs::directory_iterator it2( outputFsPath ); it2 != fs::directory_iterator(); it2++ ) {
            if( !fs::is_regular_file( it2->status() ) )
                continue;
            string resuFilename = it2->path().filename().string();
            if (not boost::algorithm::ends_with(resuFilename, ".RESU"))
                continue;
            if (not boost::algorithm::starts_with(resuFilename, fnamePrefix))
                continue;
            cout << "Found RESU file:" << resuFilename << endl;
            //check if it contains nook
            string resuFileStr = (outputFsPath / fs::path(resuFilename)).string();
            string line2;
            int lineNumber2 = 0;
            ifstream resuFile(resuFileStr);
            while (getline(resuFile, line2)) {
                lineNumber2 += 1;
                if (line2.find("NOOK") != string::npos) {
                    cerr << "Test fail: line " << lineNumber2 << " file: " << resuFilename << endl;
                    cerr << line2 << endl;
                    exitCode = Runner::ExitCode::TEST_FAIL;
                }
            }
        }
    }
    return exitCode;
}

} /* namespace systus */
} /* namespace vega */

