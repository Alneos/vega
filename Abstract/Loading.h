/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Loading.h
 *
 *  Created on: Aug 24, 2013
 *      Author: devel
 */

#ifndef LOADING_H_
#define LOADING_H_

#include "MeshComponents.h"
#include "Object.h"
#include "BoundaryCondition.h"
#include "Dof.h"
#include "Value.h"
#include "CoordinateSystem.h"
#include "Reference.h"
#include <climits>

namespace vega {

class Model;

/**
 * Base class for all Loadings
 */
class Loading: public Identifiable<Loading>, public BoundaryCondition {
private:
	friend std::ostream &operator<<(std::ostream&, const Loading&);    //output
public:
	enum Type {
		DYNAMIC_EXCITATION,
		FORCE_SURFACE,
		GRAVITY,
		ROTATION,
		FORCE_LINE,
		NODAL_FORCE,
		NORMAL_PRESSION_FACE,
		COMBINED_LOADING,
		INITIAL_TEMPERATURE
	};
	enum ApplicationType {
		NODE,
		ELEMENT,
		NONE
	};
protected:
	const Model& model;
	Loading(const Model&, Loading::Type, Loading::ApplicationType, const int original_id =
			NO_ORIGINAL_ID, int coordinateSystemId = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
public:
	const Type type;
	static const std::string name;
	static const std::map<Type, std::string> stringByType;
	const ApplicationType applicationType;
	const Reference<CoordinateSystem> coordinateSystem_reference;
	bool hasCoordinateSystem() const {
		return coordinateSystem_reference.original_id
				!= CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID;
	}
	virtual std::shared_ptr<Loading> clone() const = 0;
	virtual void scale(const double factor) {
		std::cerr << "loading scale(" << factor << ") used but not implemented" << std::endl;
		throw std::logic_error("loading scale() used but not implemented");
	}
	bool validate() const override;
};

/**
 * Set of loadings that are often referenced by an analysis.
 */
class LoadSet: public Identifiable<LoadSet> {
private:
	const Model& model;
	friend std::ostream &operator<<(std::ostream&, const LoadSet&);
public:
	enum Type {
		LOAD,
		DLOAD,
		EXCITEID,
		ALL
	};
	LoadSet(const Model&, Type type = LOAD, int original_id = NO_ORIGINAL_ID);
	static constexpr int COMMON_SET_ID = 0;
	std::vector<std::pair<Reference<LoadSet>, double>> embedded_loadsets;
	const Type type;
	static const std::string name;
	static const std::map<Type, std::string> stringByType;
	int size() const;
	const std::set<std::shared_ptr<Loading> > getLoadings() const;
	const std::set<std::shared_ptr<Loading> > getLoadingsByType(Loading::Type) const;
	bool validate() const override;
	std::shared_ptr<LoadSet> clone() const;
	bool hasFunctions() const;
	//bool operator<(const LoadSet &rhs) const;
};

/**
 * Represent loading applied on nodes
 */
class NodeLoading: public Loading, public NodeContainer {
protected:
	NodeLoading(const Model&, Loading::Type, const int original_id = NO_ORIGINAL_ID,
			int coordinateSystemId = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
public:
	std::set<int> nodePositions() const override final;
	SpaceDimension getLoadingDimension() const {
		return SpaceDimension::DIMENSION_0D;
	}
};

class Gravity: public Loading {
private:
	double acceleration;
	const VectorialValue direction;
public:
	const VectorialValue getDirection() const;
	/**
	 * Get gravity acceleration (in m.s^{-2}).
	 */
	double getAcceleration() const;
	/**
	 * acceleration vector is given by acceleration * direction.
	 */
	const VectorialValue getAccelerationVector() const;
	/**
	 * Get gravity acceleration scale (no units), useful if density material is expressed as a force density.
	 */
	double getAccelerationScale() const;
	/**
	 * acceleration vector is given by acceleration * direction.
	 */
	Gravity(const Model&, double acceleration, const VectorialValue& direction,
			const int original_id = NO_ORIGINAL_ID);
	const DOFS getDOFSForNode(int nodePosition) const override;
	std::set<int> nodePositions() const override;
	std::shared_ptr<Loading> clone() const override;
	void scale(const double factor) override;
	bool ineffective() const override;
};

class Rotation: public Loading {
protected:
	Rotation(const Model& model, const int original_id = NO_ORIGINAL_ID);
public:
	/**
	 * Get rotation speed (in rad/s).
	 */
	virtual double getSpeed() const = 0;
	/**
	 * Get rotation axis.
	 */
	virtual const VectorialValue getAxis() const = 0;

