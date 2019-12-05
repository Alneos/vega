/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * VegaCommandLine_test.cpp
 *
 *  Created on: Sep 1, 2013
 *      Author: devel
 */

#define BOOST_TEST_MODULE nastran2aster_test
#define BOOST_TEST_IGNORE_NON_ZERO_CHILD_CODE
//#define BOOST_TEST_IGNORE_SIGCHLD

//#define BOOST_TEST_LIMITED_SIGNAL_DETAILS

#include <boost/test/unit_test.hpp>
#include <string>
#include "build_properties.h"
#include "CommandLineUtils.h"

//____________________________________________________________________________//

namespace vega {
namespace tests {

using namespace std;

BOOST_AUTO_TEST_CASE( prob19 ) {
	CommandLineUtils::nastranStudy2Aster("/caw/prob19/prob19.dat", RUN_ASTER, true, 0.002);
}

BOOST_AUTO_TEST_CASE( prob30c ) {
	CommandLineUtils::nastranStudy2Aster("/caw/prob30c/prob30c.dat", RUN_ASTER, true, 0.9);
}

BOOST_AUTO_TEST_CASE( test4a ) {
	CommandLineUtils::nastranStudy2Aster("/alneos/test4a/test4a.dat", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rod1freeforce ) {
	CommandLineUtils::nastranStudy2Aster("/alneos/rod1freeforce/rod1freeforce.dat", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( fixedcircularplate ) {
	CommandLineUtils::nastranStudy2Aster("/irt/fixed_circular_plate/fixed_circular_plate.dat", RUN_ASTER, true, 0.2);
}

BOOST_AUTO_TEST_CASE( t01331a ) {
    // Results NOOK, still missing THETA CTRIA3 orientation (should become a ANGL_VRIL)
	CommandLineUtils::nastranStudy2Aster("/irt/t01331/t01331a.inp", false, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( cbush ) {
	CommandLineUtils::nastranStudy2Aster("/irt/cbush/cbush.inp", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( cbush1 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/cbush1/cbush1.bdf", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( cbush1b ) {
	CommandLineUtils::nastranStudy2Aster("/irt/cbush1b/cbush1b.bdf", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( dmigstfs ) {
	CommandLineUtils::nastranStudy2Aster("/irt/dmigstfs/dmigstfs.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( dmigstfl ) {
	CommandLineUtils::nastranStudy2Aster("/irt/dmigstfl/dmigstfl.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( sdld27a ) {
	CommandLineUtils::nastranStudy2Aster("/irt/sdld27a/sdld27a.bdf", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( ssll11a ) {
	CommandLineUtils::nastranStudy2Aster("/irt/ssll11a/ssll11a.nas", RUN_ASTER, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( ssll11c ) {
	CommandLineUtils::nastranStudy2Aster("/irt/ssll11c/ssll11c.nas", RUN_ASTER, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( ssnv129a ) {
	CommandLineUtils::nastranStudy2Aster("/irt/ssnv129a/ssnv129a.bdf", RUN_ASTER, true, 0.01);
}

BOOST_AUTO_TEST_CASE( ssnv104i ) {
	CommandLineUtils::nastranStudy2Aster("/irt/ssnv104i/ssnv104i.bdf", RUN_ASTER, true, 0.04);
}

BOOST_AUTO_TEST_CASE( appd ) {
	CommandLineUtils::nastranStudy2Aster("/irt/appd/appd.nas", RUN_ASTER, true, 0.02);
}

BOOST_AUTO_TEST_CASE( bugcoords ) {
	CommandLineUtils::nastranStudy2Aster("/irt/bug-coord/bug-coord.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( pload4 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/pload4/pload4.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( pload4b ) {
	CommandLineUtils::nastranStudy2Aster("/irt/pload4b/pload4b.nas", RUN_ASTER, true, 0.00001);
}

//BOOST_AUTO_TEST_CASE( pload4c ) {
// underintegration different results
//	CommandLineUtils::nastranStudy2Aster("/irt/pload4c/pload4c.nas", RUN_ASTER, true, 0.00001);
//}

BOOST_AUTO_TEST_CASE( nas101prob2 ) {
    // ok but using refinement in beams (cannot refine a truss in Aster)
	CommandLineUtils::nastranStudy2Aster("/irt/nas101prob2/nas101prob2.dat", RUN_ASTER, true, 0.03);
}

BOOST_AUTO_TEST_CASE( nas101prob3 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/nas101prob3/nas101prob3.nas", RUN_ASTER, true, 0.005);
}

BOOST_AUTO_TEST_CASE( nas101prob4 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/nas101prob4/nas101prob4.nas", RUN_ASTER, true, 0.005);
}

BOOST_AUTO_TEST_CASE( nas101prob5 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/nas101prob5/nas101prob5.nas", RUN_ASTER, true, 0.0001);
}

BOOST_AUTO_TEST_CASE( nas101prob6 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/nas101prob6/nas101prob6.nas", RUN_ASTER, true, 0.09);
}

BOOST_AUTO_TEST_CASE( nas101prob7 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/nas101prob7/nas101prob7.nas", RUN_ASTER, true, 0.035);
}

BOOST_AUTO_TEST_CASE( nas101probA ) {
	CommandLineUtils::nastranStudy2Aster("/irt/nas101probA/nas101probA.bdf", RUN_ASTER, true, 0.05);
}

BOOST_AUTO_TEST_CASE( nas101probB ) {
	CommandLineUtils::nastranStudy2Aster("/irt/nas101probB/nas101probB.nas", RUN_ASTER, true, 0.05);
}

BOOST_AUTO_TEST_CASE( nas102prob5 ) {
    // TODO : LD missing G param, tests not found, otherwise ok
	CommandLineUtils::nastranStudy2Aster("/irt/nas102prob5/nas102prob5.bdf", RUN_ASTER, true, 0.05);
}

BOOST_AUTO_TEST_CASE( nas102prob6 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/nas102prob6/nas102prob6.nas", RUN_ASTER, true, 0.03);
}

BOOST_AUTO_TEST_CASE( nas102prob14a ) {
	CommandLineUtils::nastranStudy2Aster("/irt/nas102prob14a/nas102prob14a.nas", RUN_ASTER, true, 0.006);
}

BOOST_AUTO_TEST_CASE( nas103prob1c ) {
    // Singular matrix in Aster (rod)
	CommandLineUtils::nastranStudy2Aster("/irt/nas103prob1c/nas103prob1c.nas", false, true, 0.02);
}

BOOST_AUTO_TEST_CASE( nas103prob3 ) {
    // Singular matrix in Aster in LGDISP (rod), ok without LGDISP and NLPCI
	CommandLineUtils::nastranStudy2Aster("/irt/nas103prob3/nas103prob3.nas", RUN_ASTER, true, 0.02);
}

BOOST_AUTO_TEST_CASE( nas103prob4a ) {
    // Cannot solve buckling using BARRE, should use POUTRE (+maybe DISCRETS at each end)
	CommandLineUtils::nastranStudy2Aster("/irt/nas103prob4a/nas103prob4a.nas", false, true, 0.02);
}

BOOST_AUTO_TEST_CASE( nas103prob5 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/nas103prob5/nas103prob5.nas", RUN_ASTER, true, 0.005);
}

BOOST_AUTO_TEST_CASE( nas103prob6 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/nas103prob6/nas103prob6.dat", RUN_ASTER, true, 0.02);
}

BOOST_AUTO_TEST_CASE( nas103prob8 ) {
    // Singular matrix in Aster (rod)
	CommandLineUtils::nastranStudy2Aster("/irt/nas103prob8/nas103prob8.nas", false, true, 0.02);
}

BOOST_AUTO_TEST_CASE( nas103prob9 ) {
    // TODO : LD <CONTACT_60>
	CommandLineUtils::nastranStudy2Aster("/irt/nas103prob9/nas103prob9.dat", false, true, 0.02);
}

BOOST_AUTO_TEST_CASE( nas120probF ) {
	CommandLineUtils::nastranStudy2Aster("/irt/nas120probF/nas120probF.nas", RUN_ASTER, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( test_2dof108 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/2dof108/2dof108.bdf", false, true, 0.05);
}

BOOST_AUTO_TEST_CASE( rbe3 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/rbe3/rbe3.nas", RUN_ASTER, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( truss1 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/truss1/truss1.nas", RUN_ASTER, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( truss3 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/truss3/truss3.nas", RUN_ASTER, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( truss4 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/truss4/truss4.nas", RUN_ASTER, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( truss5 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/truss5/truss5.nas", RUN_ASTER, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( forcefour ) {
	CommandLineUtils::nastranStudy2Aster("/irt/forcefour/forcefour.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( pload ) {
	CommandLineUtils::nastranStudy2Aster("/irt/pload/pload.nas", RUN_ASTER, true, 0.00001);
}

//BOOST_AUTO_TEST_CASE( sine_wave ) {
//  RLOAD2 TYPE=ACCE
//	CommandLineUtils::nastranStudy2Aster("/irt/sine_test/sine_test.nas", RUN_ASTER, true, 0.00001);
//}

//BOOST_AUTO_TEST_CASE( aero2728 ) {
// RLOAD2 referencing GRAV
//	CommandLineUtils::nastranStudy2Aster("/irt/aero2728/aero2728.nas", RUN_ASTER, true, 0.00001);
//}

BOOST_AUTO_TEST_CASE( nas102prob6resvec ) {
	CommandLineUtils::nastranStudy2Aster("/irt/nas102prob6resvec/nas102prob6resvec.nas", RUN_ASTER, true, 0.03);
}

BOOST_AUTO_TEST_CASE( cantibeam ) {
	CommandLineUtils::nastranStudy2Aster("/irt/cantibeam/cantibeam.nas", RUN_ASTER, true, 0.02);
}

//BOOST_AUTO_TEST_CASE( cantibox ) {
// SLOW after parsing complete ??
//	CommandLineUtils::nastranStudy2Aster("/irt/cantibox/cantibox.nas", RUN_ASTER, true, 0.02);
//}

BOOST_AUTO_TEST_CASE( n4w101 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/n4w101/n4w101.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( pload4d ) {
    // Tests NOOK
	CommandLineUtils::nastranStudy2Aster("/irt/pload4d/pload4d.nas", false, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( pload4_ctetra_multi ) {
	CommandLineUtils::nastranStudy2Aster("/irt/pload4-ctetra-multi/pload4-ctetra-multi.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( pload4_chexa_multi ) {
	CommandLineUtils::nastranStudy2Aster("/irt/pload4-chexa-multi/pload4-chexa-multi.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( gpstress ) {
	CommandLineUtils::nastranStudy2Aster("/irt/gpstress/gpstress.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( test4a_rbe2 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/test4a_rbe2/test4a_rbe2.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbar1mod ) {
    // cannot understand why should use all 6 dofs in rbar in this case
	CommandLineUtils::nastranStudy2Aster("/alneos/rbar1mod/rbar1mod.dat", false, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbe2_z ) {
	CommandLineUtils::nastranStudy2Aster("/irt/rbe2_z/rbe2_z.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbar_z ) {
	CommandLineUtils::nastranStudy2Aster("/irt/rbar_z/rbar_z.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbe2_z_mpc ) {
	CommandLineUtils::nastranStudy2Aster("/irt/rbe2_z_mpc/rbe2_z_mpc.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbe2_y ) {
	CommandLineUtils::nastranStudy2Aster("/irt/rbe2_y/rbe2_y.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbe2_yz ) {
    // invalid pointer in Aster ??
	CommandLineUtils::nastranStudy2Aster("/irt/rbe2_yz/rbe2_yz.nas", false, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbe2_3nodes ) {
	CommandLineUtils::nastranStudy2Aster("/irt/rbe2_3nodes/rbe2_3nodes.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbe2_4nodes ) {
	CommandLineUtils::nastranStudy2Aster("/irt/rbe2_4nodes/rbe2_4nodes.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbe2_beam_coinc ) {
	CommandLineUtils::nastranStudy2Aster("/irt/rbe2_beam_coinc/rbe2_beam_coinc.nas", RUN_ASTER, true, 0.03);
}

BOOST_AUTO_TEST_CASE( rbe2_hexapoi_coinc ) {
	CommandLineUtils::nastranStudy2Aster("/irt/rbe2_hexapoi_coinc/rbe2_hexapoi_coinc.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbe2_hexapoi_3nodes ) {
	CommandLineUtils::nastranStudy2Aster("/irt/rbe2_hexapoi_3nodes/rbe2_hexapoi_3nodes.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbe2_hexapoi_aligndist ) {
	CommandLineUtils::nastranStudy2Aster("/irt/rbe2_hexapoi_aligndist/rbe2_hexapoi_aligndist.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbe2_hexapoi_nonalign_dist ) {
	CommandLineUtils::nastranStudy2Aster("/irt/rbe2_hexapoi_nonalign_dist/rbe2_hexapoi_nonalign_dist.nas", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbar_massif) {
	CommandLineUtils::nastranStudy2Aster("/irt/rbar_massif/rbar_massif.nas", RUN_ASTER, true, 0.02);
}

BOOST_AUTO_TEST_CASE( allpload) {
	CommandLineUtils::nastranStudy2Aster("/irt/allpload/allpload.nas", RUN_ASTER, true, 0.02);
}

} /* namespace test */
} /* namespace vega */
