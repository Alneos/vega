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
	friend std::ostream &operator<<(ostream&, const Loading&);    //output
public:
	enum Type {
		DYNAMIC_EXCITATION,
		FORCE_SURFACE,
		GRAVITY,
		ROTATION,
		FORCE_LINE,
		NODAL_FORCE,
		NORMAL_PRESSION_FACE,
		COMBINED_LOADING
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
	virtual void scale(double factor) {
		cerr << "loading scale(" << factor << ") used but not implemented" << endl;
		throw logic_error("loading scale() used but not implemented");
	}
	bool validate() const override;
};

/**
 * Set of loadings that are often referenced by an analysis.
 */
class LoadSet: public Identifiable<LoadSet> {
private:
	const Model& model;
	friend ostream &operator<<(ostream&, const LoadSet&);
	public:
	enum Type {
		LOAD,
		DLOAD,
		EXCITEID,
		ALL
	};
	LoadSet(const Model&, Type type = LOAD, int original_id = NO_ORIGINAL_ID);
	static const int COMMON_SET_ID = 0;
	vector<pair<Reference<LoadSet>, double>> embedded_loadsets;
	const Type type;
	static const string name;
	static const map<Type, string> stringByType;
	int size() const;
	const set<std::shared_ptr<Loading> > getLoadings() const;
	const set<std::shared_ptr<Loading> > getLoadingsByType(Loading::Type) const;
	bool validate() const override;
	std::shared_ptr<LoadSet> clone() const;
	//bool operator<(const LoadSet &rhs) const;
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
	void scale(double factor) override;
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
	void scale(double factor) override;
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
	void scale(double factor) override;
};

/**
 * Base class for all forces applied on a single node.
 */
class NodalForce: public Loading {
public:
	NodalForce(const Model&, int node_id, const VectorialValue& force, const VectorialValue& moment,
			const int original_id = NO_ORIGINAL_ID, int coordinateSystemId =
					CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
	NodalForce(const Model&, int node_id, double fx, double fy = 0, double fz = 0, double mx = 0,
			double my = 0, double mz = 0, const int original_id = NO_ORIGINAL_ID,
			int coordinateSystemId = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
	protected:
	NodalForce(const Model&, int node_id, const int original_id = NO_ORIGINAL_ID,
			int coordinateSystemId = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
	const VectorialValue localToGlobal(const VectorialValue&) const;
	const int node_position;
	VectorialValue force;
	VectorialValue moment;
	public:
	Node getNode() const;
	virtual const VectorialValue getForce() const;
	virtual const VectorialValue getMoment() const;
	const DOFS getDOFSForNode(int nodePosition) const override;
	std::set<int> nodePositions() const override;
	std::shared_ptr<Loading> clone() const override;
	void scale(double factor) override;
	bool ineffective() const override;
};

class NodalForceTwoNodes: public NodalForce {
	const int node_position1;
	const int node_position2;
	double magnitude;
	public:
	NodalForceTwoNodes(const Model&, const int node_id, const int node1_id, const int node2_id,
			double magnitude, const int original_id = NO_ORIGINAL_ID);
	const VectorialValue getForce() const;
	std::shared_ptr<Loading> clone() const;
	void scale(double factor) override;
	bool ineffective() const override;
};

/**
 * Represent loading applied on cells
 */
class ElementLoading: public Loading, public CellContainer {
private:
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
	void scale(double factor) override;
	bool ineffective() const override;
	SpaceDimension getLoadingDimension() const {
		return SpaceDimension::DIMENSION_2D;
	}
	// TODO validate : Check that all the elements are 2D
	bool validate() const override;
	virtual vector<int> getApplicationFace() const {
		return vector<int>();
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
	vector<int> getApplicationFace() const;
	virtual std::shared_ptr<Loading> clone() const override;
};

/**
 Responsible of being a force applied to a line (i.e. a Pressure over a Bar)
 */
class ForceLine: public ElementLoading {
public:

	VectorialValue force;
	VectorialValue moment;
	ForceLine(const Model&, const VectorialValue& force, const VectorialValue& moment,
			const int original_id = NO_ORIGINAL_ID);

	virtual std::shared_ptr<Loading> clone() const override;
	const DOFS getDOFSForNode(int nodePosition) const override;
	void scale(double factor) override;
	bool ineffective() const override;
	SpaceDimension getLoadingDimension() const {
		return SpaceDimension::DIMENSION_1D;
	}
	bool validate() const override;

};

class NormalPressionFace: public ElementLoading {

public:
	double intensity;
	NormalPressionFace(const Model&, double intensity, const int original_id = NO_ORIGINAL_ID);
	const DOFS getDOFSForNode(int nodePosition) const override;
	bool validate() const override;
	SpaceDimension getLoadingDimension() const {
		return SpaceDimension::DIMENSION_2D;
	}
	virtual std::shared_ptr<Loading> clone() const override;
	void scale(double factor) override;
	bool ineffective() const override;
};

/**
 Responsible of being a frequency-dependent dynamic excitation for use in frequency response problems { P( f) } = { A} ⋅ B( f) e^i { φ ( f ) + θ – 2πfτ }.
 */
class DynamicExcitation: public Loading {
private:
	Reference<Value> dynaPhase;
	Reference<Value> functionTableB;
	Reference<LoadSet> loadSet;
	public:
	DynamicExcitation(const Model&, const Reference<Value> dynaPhase,
			const Reference<Value> functionTableB, const Reference<LoadSet>, const int original_id =
					NO_ORIGINAL_ID);

	std::shared_ptr<DynaPhase> getDynaPhase() const;
	std::shared_ptr<FunctionTable> getFunctionTableB() const;
	std::shared_ptr<LoadSet> getLoadSet() const;
	const ValuePlaceHolder getFunctionTableBPlaceHolder() const;
	std::set<int> nodePositions() const override;
	const DOFS getDOFSForNode(int nodePosition) const override;
	std::shared_ptr<Loading> clone() const override;
	bool validate() const override;
	bool ineffective() const override;
};

} /* namespace vega */

#endif /* LOADING_H_ */
