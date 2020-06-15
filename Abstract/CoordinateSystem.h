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

class Mesh;

class CoordinateSystem: public Identifiable<CoordinateSystem> {
    friend std::ostream& operator<<(std::ostream&, const CoordinateSystem&);
public:
    static constexpr int GLOBAL_COORDINATE_SYSTEM_ID = 0;
    static const Reference<CoordinateSystem> GLOBAL_COORDINATE_SYSTEM;
    enum class Type {
        ABSOLUTE,
        RELATIVE
    };
    enum class CoordinateType {
        CARTESIAN,
        CYLINDRICAL,
        SPHERICAL,
        VECTOR
    };
    protected:
    const Mesh &mesh;
    public:
    const Type type;
    const CoordinateType coordType;

protected:
    VectorialValue origin; /** local origin */
    VectorialValue ex; /** local X axis */
    VectorialValue ey; /** local Y axis */
    const Reference<CoordinateSystem> rcs; /** Identification of a coordinate system that is defined independently from this coordinate system. */
    VectorialValue ez; /** internally computed as cross product of ex and ey */
    bool isVirtual = false;
    std::vector<int> nodesId;
    boost::numeric::ublas::matrix<double> inverseMatrix;

public:
    static const std::string name;
    static const std::map<Type, std::string> stringByType;
    static const std::map<CoordinateType, std::string> stringByCoordinateSystemType;
    inline VectorialValue getOrigin() const noexcept {return origin;};
    inline VectorialValue getEx() const noexcept {return ex;};
    inline VectorialValue getEy() const noexcept {return ey;};
    inline VectorialValue getEz() const noexcept {return ez;};

    protected:
    CoordinateSystem(const Mesh&, Type, CoordinateType, const VectorialValue origin, const VectorialValue ex,
            const VectorialValue ey, const Reference<CoordinateSystem> rcs = GLOBAL_COORDINATE_SYSTEM, int original_id = NO_ORIGINAL_ID);
    public:
    virtual void updateLocalBase(const VectorialValue&) {
    }
    virtual void build(){
    }
    /**
     *  Translate a position (x,y,z) expressed in this local Coordinate system,
     *   to its global counterpart.
     */
    virtual VectorialValue positionToGlobal(const VectorialValue&) const = 0;
    /**
     *  Translate a vector, expressed in this local Coordinate system,
     *   to its global counterpart. Warning, it does not take the origin into
     *   account, so do NOT use this to convert coordinates.
     */
    virtual VectorialValue vectorToGlobal(const VectorialValue&) const = 0;
    /**
     *  Translate a vector, expressed in the global Coordinate system,
     *   to its local counterpart. Warning, it does not take the origin into
     *   account, so do NOT use this to convert coordinates.
     */
    virtual VectorialValue vectorToLocal(const VectorialValue&) const = 0;
    /**
     *  Compute the Euler Angles (PSI,THETA,PHI) around the axes (OZ, OY, OX)
     *  of the reference coordinate system RCS. If no rcsPos is provided, the global
     *  coordinate system is used.
     * TODO LD use internal RCS ?
     */
    virtual VectorialValue getEulerAnglesIntrinsicZYX(const CoordinateSystem *cs = nullptr) const;
    virtual std::unique_ptr<CoordinateSystem> clone() const = 0;
};

class CartesianCoordinateSystem: public CoordinateSystem {
public:
    CartesianCoordinateSystem(const Mesh&, const VectorialValue& origin = VectorialValue::O, const VectorialValue& ex = VectorialValue::X,
            const VectorialValue& ey = VectorialValue::Y, const Reference<CoordinateSystem> rcs = GLOBAL_COORDINATE_SYSTEM, int original_id = NO_ORIGINAL_ID);
    CartesianCoordinateSystem(const Mesh&, int nO, int nZ, int nXZ, const Reference<CoordinateSystem> rcs = GLOBAL_COORDINATE_SYSTEM, int original_id = NO_ORIGINAL_ID);

    /**
     * Build (O, ex,ey,ez) from nodesId.
     *    - nodesId[0] is the Origin point O.
     *    - nodesId[1] is the Z point: ez=OZ.
     *    - nodesId[2] is in the x-z plane.
     */
    void build() override;
    VectorialValue positionToGlobal(const VectorialValue&) const override;
    VectorialValue vectorToGlobal(const VectorialValue&) const override;
    VectorialValue vectorToLocal(const VectorialValue&) const override;
    std::unique_ptr<CoordinateSystem> clone() const override;
};

/**
 *  A Cartesian Coordinate System computed from a element-specific orientation.
 *  Axis are  ex=OX, V is in the Oxy plane, in direct sense, and ez=OX^V.
 *  For more information, see, among others, the CBAR entry of the NASTRAN manual.
 *
 */
