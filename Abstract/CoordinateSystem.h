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
 * CoordinateSystem.h
 *
 *  Created on: Oct 19, 2013
 *      Author: devel
 */

#ifndef COORDINATESYSTEM_H_
#define COORDINATESYSTEM_H_

#include <climits>

#include "Value.h"
#include "Object.h"
#include <map>
#include "ConfigurationParameters.h"
#include <boost/numeric/ublas/matrix.hpp>

namespace vega {

class Model;

class CoordinateSystem: public Identifiable<CoordinateSystem> {
    friend std::ostream& operator<<(std::ostream&, const CoordinateSystem&);
    public:
    static constexpr int GLOBAL_COORDINATE_SYSTEM_ID = 0;
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
    VectorialValue origin; /** local origin */
    VectorialValue ex; /** local X axis */
    VectorialValue ey; /** local Y axis */
    const int rcs; /** Identification number of a coordinate system that is defined independently from this coordinate system. */
    VectorialValue ez; /** internally computed as cross product of ex and ey */
    bool isVirtual=false;
    std::vector<int> nodesId;
    boost::numeric::ublas::matrix<double> inverseMatrix;

    public:
    static const std::string name;
    static const std::map<Type, std::string> stringByType;
    inline const VectorialValue getOrigin() const {return origin;};
    inline const VectorialValue getEx() const {return ex;};
    inline const VectorialValue getEy() const {return ey;};
    inline const VectorialValue getEz() const {return ez;};

    protected:
    CoordinateSystem(const Model&, Type, const VectorialValue origin, const VectorialValue ex,
            const VectorialValue ey, const int rcs = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, int original_id = NO_ORIGINAL_ID);
    public:
    virtual void updateLocalBase(const VectorialValue&) {
    }
    virtual void build(){
    }
    /**
     *  Translate a position (x,y,z) expressed in this local Coordinate system,
     *   to its global counterpart.
     */
    virtual const VectorialValue positionToGlobal(const VectorialValue&) const = 0;
    /**
     *  Translate a vector, expressed in this local Coordinate system,
     *   to its global counterpart. Warning, it does not take the origin into
     *   account, so do NOT use this to convert coordinates.
     */
    virtual const VectorialValue vectorToGlobal(const VectorialValue&) const = 0;
    /**
     *  Translate a vector, expressed in the global Coordinate system,
     *   to its local counterpart. Warning, it does not take the origin into
     *   account, so do NOT use this to convert coordinates.
     */
    virtual const VectorialValue vectorToLocal(const VectorialValue&) const = 0;
    /**
     *  Compute the Euler Angles (PSI,THETA,PHI) around the axes (OZ, OY, OX)
     *  of the reference coordinate system RCS. If no rcs is provided, the global
     *  coordinate system is used.
     * TODO LD use internal RCS ?
     */
    virtual const VectorialValue getEulerAnglesIntrinsicZYX(const CoordinateSystem *cs = nullptr) const;
    virtual std::shared_ptr<CoordinateSystem> clone() const = 0;
};

class CartesianCoordinateSystem: public CoordinateSystem {
public:
    CartesianCoordinateSystem(const Model&, const VectorialValue& origin = VectorialValue::O, const VectorialValue& ex = VectorialValue::X,
            const VectorialValue& ey = VectorialValue::Y, const int rcs = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, int original_id = NO_ORIGINAL_ID);
    CartesianCoordinateSystem(const Model& model, int nO, int nZ, int nXZ, const int rcs = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, int original_id = NO_ORIGINAL_ID);

    /**
     * Build (O, ex,ey,ez) from nodesId.
     *    - nodesId[0] is the Origin point O.
     *    - nodesId[1] is the Z point: ez=OZ.
     *    - nodesId[2] is in the x-z plane.
     */
    void build() override;
    const VectorialValue positionToGlobal(const VectorialValue&) const override;
    const VectorialValue vectorToGlobal(const VectorialValue&) const override;
    const VectorialValue vectorToLocal(const VectorialValue&) const override;
    std::shared_ptr<CoordinateSystem> clone() const override;
};

/**
 *  A Cartesian Coordinate System computed from a element-specific orientation.
 *  Axis are  ex=OX, V is in the Oxy plane, in direct sense, and ez=OX^V.
 *  For more information, see, among others, the CBAR entry of the NASTRAN manual.
 *
 */
class OrientationCoordinateSystem: public CoordinateSystem {
public:
    OrientationCoordinateSystem(const Model&, const int nO, const int nX,
            const int nV, const int rcs = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, int original_id = NO_ORIGINAL_ID);
    OrientationCoordinateSystem(const Model&, const int nO, const int nX,
                const VectorialValue v, const int rcs = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, int original_id = NO_ORIGINAL_ID);

protected:
    VectorialValue v; /**< Orientation vector */

public:
    void build() override; /**< Build (O,ex,ey,ez) from the node and v */
    const VectorialValue getOrigin() const;
    const VectorialValue getEx() const;
    const VectorialValue getEy() const;
    const VectorialValue getEz() const;
    inline const VectorialValue getV() const {return v;};
    inline int getNodeO() const {return nodesId[0];}; /**< Return node Id of O (Origin point) */
    inline int getNodeX() const {return nodesId[1];}; /**< Return node Id of X (X axis is built with ex=OX) */
    inline int getNodeV() const {return nodesId[2];}; /**< Return node Id of V point (alternate method to supply v: v= OV) */
    bool operator==(const OrientationCoordinateSystem&) const;  /**< Equal operator */

