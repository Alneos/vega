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
 */

#define BOOST_TEST_MODULE CoordinateSystem_test
#include <boost/test/unit_test.hpp>
#include "../../Abstract/Model.h"
#include "../../Abstract/CoordinateSystem.h"

using namespace std;
using namespace vega;

BOOST_AUTO_TEST_CASE( test_CoordinateSystem ) {
    Model model("test");

    VectorialValue O(0., 0., 0.);
    const VectorialValue X = VectorialValue::X;
    const VectorialValue Y = VectorialValue::Y;
    const VectorialValue Z = VectorialValue::Z;
    const double k = 2.5;

    {
        // same as global coordinate system
        CartesianCoordinateSystem XYZ(*model.mesh, O, X, Y);
        BOOST_CHECK_EQUAL(k * X , XYZ.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * Y , XYZ.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , XYZ.vectorToGlobal(k * Z));

        // permutation 1
        CartesianCoordinateSystem YZX(*model.mesh, O, Y, Z);
        BOOST_CHECK_EQUAL(k * Y , YZX.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * Z , YZX.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * X , YZX.vectorToGlobal(k * Z));

        // permutation 2
        CartesianCoordinateSystem ZXY(*model.mesh, O, Z, X);
        BOOST_CHECK_EQUAL(k * Z , ZXY.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * X , ZXY.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Y , ZXY.vectorToGlobal(k * Z));

        // translation
        CartesianCoordinateSystem Ztran(*model.mesh, Z, X, Y);
        BOOST_CHECK_EQUAL(Ztran.getOrigin() , Z);
        BOOST_CHECK_EQUAL(k * X , XYZ.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * Y , XYZ.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , XYZ.vectorToGlobal(k * Z));

        // re-orientation of X by rotation over Z (45°)
        CartesianCoordinateSystem Xrot45(*model.mesh, O, X + Y, Y);
        BOOST_CHECK_EQUAL(k * (X + Y) / sqrt(2) , Xrot45.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * (Y - X) / sqrt(2) , Xrot45.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , Xrot45.vectorToGlobal(k * Z));

        // re-orientation of X by rotation over Z (45°) and over y (45°)
        CartesianCoordinateSystem Xrot4545(*model.mesh, O, X + Y + Z, Y);
        BOOST_CHECK_EQUAL(k * (X + Y + Z) / sqrt(3) , Xrot4545.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * (-1 * X + 2 * Y - Z) / sqrt(6) , Xrot4545.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * (-1 * X + Z) / sqrt(2) , Xrot4545.vectorToGlobal(k * Z));
    }

    {
        // same as global coordinate system
        CylindricalCoordinateSystem XYZ(*model.mesh, O, X, Y);
        BOOST_CHECK_EQUAL(k * X , XYZ.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * Y , XYZ.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , XYZ.vectorToGlobal(k * Z));

        XYZ.updateLocalBase(X); // no change
        BOOST_CHECK_EQUAL(k * X , XYZ.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * Y , XYZ.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , XYZ.vectorToGlobal(k * Z));

        XYZ.updateLocalBase(Y); // Pi/2 rotation
        BOOST_CHECK_EQUAL(k * Y , XYZ.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(-k * X , XYZ.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , XYZ.vectorToGlobal(k * Z));

        XYZ.updateLocalBase(X + Y); // Pi/4 rotation
        BOOST_CHECK_EQUAL(k * (X + Y) / sqrt(2) , XYZ.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * (-1 * X + Y) / sqrt(2) , XYZ.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , XYZ.vectorToGlobal(k * Z));

        XYZ.updateLocalBase(X + Y + Z); // no change (Pi/4 rotation)
        BOOST_CHECK_EQUAL(k * (X + Y) / sqrt(2) , XYZ.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * (-1 * X + Y) / sqrt(2) , XYZ.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , XYZ.vectorToGlobal(k * Z));

        // translation over Z (no changes)
        CylindricalCoordinateSystem Ztran(*model.mesh, Z, X, Y);
        BOOST_CHECK_EQUAL(Ztran.getOrigin() , Z);
        BOOST_CHECK_EQUAL(k * X , Ztran.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * Y , Ztran.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , Ztran.vectorToGlobal(k * Z));

        Ztran.updateLocalBase(X); // no change
        BOOST_CHECK_EQUAL(k * X , Ztran.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * Y , Ztran.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , Ztran.vectorToGlobal(k * Z));

        Ztran.updateLocalBase(Y); // Pi/2 rotation
        BOOST_CHECK_EQUAL(k * Y , Ztran.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(-k * X , Ztran.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , Ztran.vectorToGlobal(k * Z));

        // translation over X, changes center of rotation
        CylindricalCoordinateSystem Xtran(*model.mesh, X, X, Y);
        BOOST_CHECK_EQUAL(k * X , Xtran.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * Y , Xtran.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , Xtran.vectorToGlobal(k * Z));

        Xtran.updateLocalBase(2 * X); // no change
        BOOST_CHECK_EQUAL(k * X , Xtran.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * Y , Xtran.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , Xtran.vectorToGlobal(k * Z));

        Xtran.updateLocalBase(Y); // 3*Pi/4 rotation because center of rotation changed
        BOOST_CHECK_EQUAL(k * (-1 * X + Y) / sqrt(2) , Xtran.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * (-1 * X - 1 * Y) / sqrt(2) , Xtran.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , Xtran.vectorToGlobal(k * Z));

        // re-orientation of X by rotation over Z (45°)
        CylindricalCoordinateSystem Xrot45(*model.mesh, O, X + Y, Y);
        BOOST_CHECK_EQUAL(k * (X + Y) / sqrt(2) , Xrot45.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * (Y - X) / sqrt(2) , Xrot45.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , Xrot45.vectorToGlobal(k * Z));

        Xrot45.updateLocalBase(X + Y); // no change
        BOOST_CHECK_EQUAL(k * (X + Y) / sqrt(2) , Xrot45.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(k * (Y - X) / sqrt(2) , Xrot45.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , Xrot45.vectorToGlobal(k * Z));

        Xrot45.updateLocalBase(Y); // Pi/2 rotation
        BOOST_CHECK_EQUAL(k * Y , Xrot45.vectorToGlobal(k * X));
        BOOST_CHECK_EQUAL(-k * X , Xrot45.vectorToGlobal(k * Y));
        BOOST_CHECK_EQUAL(k * Z , Xrot45.vectorToGlobal(k * Z));

    }

}

BOOST_AUTO_TEST_CASE( test_chaining_CoordinateSystem ) {
    Model model("test2");

    VectorialValue O1(10., 20., 30.);
    const VectorialValue X = VectorialValue::X;
    const VectorialValue Y = VectorialValue::Y;
    const VectorialValue Z = VectorialValue::Z;

    int csid1 = 42;
    int rpos1 = model.mesh->findOrReserveCoordinateSystem(csid1);
    CartesianCoordinateSystem cs1(*model.mesh, O1, X, Y, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, csid1);
    model.mesh->add(cs1);

    VectorialValue O2(15., 25., 35.);

    CartesianCoordinateSystem cs2(*model.mesh, O2, X, Y, rpos1);
    model.mesh->add(cs2);

    VectorialValue O(0., 0., 0.);
    VectorialValue expectCS2O(25., 45., 65.);
    BOOST_CHECK_EQUAL(expectCS2O, cs2.positionToGlobal(O));
}