class OrientationCoordinateSystem: public CoordinateSystem {
public:
    OrientationCoordinateSystem(const Mesh&, const int nO, const int nX,
            const int nV, const Reference<CoordinateSystem> rcs = GLOBAL_COORDINATE_SYSTEM, int original_id = NO_ORIGINAL_ID);
    OrientationCoordinateSystem(const Mesh&, const int nO, const int nX,
                const VectorialValue v, const Reference<CoordinateSystem> rcs = GLOBAL_COORDINATE_SYSTEM, int original_id = NO_ORIGINAL_ID);

protected:
    VectorialValue v; /**< Orientation vector */

public:
    void build() override; /**< Build (O,ex,ey,ez) from the node and v */
    VectorialValue getOrigin() const;
    VectorialValue getEx() const;
    VectorialValue getEy() const;
    VectorialValue getEz() const;
    inline VectorialValue getV() const noexcept {return v;};
    inline int getNodeO() const noexcept {return nodesId[0];}; /**< Return node Id of O (Origin point) */
    inline int getNodeX() const noexcept {return nodesId[1];}; /**< Return node Id of X (X axis is built with ex=OX) */
    inline int getNodeV() const noexcept {return nodesId[2];}; /**< Return node Id of V point (alternate method to supply v: v= OV) */
    bool operator==(const OrientationCoordinateSystem&) const noexcept;  /**< Equal operator */

    VectorialValue positionToGlobal(const VectorialValue&) const override;
    VectorialValue vectorToGlobal(const VectorialValue&) const override;
    VectorialValue vectorToLocal(const VectorialValue&) const override;
    std::unique_ptr<CoordinateSystem> clone() const override;
};


class CylindricalCoordinateSystem: public CoordinateSystem {
    VectorialValue ur;
    VectorialValue utheta;
public:
    CylindricalCoordinateSystem(const Mesh&, const VectorialValue origin, const VectorialValue ex,
            const VectorialValue ey, const Reference<CoordinateSystem> rcs = GLOBAL_COORDINATE_SYSTEM, int original_id = NO_ORIGINAL_ID);

    /**
     *  Compute the local cylindrical base (ur, utheta, uz) corresponding to point.
     *   Point must be expressed in the reference cartesian coordinate system.
     */
    void updateLocalBase(const VectorialValue & point) override;
    /**
     *  Translate a position expressed in this coordinate system (r, theta, z),
     *   to its global counterpart (x,y,z). theta is expressed in degrees.
     */
    VectorialValue positionToGlobal(const VectorialValue&) const override;
    /**
     *  Translate a vector, expressed in this coordinate system (ur, utheta, uz),
     *   to its global counterpart. Warning, it does not take the origin into
     *   account, so do NOT use this to convert coordinates.
     */
    VectorialValue vectorToGlobal(const VectorialValue&) const override;
    VectorialValue vectorToLocal(const VectorialValue&) const override;

    /**
     *  Compute the Euler Angles (PSI,THETA,PHI) of the local base,
     *  around the axes (OZ, OY, OX) of the coordinate system RCS.
     *  If no rcsPos is provided, the global coordinate system is used.
     */
    VectorialValue getLocalEulerAnglesIntrinsicZYX(const CoordinateSystem *rcs = nullptr) const;


    std::unique_ptr<CoordinateSystem> clone() const override;
};

class SphericalCoordinateSystem: public CoordinateSystem {
    public:
    SphericalCoordinateSystem(const Mesh&, const VectorialValue origin, const VectorialValue ex,
            const VectorialValue ey, const Reference<CoordinateSystem> rcs = GLOBAL_COORDINATE_SYSTEM, int original_id = NO_ORIGINAL_ID);
    /**
     *  Compute the local spheric base (ur, utheta, uphi) corresponding to point.
     *   Point must be expressed in the reference cartesian coordinate system.
     */
    void updateLocalBase(const VectorialValue& point) override;
    /**
     *  Not done
     */
    VectorialValue positionToGlobal(const VectorialValue&) const override;
    /**
     *  Translate a vector, expressed in this coordinate system (ur, utheta, uphi),
     *   to its global counterpart. Warning, it does not take the origin into
     *   account, so do NOT use this to convert coordinates.
     */
    VectorialValue vectorToGlobal(const VectorialValue&) const override;
    VectorialValue vectorToLocal(const VectorialValue&) const override;
    std::unique_ptr<CoordinateSystem> clone() const override;
};


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
    friend Mesh;
    friend CoordinateSystem;
    static pos_t cs_next_position;          /**< Static token for the next CS Position. */
    //static constexpr int UNAVAILABLE_ID = -INT_MAX;
    static constexpr pos_t UNAVAILABLE_POSITION = Globals::UNAVAILABLE_POS;
    const LogLevel logLevel;
    std::map<pos_t, Reference<CoordinateSystem>> refByPosition;  /**< A map < Position, Original Id > to keep track of coordinate System. */

    /**
     * Reserve a CS position given a user id (input model id).
     */
    pos_t reserve(const Reference<CoordinateSystem>);
    const Mesh& mesh;
    CoordinateSystemStorage(const Mesh&, LogLevel logLevel);
public:
    std::map<Reference<CoordinateSystem>, std::shared_ptr<CoordinateSystem>> coordinateSystemByRef;

    /** Find the Position related to the input user id.
     *  Return UNAVAILABLE_POSITION if nothing is found.
     */
    pos_t findPosition(const Reference<CoordinateSystem>) const;

    /** Update the maps with the data (id, original_id)
     *  of this CS.
     *  Return the corresponding Position.
     */
    pos_t add(const CoordinateSystem& coordinateSystem);
    std::shared_ptr<CoordinateSystem> find(Reference<CoordinateSystem> csref) const;
    std::shared_ptr<CoordinateSystem> findByPosition(pos_t cspos) const;
};



} /* namespace vega */
#endif /* COORDINATESYSTEM_H_ */