    const VectorialValue positionToGlobal(const VectorialValue&) const override;
    const VectorialValue vectorToGlobal(const VectorialValue&) const override;
    const VectorialValue vectorToLocal(const VectorialValue&) const override;
    std::shared_ptr<CoordinateSystem> clone() const override;
};


class CylindricalCoordinateSystem: public CoordinateSystem {
    VectorialValue ur;
    VectorialValue utheta;
    public:
    CylindricalCoordinateSystem(const Model&, const VectorialValue origin, const VectorialValue ex,
            const VectorialValue ey, const int rcs = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, int original_id = NO_ORIGINAL_ID);

    /**
     *  Compute the local cylindrical base (ur, utheta, uz) corresponding to point.
     *   Point must be expressed in the reference cartesian coordinate system.
     */
    void updateLocalBase(const VectorialValue & point);
    /**
     *  Translate a position expressed in this coordinate system (r, theta, z),
     *   to its global counterpart (x,y,z). theta is expressed in degrees.
     */
    const VectorialValue positionToGlobal(const VectorialValue&) const override;
    /**
     *  Translate a vector, expressed in this coordinate system (ur, utheta, uz),
     *   to its global counterpart. Warning, it does not take the origin into
     *   account, so do NOT use this to convert coordinates.
     */
    const VectorialValue vectorToGlobal(const VectorialValue&) const override;
    const VectorialValue vectorToLocal(const VectorialValue&) const override;

    /**
     *  Compute the Euler Angles (PSI,THETA,PHI) of the local base,
     *  around the axes (OZ, OY, OX) of the coordinate system RCS.
     *  If no rcs is provided, the global coordinate system is used.
     */
    const VectorialValue getLocalEulerAnglesIntrinsicZYX(const CoordinateSystem *rcs = nullptr) const;


    std::shared_ptr<CoordinateSystem> clone() const override;
};

//class SphericalCoordinateSystem: public CoordinateSystem {
//    VectorialValue ur;
//    VectorialValue utheta;
//    VectorialValue uphi;
//    public:
//    SphericalCoordinateSystem(const Model&, const VectorialValue origin, const VectorialValue ex,
//            const VectorialValue ey, int original_id = NO_ORIGINAL_ID);
//    /**
//     *  Compute the local spheric base (ur, utheta, uphi) corresponding to point.
//     *   Point must be expressed in the reference cartesian coordinate system.
//     */
//    void updateLocalBase(const VectorialValue& point);
//    /**
//     *  Not done
//     */
//    const VectorialValue positionToGlobal(const VectorialValue&) const override;
//    /**
//     *  Translate a vector, expressed in this coordinate system (ur, utheta, uphi),
//     *   to its global counterpart. Warning, it does not take the origin into
//     *   account, so do NOT use this to convert coordinates.
//     */
//    const VectorialValue vectorToGlobal(const VectorialValue&) const override;
//    const VectorialValue vectorToLocal(const VectorialValue&) const override;
//    std::shared_ptr<CoordinateSystem> clone() const override;
//};


/** This class allows the model to store Coordinate System BEFORE they are created.
 *  Each CS, user-defined or model-defined (Orientation) get a unique Position number
 *  when it's created or reserved (whatever comes first).
 *  This Position number is stored into CellData and NodeData, for further use.
 *
 *  When the CS is created, we associate its model ID (Vega Identifiable number)
 *  and user ID (provided by the input file) to the POSITION.
 *  When the CS is reserved, we associate its user ID to the POSITION.
 *  */
class CoordinateSystemStorage final {
private:
    friend Model;
    friend CoordinateSystem;
    static int cs_next_position;          /**< Static token for the next CS Position. */
    static constexpr int UNAVAILABLE_ID = -INT_MAX;
    static constexpr int UNAVAILABLE_POSITION = -INT_MAX;
    const LogLevel logLevel;
    std::map<int, int> modelIdByPosition; /**< A map < Position, VEGA Id > to keep track of coordinate System. */
    std::map<int, int> userIdByPosition;  /**< A map < Position, Original Id > to keep track of coordinate System. */

    /**
     * Reserve a CS position given a user id (input model id).
     */
    int reserve(int user_id);
public:
    Model* model;
    CoordinateSystemStorage(Model* model, LogLevel logLevel);
    /** Find the Position related to the input user id.
     *  Return UNAVAILABLE_POSITION if nothing is found.
     */
    int findPositionByUserId(int user_id) const;
    /** Find the Position related to the input model id.
     *  Return UNAVAILABLE_POSITION if nothing is found.
     */
    int findPositionById(int model_id) const;
    /** Update the maps with the data (id, original_id)
     *  of this CS.
     *  Return the corresponding Position.
     */
    int add(const CoordinateSystem& coordinateSystem);
    int getId(int cpos) const;
    //bool validate() const;
};



} /* namespace vega */
#endif /* COORDINATESYSTEM_H_ */

