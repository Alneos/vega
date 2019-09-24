/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * VegaCommandLine_test.cpp
 *
 *  Created on: Sep 1, 2013
 *      Author: devel
 */

#define BOOST_TEST_MODULE nastran2systus_test
#define BOOST_TEST_IGNORE_NON_ZERO_CHILD_CODE

#include <boost/test/unit_test.hpp>
#include <string>
#include "build_properties.h"
//#define RUN_SYSTUS false
#include "CommandLineUtils.h"
//____________________________________________________________________________//

namespace vega {
namespace tests {

using namespace std;

BOOST_AUTO_TEST_CASE( test4a ) {
	CommandLineUtils::nastranStudy2Systus("/alneos/test4a/test4a.dat", RUN_SYSTUS, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbar1mod ) {
	CommandLineUtils::nastranStudy2Systus("/alneos/rbar1mod/rbar1mod.dat", RUN_SYSTUS, true, 0.005);
}

BOOST_AUTO_TEST_CASE( rod1freeforce ) {
	CommandLineUtils::nastranStudy2Systus("/alneos/rod1freeforce/rod1freeforce.dat", RUN_SYSTUS, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( prob19 ) {
	CommandLineUtils::nastranStudy2Systus("/caw/prob19/prob19.dat", RUN_SYSTUS, true, 0.005);
}

BOOST_AUTO_TEST_CASE( fixedcircularplate ) {
	CommandLineUtils::nastranStudy2Systus("/irt/fixed_circular_plate/fixed_circular_plate.dat", RUN_SYSTUS, true, 0.2);
}

BOOST_AUTO_TEST_CASE( cbush ) {
	CommandLineUtils::nastranStudy2Systus("/irt/cbush/cbush.inp", RUN_SYSTUS, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( dmigstfs ) {
	CommandLineUtils::nastranStudy2Systus("/irt/dmigstfs/dmigstfs.nas", RUN_SYSTUS, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( dmigstfl ) {
	CommandLineUtils::nastranStudy2Systus("/irt/dmigstfl/dmigstfl.nas", RUN_SYSTUS, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( truss1 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/truss1/truss1.nas", RUN_SYSTUS, true, 0.001);
}

BOOST_AUTO_TEST_CASE( truss3 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/truss3/truss3.nas", RUN_SYSTUS, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( truss4 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/truss4/truss4.nas", RUN_SYSTUS, true, 0.000001);
}

//BOOST_AUTO_TEST_CASE( truss5 ) {
// COMBINATION
//	CommandLineUtils::nastranStudy2Systus("/irt/truss5/truss5.nas", RUN_SYSTUS, true, 0.001);
//}

BOOST_AUTO_TEST_CASE( pload4 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/pload4/pload4.nas", RUN_SYSTUS, true, 0.00001);
}

//BOOST_AUTO_TEST_CASE( VSLX0005 ) {
//	CommandLineUtils::nastranStudy2Systus("/../../../../tmp/tests/VSLX0005-19/VSLX0005-19.nas", RUN_SYSTUS, true, 0.00001);
//}
//
//BOOST_AUTO_TEST_CASE( VSLX0006 ) {
//	CommandLineUtils::nastranStudy2Systus("/../../../../tmp/tests/VSLX0006-19/VSLX0006-19.nas", RUN_SYSTUS, true, 0.00001);
//}

BOOST_AUTO_TEST_CASE( rbe3 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/rbe3/rbe3.nas", RUN_SYSTUS, true, 0.5);
}

//BOOST_AUTO_TEST_CASE( cbush1 ) {
//  Missing DIRECT solver
//	CommandLineUtils::nastranStudy2Systus("/irt/cbush1/cbush1.bdf", RUN_SYSTUS, true, 0.00001);
//}

//BOOST_AUTO_TEST_CASE( cbush1b ) {
//  Missing DIRECT solver
//	CommandLineUtils::nastranStudy2Systus("/irt/cbush1b/cbush1b.bdf", RUN_SYSTUS, true, 0.00001);
//}

//BOOST_AUTO_TEST_CASE( sdld27a ) {
// Missing DISCRET_0D translation
//	CommandLineUtils::nastranStudy2Systus("/irt/sdld27a/sdld27a.bdf", RUN_SYSTUS, true, 0.00001);
//}

BOOST_AUTO_TEST_CASE( ssll11a ) {
	CommandLineUtils::nastranStudy2Systus("/irt/ssll11a/ssll11a.nas", RUN_SYSTUS, true, 0.0001);
}

BOOST_AUTO_TEST_CASE( ssll11c ) {
	CommandLineUtils::nastranStudy2Systus("/irt/ssll11c/ssll11c.nas", RUN_SYSTUS, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( bugcoords ) {
	CommandLineUtils::nastranStudy2Systus("/irt/bug-coord/bug-coord.nas", false, true, 0.00001);
}

//BOOST_AUTO_TEST_CASE( nas101prob2 ) {
    // ForceLine
	//CommandLineUtils::nastranStudy2Systus("/irt/nas101prob2/nas101prob2.dat", RUN_SYSTUS, true, 0.15);
//}

BOOST_AUTO_TEST_CASE( nas101prob3 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/nas101prob3/nas101prob3.nas", RUN_SYSTUS, true, 0.005);
}

BOOST_AUTO_TEST_CASE( nas101prob4 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/nas101prob4/nas101prob4.nas", RUN_SYSTUS, false, 0.005);
}

//BOOST_AUTO_TEST_CASE( nas102prob5 ) {
//  Missing DIRECT solver
//	CommandLineUtils::nastranStudy2Systus("/irt/nas102prob5/nas102prob5.bdf", RUN_SYSTUS, true, 0.05);
//}

BOOST_AUTO_TEST_CASE( nas101prob6 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/nas101prob6/nas101prob6.nas", RUN_SYSTUS, true, 0.3);
}

BOOST_AUTO_TEST_CASE( nas101probA ) {
	CommandLineUtils::nastranStudy2Systus("/irt/nas101probA/nas101probA.bdf", RUN_SYSTUS, true, 0.05);
}

BOOST_AUTO_TEST_CASE( nas101probB ) {
	CommandLineUtils::nastranStudy2Systus("/irt/nas101probB/nas101probB.nas", RUN_SYSTUS, true, 0.1);
}

BOOST_AUTO_TEST_CASE( cantibeam ) {
	CommandLineUtils::nastranStudy2Systus("/irt/cantibeam/cantibeam.nas", RUN_SYSTUS, true, 0.05);
}

BOOST_AUTO_TEST_CASE( n4w101 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/n4w101/n4w101.nas", RUN_SYSTUS, true, 0.02);
}

BOOST_AUTO_TEST_CASE( nas103prob5 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/nas103prob5b/nas103prob5b.nas", RUN_SYSTUS, true, 0.005);
}

BOOST_AUTO_TEST_CASE( pload4_ctetra_multi ) {
	CommandLineUtils::nastranStudy2Systus("/irt/pload4-ctetra-multi/pload4-ctetra-multi.nas", RUN_SYSTUS, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( pload4_chexa_multi ) {
	CommandLineUtils::nastranStudy2Systus("/irt/pload4-chexa-multi/pload4-chexa-multi.nas", false, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( gpstress ) {
	CommandLineUtils::nastranStudy2Systus("/irt/gpstress/gpstress.nas", RUN_SYSTUS, true, 0.00001);
}

//BOOST_AUTO_TEST_CASE( hexa1 ) {
//	CommandLineUtils::nastranStudy2Systus("/irt/hexa1/hexa1.nas", RUN_SYSTUS, true, 0.00001);
//}

//BOOST_AUTO_TEST_CASE( q4sdcon ) {
//	CommandLineUtils::nastranStudy2Systus("/irt/q4sdcon/q4sdcon.nas", RUN_SYSTUS, true, 0.00001);
//}

BOOST_AUTO_TEST_CASE( test4a_rbe2 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/test4a_rbe2/test4a_rbe2.nas", RUN_SYSTUS, true, 0.05);
}

BOOST_AUTO_TEST_CASE( rbe2_z ) {
	CommandLineUtils::nastranStudy2Systus("/irt/rbe2_z/rbe2_z.nas", RUN_SYSTUS, true, 1.5);
}

BOOST_AUTO_TEST_CASE( rbe2_y ) {
	CommandLineUtils::nastranStudy2Systus("/irt/rbe2_y/rbe2_y.nas", RUN_SYSTUS, true, 1.5);
}

BOOST_AUTO_TEST_CASE( rbe2_yz ) {
	CommandLineUtils::nastranStudy2Systus("/irt/rbe2_yz/rbe2_yz.nas", RUN_SYSTUS, true, 1.5);
}

BOOST_AUTO_TEST_CASE( rbe2_3nodes ) {
	CommandLineUtils::nastranStudy2Systus("/irt/rbe2_3nodes/rbe2_3nodes.nas", RUN_SYSTUS, true, 1.5);
}

BOOST_AUTO_TEST_CASE( rbe2_4nodes ) {
	CommandLineUtils::nastranStudy2Systus("/irt/rbe2_4nodes/rbe2_4nodes.nas", RUN_SYSTUS, true, 1.5);
}

BOOST_AUTO_TEST_CASE( rbe2_beam_coinc ) {
	CommandLineUtils::nastranStudy2Systus("/irt/rbe2_beam_coinc/rbe2_beam_coinc.nas", RUN_SYSTUS, true, 0.03);
}

BOOST_AUTO_TEST_CASE( rbe2_hexapoi_coinc ) {
	CommandLineUtils::nastranStudy2Systus("/irt/rbe2_hexapoi_coinc/rbe2_hexapoi_coinc.nas", RUN_SYSTUS, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbe2_hexapoi_3nodes ) {
	CommandLineUtils::nastranStudy2Systus("/irt/rbe2_hexapoi_3nodes/rbe2_hexapoi_3nodes.nas", RUN_SYSTUS, true, 0.02);
}

BOOST_AUTO_TEST_CASE( rbe2_hexapoi_aligndist ) {
	CommandLineUtils::nastranStudy2Systus("/irt/rbe2_hexapoi_aligndist/rbe2_hexapoi_aligndist.nas", RUN_SYSTUS, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbe2_hexapoi_nonalign_dist ) {
	CommandLineUtils::nastranStudy2Systus("/irt/rbe2_hexapoi_nonalign_dist/rbe2_hexapoi_nonalign_dist.nas", RUN_SYSTUS, true, 0.00001);
}

} /* namespace test */
} /* namespace vega */
