/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * vegamain.cpp
 *
 *  Created on: Dec 13, 2012
 *      Author: dallolio
 */
#if defined(unix)
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "build_properties.h"
#include "VegaCommandLine.h"
#include <clocale>
#include <cstdlib>
#include <iostream>

using namespace vega;
using namespace std;

int main(int ac, const char* av[]) {
    //workaround for https://svn.boost.org/trac/boost/ticket/5928
    setlocale(LC_ALL, "C");
    putenv(strdup("LC_ALL=C" /* fix for deprecated conversion */));
#if defined(unix)
    pid_t child_pid = fork();
    if (child_pid > 0) {
        //parent process
        int status = 0;
        wait(&status);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            cerr << "The child did not provide an exit code. It probably crashed?" << endl;
            return VegaCommandLine::CHILD_CRASHED;
        }
    } else if (child_pid == 0) {
#endif
        VegaCommandLine vcl = VegaCommandLine();
        VegaCommandLine::ExitCode exitCode = vcl.process(ac, av);
        if (exitCode != VegaCommandLine::OK) {
            cerr << VegaCommandLine::exitCodeToString(exitCode) << endl;
            cerr << "Exitcode:" << exitCode << endl;
        }
        return exitCode;
#if defined(unix)
    } else {
        //child pid <0
        cerr << "fork failed " << child_pid << endl << " exiting " << endl;
        return VegaCommandLine::FORK_FAILED;
    }
#endif
}