	/**
	 * Get rotation center.
	 */
	virtual const VectorialValue getCenter() const = 0;
	const DOFS getDOFSForNode(int nodePosition) const override;
	std::set<int> nodePositions() const override;
	bool ineffective() const override;
};

class RotationCenter: public Rotation {
	double speed;
	const VectorialValue axis;
	const VectorialValue center;
public:
	RotationCenter(const Model& model, double speed, double center_x, double center_y,
			double center_z, double axis_x, double axis_y, double axis_z, const int original_id =
					NO_ORIGINAL_ID);
	double getSpeed() const;
	const VectorialValue getAxis() const;
	const VectorialValue getCenter() const;
	std::shared_ptr<Loading> clone() const override;
	void scale(const double factor) override;
};

class RotationNode: public Rotation {
	double speed;
	const VectorialValue axis;
	const int node_position;
public:
	RotationNode(const Model& model, double speed, const int node_id, double axis_x, double axis_y,
			double axis_z, const int original_id = NO_ORIGINAL_ID);
	double getSpeed() const;
	const VectorialValue getAxis() const;
	const VectorialValue getCenter() const;
	std::shared_ptr<Loading> clone() const override;
	void scale(const double factor) override;
};

/**
 * Base class for all forces applied on nodes
 */
class NodalForce: public NodeLoading {
public:
	NodalForce(const Model&, const VectorialValue& force, const VectorialValue& moment,
			const int original_id = NO_ORIGINAL_ID, int coordinateSystemId =
					CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
	NodalForce(const Model&, double fx, double fy = 0, double fz = 0, double mx = 0,
			double my = 0, double mz = 0, const int original_id = NO_ORIGINAL_ID,
			int coordinateSystemId = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
protected:
	//NodalForce(const Model&, const int original_id = NO_ORIGINAL_ID,
	//		int coordinateSystemId = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
	const VectorialValue localToGlobal(int nodePosition, const VectorialValue&) const;
	VectorialValue force;
	VectorialValue moment;
public:
	virtual const VectorialValue getForceInGlobalCS(const int nodePosition) const;
	virtual const VectorialValue getMomentInGlobalCS(const int nodePosition) const;
	const DOFS getDOFSForNode(int nodePosition) const override;
	std::shared_ptr<Loading> clone() const override;
	void scale(const double factor) override;
	bool ineffective() const override;
};

class NodalForceTwoNodes: public NodalForce {
	const int node_position1;
	const int node_position2;
	double magnitude;
public:
	NodalForceTwoNodes(const Model&, const int node1_id, const int node2_id,
			double magnitude, const int original_id = NO_ORIGINAL_ID);
	const VectorialValue getForceInGlobalCS(const int) const override;
	std::shared_ptr<Loading> clone() const;
	void scale(const double factor) override;
	bool ineffective() const override;
};

/**
 *  In this class, the vector force is defined by N1N2^N3N4.
 */
//TODO: We build three classes for Nodal Force... because we have 3 ways to define a vector. That's not good.
class NodalForceFourNodes: public NodalForce {
    const int node_position1;
    const int node_position2;
    const int node_position3;
    const int node_position4;
    double magnitude;
public:
    NodalForceFourNodes(const Model&, const int node1_id, const int node2_id,
            const int node3_id, const int node4_id, double magnitude, const int original_id = NO_ORIGINAL_ID);
    const VectorialValue getForceInGlobalCS(const int) const override;
    std::shared_ptr<Loading> clone() const;
    void scale(const double factor) override;
    bool ineffective() const override;
};

/**
 * See Nastran PLOAD
 */
class StaticPressure: public NodalForce {
    const int node_position1;
    const int node_position2;
    const int node_position3;
    const int node_position4 = Globals::UNAVAILABLE_INT;
    double magnitude;
public:
    StaticPressure(const Model&, const int node1_id, const int node2_id,
            const int node3_id, const int node4_id, double magnitude, const int original_id = NO_ORIGINAL_ID);
    const VectorialValue getForceInGlobalCS(const int) const override;
    std::shared_ptr<Loading> clone() const;
    void scale(const double factor) override;
    bool ineffective() const override;
};

/**
 * Represent loading applied on cells
 */
class ElementLoading: public Loading, public CellContainer {
protected:
	ElementLoading(const Model&, Loading::Type, const int original_id = NO_ORIGINAL_ID,
			int coordinateSystemId = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
public:
	/**
	 * Return true if all cells are of the dimension passed as parameter.
	 */
	bool cellDimensionGreatherThan(SpaceDimension dimension);
	std::set<int> nodePositions() const override final;
	virtual SpaceDimension getLoadingDimension() const = 0;
	//implement a function that tell if this force is applied to a
	//geometrical element or to a Poutre.
	bool appliedToGeometry();
};

// TODO : refactor conception of ForceSurface to have only one base class and virtual fct to generate skin get/add applications

/**
 *  Responsible of being a force applied to a surface (i.e. a Pressure over a Shell)
 */
class ForceSurface: public ElementLoading {
protected:
	VectorialValue force;
	VectorialValue moment;
public:
	ForceSurface(const Model&, const VectorialValue& force, const VectorialValue& moment,
			const int original_id = NO_ORIGINAL_ID);
	const VectorialValue getForce() const;
	const VectorialValue getMoment() const;
	const DOFS getDOFSForNode(int nodePosition) const override;
	virtual std::shared_ptr<Loading> clone() const;
	void scale(const double factor) override;
	bool ineffective() const override;
	SpaceDimension getLoadingDimension() const {
		return SpaceDimension::DIMENSION_2D;
	}
	// TODO validate : Check that all the elements are 2D
	bool validate() const override;
	virtual std::vector<int> getApplicationFace() const {
		return std::vector<int>();
	}
};

/**
 Responsible of being a ForceSurface applied to an element face, identified by two nodes
 */
class PressionFaceTwoNodes: public ForceSurface {
public:
	const int nodePosition1;
	const int nodePosition2;

	PressionFaceTwoNodes(const Model&, int nodeId1, int nodeId2, const VectorialValue& force,
			const VectorialValue& moment, const int original_id = NO_ORIGINAL_ID);
	std::vector<int> getApplicationFace() const;
	virtual std::shared_ptr<Loading> clone() const override;
};

/**
 Responsible of being a force applied to a line (i.e. a Pressure over a Bar)
 */
class ForceLine: public ElementLoading {
public:

	std::shared_ptr<NamedValue> force;
	DOF dof;
    ForceLine(const Model&, const std::shared_ptr<NamedValue> force, DOF component,
			const int original_id = NO_ORIGINAL_ID);

	virtual std::shared_ptr<Loading> clone() const override;
	const DOFS getDOFSForNode(const int nodePosition) const override;
	void scale(const double factor) override;
	bool ineffective() const override;
	SpaceDimension getLoadingDimension() const {
		return SpaceDimension::DIMENSION_1D;
	}
	bool hasFunctions() const override {
	    return force->isfunction();
	}
	bool validate() const override;

};

class NormalPressionFace: public ElementLoading {

public:
	double intensity;
	NormalPressionFace(const Model&, double intensity, const int original_id = NO_ORIGINAL_ID);
	const DOFS getDOFSForNode(const int nodePosition) const override;
	bool validate() const override;
	SpaceDimension getLoadingDimension() const {
		return SpaceDimension::DIMENSION_2D;
	}
	virtual std::shared_ptr<Loading> clone() const override;
	void scale(const double factor) override;
	bool ineffective() const override;
};

/**
 Responsible of being a frequency-dependent dynamic excitation for use in frequency response problems { P( f) } = { A} ⋅ B( f) e^i { φ ( f ) + θ – 2πfτ }.
 */
class DynamicExcitation: public Loading {
private:
    Reference<NamedValue> dynaDelay;
    Reference<NamedValue> dynaPhase;
    Reference<NamedValue> functionTableB;
    Reference<NamedValue> functionTableP;
    Reference<LoadSet> loadSet;   /**< Excitation Loadset **/
public:
    DynamicExcitation(const Model&, const Reference<NamedValue> dynaDelay, const Reference<NamedValue> dynaPhase,
            const Reference<NamedValue> functionTableB, const Reference<NamedValue> functionTableP, const Reference<LoadSet>, const int original_id =
                    NO_ORIGINAL_ID);

    std::shared_ptr<DynaPhase> getDynaDelay() const;
    std::shared_ptr<DynaPhase> getDynaPhase() const;
    std::shared_ptr<FunctionTable> getFunctionTableB() const;
    std::shared_ptr<FunctionTable> getFunctionTableP() const;
    std::shared_ptr<LoadSet> getLoadSet() const;
    const ValuePlaceHolder getFunctionTableBPlaceHolder() const;
    const ValuePlaceHolder getFunctionTablePPlaceHolder() const;
    std::set<int> nodePositions() const override;
    const DOFS getDOFSForNode(const int nodePosition) const override;
    std::shared_ptr<Loading> clone() const override;
    bool validate() const override;
    bool ineffective() const override;
};

class InitialTemperature: public NodeLoading {
protected:
	double temperature;
public:
    InitialTemperature(const Model&, double temperature, const int original_id = NO_ORIGINAL_ID);
	const DOFS getDOFSForNode(const int nodePosition) const override;
	std::shared_ptr<Loading> clone() const override;
	void scale(const double factor) override;
	bool ineffective() const override;
};

} /* namespace vega */

#endif /* LOADING_H_ */
