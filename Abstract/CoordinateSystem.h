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
		ORIENTATION,
		SPHERICAL,
		UNKNOWN
	};
	protected:
	const Model &model;
	public:
	const Type type;

	protected:
	VectorialValue origin;
	VectorialValue ex;
	VectorialValue ey;
	VectorialValue ez;

	public:
	static const string name;
	static const map<Type, string> stringByType;
	inline const VectorialValue getOrigin() const {return origin;}
	inline const VectorialValue getEx() const {return ex;}
	inline const VectorialValue getEy() const {return ey;}
	inline const VectorialValue getEz() const {return ez;}

	protected:
	CoordinateSystem(const Model&, Type, const VectorialValue origin, const VectorialValue ex,
			const VectorialValue ey, int original_id = NO_ORIGINAL_ID);
	public:
	virtual void updateLocalBase(const VectorialValue&) {
	}
	virtual void build(){
	}
	/**
	 *  Translate a vector, expressed in this local Coordinate system,
	 *   to its global counterpart. Warning, it does not take the origin into
	 *   account, so do NOT use this to convert coordinates.
	 **/
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

/**
 *  A Cartesian Coordinate System computed from a element-specific orientation.
 *  Axis are  ex=OX, V is in the Oxy plane, in direct sense, and ez=OX^V.
 *  For more information, see, among others, the CBAR entry of the NASTRAN manual.
 **/
class OrientationCoordinateSystem: public CoordinateSystem {
public:
	OrientationCoordinateSystem(const Model&, const int nO, const int nX,
			const int nV, int original_id = NO_ORIGINAL_ID);
	OrientationCoordinateSystem(const Model&, const int nO, const int nX,
				const VectorialValue v, int original_id = NO_ORIGINAL_ID);

	const int nO;  /**< Id of Origin Node **/
	const int nX;  /**< X axis is built with ex=OX **/
	VectorialValue v; /**< Orientation vector **/
	const int nV;  /**< Alternate method to supply v: v= OV **/

	void build() override; /**< Build (O,ex,ey,ez) from the node and v **/
	const VectorialValue getOrigin() const;
	const VectorialValue getEx() const;
	const VectorialValue getEy() const;
	const VectorialValue getEz() const;

	const VectorialValue vectorToGlobal(const VectorialValue&) const override;
	std::shared_ptr<CoordinateSystem> clone() const override;
protected:
	bool isVirtual=true;

};


class CylindricalCoordinateSystem: public CoordinateSystem {
	VectorialValue ur;
	VectorialValue utheta;
	public:
	CylindricalCoordinateSystem(const Model&, const VectorialValue origin, const VectorialValue ex,
			const VectorialValue ey, int original_id = NO_ORIGINAL_ID);

	/**
	 *  Compute the local cylindrical base (ur, utheta, uz) corresponding to point.
	 *   Point must be expressed in the reference cartesian coordinate system.
	 **/
	void updateLocalBase(const VectorialValue & point);
	/**
	 *  Translate a vector, expressed in this coordinate system (ur, utheta, uz),
	 *   to its global counterpart. Warning, it does not take the origin into
	 *   account, so do NOT use this to convert coordinates.
	 **/
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
	/**
	 *  Compute the local spheric base (ur, utheta, uphi) corresponding to point.
	 *   Point must be expressed in the reference cartesian coordinate system.
	 **/
	void updateLocalBase(const VectorialValue& point);
	/**
	 *  Translate a vector, expressed in this coordinate system (ur, utheta, uphi),
	 *   to its global counterpart. Warning, it does not take the origin into
	 *   account, so do NOT use this to convert coordinates.
	 **/
	const VectorialValue vectorToGlobal(const VectorialValue&) const override;
	std::shared_ptr<CoordinateSystem> clone() const override;
};


} /* namespace vega */
#endif /* COORDINATESYSTEM_H_ */

