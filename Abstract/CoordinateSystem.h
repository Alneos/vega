/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * CoordinateSystem.h
 *
 *  Created on: Oct 19, 2013
 *      Author: devel
 */

#ifndef COORDINATESYSTEM_H_
#define COORDINATESYSTEM_H_

#include <climits>

#include "Utility.h"
#include "Object.h"
#include <map>

using namespace std;

namespace vega {

class Model;

class CoordinateSystem: public Identifiable<CoordinateSystem> {
	friend std::ostream& operator<<(std::ostream&, const CoordinateSystem&);
	public:
	static const int GLOBAL_COORDINATE_SYSTEM_ID = 0;
	enum Type {
		CARTESIAN,
		CYLINDRICAL,
		SPHERICAL,
		UNKNOWN
	};
	protected:
	const Model &model;
	public:
	const Type type;
	static const string name;
	static const map<Type, string> stringByType;
	const VectorialValue origin;
	const VectorialValue ex;
	const VectorialValue ey;
	const VectorialValue ez;
	protected:
	CoordinateSystem(const Model&, Type, const VectorialValue origin, const VectorialValue ex,
			const VectorialValue ey, int original_id = NO_ORIGINAL_ID);
	public:
	virtual void updateLocalBase(const VectorialValue&) {
	}
	virtual const VectorialValue vectorToGlobal(const VectorialValue&) const = 0;
	virtual const VectorialValue getEulerAnglesIntrinsicZYX() const;
	virtual std::shared_ptr<CoordinateSystem> clone() const = 0;
};

class CartesianCoordinateSystem: public CoordinateSystem {
public:
	CartesianCoordinateSystem(const Model&, const VectorialValue& origin, const VectorialValue& ex,
			const VectorialValue& ey, int original_id = NO_ORIGINAL_ID);

	const VectorialValue vectorToGlobal(const VectorialValue&) const override;
	std::shared_ptr<CoordinateSystem> clone() const override;
};

class CylindricalCoordinateSystem: public CoordinateSystem {
	VectorialValue ur;
	VectorialValue utheta;
	public:
	CylindricalCoordinateSystem(const Model&, const VectorialValue origin, const VectorialValue ex,
			const VectorialValue ey, int original_id = NO_ORIGINAL_ID);

	void updateLocalBase(const VectorialValue&);
	const VectorialValue vectorToGlobal(const VectorialValue&) const override;
	std::shared_ptr<CoordinateSystem> clone() const override;
};

class SphericalCoordinateSystem: public CoordinateSystem {
	VectorialValue ur;
	VectorialValue utheta;
	VectorialValue uphi;
	public:
	SphericalCoordinateSystem(const Model&, const VectorialValue origin, const VectorialValue ex,
			const VectorialValue ey, int original_id = NO_ORIGINAL_ID);

	void updateLocalBase(const VectorialValue&);
	const VectorialValue vectorToGlobal(const VectorialValue&) const override;
	std::shared_ptr<CoordinateSystem> clone() const override;
};

class VectY;

/**
 * Permit to orient a cell
 */
class Orientation : public Identifiable<Orientation> {
protected:
	Orientation() {
	}
	;
	public:
	virtual std::shared_ptr<Orientation> clone() const = 0;
	virtual VectY toVectY() const = 0;
	bool operator==(const Orientation&) const;
	virtual ~Orientation() {
	}
};

class VectY: public Orientation {
public:
	const VectorialValue vectY;
	VectY(double x, double y, double z);
	VectY(const VectorialValue);
	VectY toVectY() const;
	std::shared_ptr<Orientation> clone() const;
};

class TwoNodesOrientation: public Orientation {
	const Model& model;
	public:
	const int node_id1, node_id2;
	TwoNodesOrientation(const Model&, int node_id1, int node_id2);
	VectY toVectY() const;
	std::shared_ptr<Orientation> clone() const;
};

class RotationAngleX: public Orientation {
	double theta;
	RotationAngleX(const Model&, double theta);
	VectY toVectY() const;
	std::shared_ptr<Orientation> clone() const;
};

} /* namespace vega */
#endif /* COORDINATESYSTEM_H_ */

