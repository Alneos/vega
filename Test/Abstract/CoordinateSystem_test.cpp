/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
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
		CartesianCoordinateSystem XYZ(model, O, X, Y);
		BOOST_CHECK(!!(k * X == XYZ.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * Y == XYZ.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == XYZ.vectorToGlobal(k * Z)));

		// permutation 1
		CartesianCoordinateSystem YZX(model, O, Y, Z);
		BOOST_CHECK(!!(k * Y == YZX.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * Z == YZX.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * X == YZX.vectorToGlobal(k * Z)));

		// permutation 2
		CartesianCoordinateSystem ZXY(model, O, Z, X);
		BOOST_CHECK(!!(k * Z == ZXY.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * X == ZXY.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Y == ZXY.vectorToGlobal(k * Z)));

		// translation
		CartesianCoordinateSystem Ztran(model, Z, X, Y);
		BOOST_CHECK(!!(Ztran.getOrigin() == Z));
		BOOST_CHECK(!!(k * X == XYZ.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * Y == XYZ.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == XYZ.vectorToGlobal(k * Z)));

		// re-orientation of X by rotation over Z (45°)
		CartesianCoordinateSystem Xrot45(model, O, X + Y, Y);
		BOOST_CHECK(!!(k * (X + Y) / sqrt(2) == Xrot45.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * (Y - X) / sqrt(2) == Xrot45.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == Xrot45.vectorToGlobal(k * Z)));

		// re-orientation of X by rotation over Z (45°) and over y (45°)
		CartesianCoordinateSystem Xrot4545(model, O, X + Y + Z, Y);
		BOOST_CHECK(!!(k * (X + Y + Z) / sqrt(3) == Xrot4545.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * (-1 * X + 2 * Y - Z) / sqrt(6) == Xrot4545.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * (-1 * X + Z) / sqrt(2) == Xrot4545.vectorToGlobal(k * Z)));
	}

	{
		// same as global coordinate system
		CylindricalCoordinateSystem XYZ(model, O, X, Y);
		BOOST_CHECK(!!(k * X == XYZ.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * Y == XYZ.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == XYZ.vectorToGlobal(k * Z)));

		XYZ.updateLocalBase(X); // no change
		BOOST_CHECK(!!(k * X == XYZ.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * Y == XYZ.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == XYZ.vectorToGlobal(k * Z)));

		XYZ.updateLocalBase(Y); // Pi/2 rotation
		BOOST_CHECK(!!(k * Y == XYZ.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(-k * X == XYZ.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == XYZ.vectorToGlobal(k * Z)));

		XYZ.updateLocalBase(X + Y); // Pi/4 rotation
		BOOST_CHECK(!!(k * (X + Y) / sqrt(2) == XYZ.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * (-1 * X + Y) / sqrt(2) == XYZ.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == XYZ.vectorToGlobal(k * Z)));

		XYZ.updateLocalBase(X + Y + Z); // no change (Pi/4 rotation)
		BOOST_CHECK(!!(k * (X + Y) / sqrt(2) == XYZ.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * (-1 * X + Y) / sqrt(2) == XYZ.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == XYZ.vectorToGlobal(k * Z)));

		// translation over Z (no changes)
		CylindricalCoordinateSystem Ztran(model, Z, X, Y);
		BOOST_CHECK(!!(Ztran.getOrigin() == Z));
		BOOST_CHECK(!!(k * X == Ztran.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * Y == Ztran.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == Ztran.vectorToGlobal(k * Z)));

		Ztran.updateLocalBase(X); // no change
		BOOST_CHECK(!!(k * X == Ztran.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * Y == Ztran.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == Ztran.vectorToGlobal(k * Z)));

		Ztran.updateLocalBase(Y); // Pi/2 rotation
		BOOST_CHECK(!!(k * Y == Ztran.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(-k * X == Ztran.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == Ztran.vectorToGlobal(k * Z)));

		// translation over X, changes center of rotation
		CylindricalCoordinateSystem Xtran(model, X, X, Y);
		BOOST_CHECK(!!(k * X == Xtran.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * Y == Xtran.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == Xtran.vectorToGlobal(k * Z)));

		Xtran.updateLocalBase(2 * X); // no change
		BOOST_CHECK(!!(k * X == Xtran.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * Y == Xtran.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == Xtran.vectorToGlobal(k * Z)));

		Xtran.updateLocalBase(Y); // 3*Pi/4 rotation because center of rotation changed
		BOOST_CHECK(!!(k * (-1 * X + Y) / sqrt(2) == Xtran.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * (-1 * X - 1 * Y) / sqrt(2) == Xtran.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == Xtran.vectorToGlobal(k * Z)));

		// re-orientation of X by rotation over Z (45°)
		CylindricalCoordinateSystem Xrot45(model, O, X + Y, Y);
		BOOST_CHECK(!!(k * (X + Y) / sqrt(2) == Xrot45.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * (Y - X) / sqrt(2) == Xrot45.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == Xrot45.vectorToGlobal(k * Z)));

		Xrot45.updateLocalBase(X + Y); // no change
		BOOST_CHECK(!!(k * (X + Y) / sqrt(2) == Xrot45.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(k * (Y - X) / sqrt(2) == Xrot45.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == Xrot45.vectorToGlobal(k * Z)));

		Xrot45.updateLocalBase(Y); // Pi/2 rotation
		BOOST_CHECK(!!(k * Y == Xrot45.vectorToGlobal(k * X)));
		BOOST_CHECK(!!(-k * X == Xrot45.vectorToGlobal(k * Y)));
		BOOST_CHECK(!!(k * Z == Xrot45.vectorToGlobal(k * Z)));

	}

}
