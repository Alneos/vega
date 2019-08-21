/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Constraint.h
 *
 *  Created on: 2 juin 2014
 *      Author: siavelis
 */

#ifndef CONSTRAINT_H_
#define CONSTRAINT_H_

#include "MeshComponents.h"
#include "Object.h"
#include "BoundaryCondition.h"
#include "Dof.h"
#include "Reference.h"
#include "CoordinateSystem.h"
#include "Value.h"
#include "Target.h"
#include <map>
#include <list>

namespace vega {

class Model;
class Group;

/**
 * Base class for all Constraints
 */
class Constraint: public Identifiable<Constraint>, public BoundaryCondition {
	friend std::ostream &operator<<(std::ostream&, const Constraint&);    //output
public:
	enum class Type {
		QUASI_RIGID,
		RIGID,
		RBE3,
		SPC,
		LMPC,
		GAP,
		SLIDE,
		SURFACE_CONTACT,
		ZONE_CONTACT,
		SURFACE_SLIDE_CONTACT,
	};

protected:
	Model& model;
	Constraint(Model&, Type, int original_id = NO_ORIGINAL_ID);
    Constraint(const Constraint& that) = delete;
public:
    virtual ~Constraint() = default;
	const Type type;
	static const std::string name;
	static const std::map<Type, std::string> stringByType;
	const std::string to_str() const;
	virtual bool isContact() const {
	    return false;
	}
	virtual bool isNodeLoading() const {
		return false;
	}
	virtual bool isCellLoading() const {
		return false;
	}
	virtual void removeNodePosition(int nodePosition) = 0;
};

/**
 * Set of constraints that are often referenced by an analysis.
 */
class ConstraintSet final: public Identifiable<ConstraintSet> {
	Model& model;
	std::vector<Reference<ConstraintSet>> constraintSetReferences;
	friend std::ostream &operator<<(std::ostream&, const ConstraintSet&);
public:
	enum class Type {
		SPC, MPC, ALL, CONTACT
	};
	ConstraintSet(Model&, Type type, int original_id = NO_ORIGINAL_ID);
    ConstraintSet(const ConstraintSet& that) = delete;
	static constexpr int COMMON_SET_ID = 0;
	const Type type;
	static const std::string name;
	static const std::map<Type, std::string> stringByType;
	void add(const Reference<ConstraintSet>&); // LD Hack : see parseSPCADD
	const std::set<std::shared_ptr<Constraint>, ptrLess<Constraint>> getConstraints() const;
	const std::set<std::shared_ptr<Constraint>, ptrLess<Constraint>> getConstraintsByType(Constraint::Type) const;
	int size() const;
	inline bool empty() const {return size() == 0;};
	std::shared_ptr<ConstraintSet> clone() const;
	bool hasFunctions() const;
	bool hasContacts() const;
};

/**
 * Represent constraint applied on nodes
 */
class NodeConstraint: public Constraint, public NodeContainer {
protected:
	NodeConstraint(Model&, Constraint::Type, const int original_id = NO_ORIGINAL_ID);
public:
	std::set<int> nodePositions() const override final;
	bool isNodeLoading() const override final {
		return true;
	}
	bool ineffective() const override;
	virtual void removeNodePosition(int nodePosition) override {
	    NodeContainer::removeNodePosition(nodePosition);
	}
};

class MasterSlaveConstraint: public NodeConstraint {
protected:
	DOFS dofs;
	int masterPosition;
	MasterSlaveConstraint(Model& model, Type type, const DOFS& dofs, int masterId =
			UNAVAILABLE_MASTER, int original_id = NO_ORIGINAL_ID, const std::set<int>& slaveIds =
			std::set<int>());
public:
    virtual ~MasterSlaveConstraint() = default;
	static const int UNAVAILABLE_MASTER;
	virtual void addSlave(int slaveId);
	virtual int getMaster() const;
	virtual bool hasMaster() const;
	virtual std::set<int> getSlaves() const final;
	const DOFS getDOFSForNode(int nodePosition) const override;
	const DOFS getDOFS() const;
	void removeNodePosition(int nodePosition) override;
};

/**
 * Responsible of being a constraint for all its elements
 * where they must be limited to rigid movements for some dofs
 */
class QuasiRigidConstraint: public MasterSlaveConstraint {

public:
	QuasiRigidConstraint(Model& model, const DOFS& dofs, int masterId = UNAVAILABLE_MASTER,
			int original_id = NO_ORIGINAL_ID, const std::set<int>& slaveIds = std::set<int>());
	inline bool isCompletelyRigid() const {
		return this->dofs == DOFS::ALL_DOFS;
	}
};

class RigidConstraint: public MasterSlaveConstraint {

public:
	RigidConstraint(Model& model, int masterId = UNAVAILABLE_MASTER, int original_id =
			NO_ORIGINAL_ID, const std::set<int>& slaveIds = std::set<int>());
};

class RBE3 final: public MasterSlaveConstraint {
	std::map<int, DOFS> slaveDofsByPosition;
	std::map<int, double> slaveCoefByPosition;
public:
	RBE3(Model& model, int masterId, const DOFS dofs = DOFS::ALL_DOFS, int original_id =
			NO_ORIGINAL_ID);
    virtual void addSlave(int slaveId) override;
	void addRBE3Slave(int slaveId, DOFS slaveDOFS = DOFS::ALL_DOFS, double slaveCoef = 1);
	const DOFS getDOFSForNode(int nodePosition) const override;
	double getCoefForNode(int nodePosition) const;
};

class SinglePointConstraint: public NodeConstraint {
	std::array<ValueOrReference, 6> spcs;
public:
	//GC: static initialization order is undefined. A reference is needed here to
	//prevent undefined behaviour.
	static const ValueOrReference& NO_SPC;
	SinglePointConstraint(Model& model, const std::array<ValueOrReference, 6>& spcs, int original_id = NO_ORIGINAL_ID);

