/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * NastranParser_test.cpp
 *
 *  Created on: Dec 24, 2012
 *      Author: dallolio
 */

#define BOOST_TEST_MODULE nastran_parser_tests
#include "../../Nastran/NastranWriter.h"
#include "build_properties.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#if defined VDEBUG && defined __GNUC_ && !defined(_WIN32)
#include <valgrind/memcheck.h>
#endif

using namespace std;
using namespace vega;
using namespace nastran;

//____________________________________________________________________________//

BOOST_AUTO_TEST_CASE( test_float_write ) {
    std::ostringstream strs;
    strs << Line("GRID").add(3.141593);
    BOOST_CHECK_EQUAL(strs.str(), "GRID    3.1416+0\n");
}


//____________________________________________________________________________//
