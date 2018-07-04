/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Utility.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: devel
 */

#include "Utility.h"
#include "Value.h"
#include <math.h>       /* pow */
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <cmath>

namespace ublas = boost::numeric::ublas;

namespace vega {

using namespace std;

bool InvertMatrix(const ublas::matrix<double>& input, ublas::matrix<double>& inverse)
{
   typedef ublas::permutation_matrix<std::size_t> pmatrix;

   // create a working copy of the input
   ublas::matrix<double> A(input);

   // create a permutation matrix for the LU-factorization
   pmatrix pm(A.size1());

   // perform LU-factorization
   int res = boost::numeric_cast<int>(lu_factorize(A, pm));
   if (res != 0)
       return false;

   // create identity matrix of "inverse"
   inverse.assign(ublas::identity_matrix<double> (A.size1()));

   // backsubstitute to get the inverse
   ublas::lu_substitute(A, pm, inverse);
   return true;
}

#if defined(__linux__)

// Call this function to get a backtrace.
void backtrace() {
  unw_cursor_t cursor;
  unw_context_t context;

  // Initialize cursor to current frame for local unwinding.
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  // Unwind frames one by one, going up the frame stack.
  while (unw_step(&cursor) > 0) {
    unw_word_t offset, pc;
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }
    printf("0x%lx:", pc);

    char sym[256];
    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      printf(" (%s+0x%lx)\n", sym, offset);
    } else {
      printf(" -- error: unable to obtain symbol name for this frame\n");
    }
  }
}
#elif _WIN32
#include <windows.h>
#include <DbgHelp.h>

#include <stdio.h>
#include <stdlib.h>

static void backtrace(void) {

  HANDLE process = GetCurrentProcess();
  HANDLE thread = GetCurrentThread();

  CONTEXT context;
  memset(&context, 0, sizeof(CONTEXT));
  context.ContextFlags = CONTEXT_FULL;
  RtlCaptureContext(&context);

  SymInitialize(process, NULL, TRUE);

  DWORD image;
  STACKFRAME64 stackframe;
  ZeroMemory(&stackframe, sizeof(STACKFRAME64));

  image = IMAGE_FILE_MACHINE_AMD64;
  stackframe.AddrPC.Offset = context.Rip;
  stackframe.AddrPC.Mode = AddrModeFlat;
  stackframe.AddrFrame.Offset = context.Rsp;
  stackframe.AddrFrame.Mode = AddrModeFlat;
  stackframe.AddrStack.Offset = context.Rsp;
  stackframe.AddrStack.Mode = AddrModeFlat;

  for (size_t i = 0; i < 25; i++) {

    BOOL result = StackWalk64(
      image, process, thread,
      &stackframe, &context, NULL,
      SymFunctionTableAccess64, SymGetModuleBase64, NULL);

    if (!result) { break; }

    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = MAX_SYM_NAME;

    DWORD64 displacement = 0;
    if (SymFromAddr(process, stackframe.AddrPC.Offset, &displacement, symbol)) {
      printf("[%i] %s\n", i, symbol->Name);
    } else {
      printf("[%i] ???\n", i);
    }

  }

  SymCleanup(process);

}

#else
// Call this function to get a backtrace.
void backtrace() {
    std::cerr << "No backtrace (yet) on this platform, should really add it...\n";
}
#endif

void handler(int sig) {
    // print out all the frames to stderr
    std::cerr << "Error: signal " << sig << std::endl;
    backtrace();
    exit(1);
}

//__________ ValueOrReference

} /* namespace vega */