	SinglePointConstraint(Model& model, const std::array<ValueOrReference, 3>& spcs, int original_id = NO_ORIGINAL_ID);

	SinglePointConstraint(Model& model, int original_id = NO_ORIGINAL_ID);
	/**
	 * Constructor for backward compatibility.
	 * The value is assigned to all the dof present in DOFS.
	 */
	SinglePointConstraint(Model& model, DOFS dofs, double value = 0, int original_id = NO_ORIGINAL_ID);

	void setDOF(const DOF& dof, const ValueOrReference& value);
	void setDOFS(const DOFS& dofs, const ValueOrReference& value);
	double getDoubleForDOF(const DOF& dof) const;
	std::shared_ptr<Value> getReferenceForDOF(const DOF& dof) const;
	const DOFS getDOFSForNode(int nodePosition) const override;
	void emulateLocalDisplacementConstraint();
	bool hasReferences() const;
	bool ineffective() const override;
};


class LinearMultiplePointConstraint: public Constraint {
private:
    std::map<int, DOFCoefs> dofCoefsByNodePosition;
public:
    const double coef_impo;
    LinearMultiplePointConstraint(Model& model, double coef_impo = 0, int original_id =
            NO_ORIGINAL_ID);
    void addParticipation(int nodeId, double dx = 0, double dy = 0, double dz = 0, double rx = 0,
            double ry = 0, double rz = 0);
    DOFCoefs getDoFCoefsForNode(int nodePosition) const;
    /**
     * Sort all nodes positions by increasing coefs. Usefull to fuse various LMPC into ones.
     */
    std::vector<int> sortNodePositionByCoefs() const;
    std::set<int> nodePositions() const override;
    const DOFS getDOFSForNode(int nodePosition) const override;
    void removeNodePosition(int nodePosition) override;
    bool ineffective() const override;
};

class Contact: public Constraint {
protected:
    Contact(Model&, Type, int original_id = NO_ORIGINAL_ID);
public:
    bool isContact() const override final {
	    return true;
	};
};

class Gap: public Contact {
public:
	class GapParticipation {
	public:
		GapParticipation(int nodePosition, VectorialValue direction) :
				nodePosition(nodePosition), direction(direction) {
		}
		const int nodePosition;
		const VectorialValue direction;
	};
	Gap(Model&, int original_id = NO_ORIGINAL_ID);
	virtual ~Gap() = default;
	double initial_gap_opening = 0;
	virtual std::vector<std::shared_ptr<GapParticipation>> getGaps() const = 0;
};

class GapTwoNodes: public Gap {
private:
	std::map<int, int> directionNodePositionByconstrainedNodePosition;
public:
	GapTwoNodes(Model& model, int original_id = NO_ORIGINAL_ID);
	void addGapNodes(int constrainedNodeId, int directionNodeId);
	std::set<int> nodePositions() const override;
	const DOFS getDOFSForNode(int nodePosition) const override;
	std::vector<std::shared_ptr<GapParticipation>> getGaps() const override;
	void removeNodePosition(int nodePosithasFunctionsion) override;
	bool ineffective() const override;
};

class GapNodeDirection: public Gap {
private:
	std::map<int, VectorialValue> directionBynodePosition;
public:
	GapNodeDirection(Model& model, int original_id = NO_ORIGINAL_ID);
	void addGapNodeDirection(int constrainedNodeId, double directionX, double directionY = 0,
			double directionZ = 0);
	std::set<int> nodePositions() const override;
	const DOFS getDOFSForNode(int nodePosition) const override;
	std::vector<std::shared_ptr<GapParticipation>> getGaps() const override;
	void removeNodePosition(int nodePosition) override;
	bool ineffective() const override;
};

/**
 * see Nastran BCONP
 */
class SlideContact: public Contact {
    ValueOrReference friction = ValueOrReference::EMPTY_VALUE;
public:
	SlideContact(Model& model, double friction, Reference<Target> master, Reference<Target> slave, int original_id = NO_ORIGINAL_ID);
	SlideContact(Model& model, Reference<NamedValue> friction, Reference<Target> master, Reference<Target> slave, int original_id = NO_ORIGINAL_ID);
    const Reference<Target> master;
    const Reference<Target> slave;
    double getFriction() const;
    std::shared_ptr<CellGroup> masterCellGroup = nullptr;
    std::shared_ptr<CellGroup> slaveCellGroup = nullptr;
    int coordinateSystemPos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID;
	std::set<int> nodePositions() const override;
	const DOFS getDOFSForNode(int nodePosition) const override;
	void removeNodePosition(int nodePosition) override;
	bool ineffective() const override;
};

/**
 * see Nastran BSCONP
 */
class SurfaceContact: public Contact {
public:
	SurfaceContact(Model& model, Reference<Target> master, Reference<Target> slave, int original_id = NO_ORIGINAL_ID);
    const Reference<Target> master;
    const Reference<Target> slave;
    std::shared_ptr<CellGroup> masterCellGroup = nullptr;
    std::shared_ptr<CellGroup> slaveCellGroup = nullptr;
	std::set<int> nodePositions() const override;
	const DOFS getDOFSForNode(int nodePosition) const override;
	void removeNodePosition(int nodePosition) override;
	void makeBoundarySurfaces();
	bool ineffective() const override;
};

/**
 * see Nastran BCTABLE
 */
class ZoneContact: public Contact {
public:
	ZoneContact(Model& model, Reference<Target> master, Reference<Target> slave, int original_id = NO_ORIGINAL_ID);
    const Reference<Target> master;
    const Reference<Target> slave;
	std::set<int> nodePositions() const override;
	const DOFS getDOFSForNode(int nodePosition) const override;
	void removeNodePosition(int nodePosition) override;
	bool ineffective() const override;
};

/**
 * see Optistruct CONTACT option SLIDE
 */
class SurfaceSlide: public Constraint {
public:
	SurfaceSlide(Model& model, Reference<Target> master, Reference<Target> slave, int original_id = NO_ORIGINAL_ID);
    const Reference<Target> master;
    const Reference<Target> slave;
	std::set<int> nodePositions() const override;
	const DOFS getDOFSForNode(int nodePosition) const override;
	void removeNodePosition(int nodePosition) override;
	bool ineffective() const override;
	void makeCellsFromSurfaceSlide();
};

} /* namespace vega */

#endif /* CONSTRAINT_H_ */
