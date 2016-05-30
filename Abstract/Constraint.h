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

#include "Object.h"
#include "BoundaryCondition.h"
#include "Dof.h"
#include "Reference.h"
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
	enum Type {
		QUASI_RIGID,
		RIGID,
		RBE3,
		SPC,
		LMPC,
		GAP,
	};		

protected:
	const Model& model;
	Constraint(const Model&, Type, int original_id = NO_ORIGINAL_ID);
public:
	const Type type;
	static const std::string name;
	static const std::map<Type, std::string> stringByType;
	virtual std::shared_ptr<Constraint> clone() const = 0;
	virtual void removeNode(int nodePosition) = 0;
};

/**
 * Set of constraints that are often referenced by an analysis.
 */
class ConstraintSet: public Identifiable<ConstraintSet> {
private:
	const Model& model;
	std::vector<Reference<ConstraintSet>> constraintSetReferences;
	friend std::ostream &operator<<(std::ostream&, const ConstraintSet&);
public:
	enum Type {
		SPC, SPCD, MPC, ALL
	};
	ConstraintSet(const Model&, Type type = SPC, int original_id = NO_ORIGINAL_ID);
	static const int COMMON_SET_ID = 0;
	const Type type;
	static const std::string name;
	static const std::map<Type, std::string> stringByType;
	void add(const Reference<ConstraintSet>&);
	const std::set<std::shared_ptr<Constraint> > getConstraints() const;
	const std::set<std::shared_ptr<Constraint> > getConstraintsByType(Constraint::Type) const;
	int size() const;
	std::shared_ptr<ConstraintSet> clone() const;
	virtual ~ConstraintSet();
};

class HomogeneousConstraint: public Constraint {
protected:
	DOFS dofs;
	int masterPosition;
	std::set<int> slavePositions;
	HomogeneousConstraint(Model& model, Type type, const DOFS& dofs, int masterId =
			UNAVAILABLE_MASTER, int original_id = NO_ORIGINAL_ID, const std::set<int>& slaveIds =
			std::set<int>());
public:
	static const int UNAVAILABLE_MASTER;
	virtual void addSlave(int slaveId);
	virtual int getMaster() const;
	virtual std::set<int> getSlaves() const;
	std::set<int> nodePositions() const override;
	const DOFS getDOFSForNode(int nodePosition) const override;
	const DOFS getDOFS() const;
	void removeNode(int nodePosition) override;
	bool ineffective() const override;
	virtual ~HomogeneousConstraint();
};

/**
 * Responsible of being a constraint for all its elements
 * where they must be limited to rigid movements for some dofs
 */
class QuasiRigidConstraint: public HomogeneousConstraint {

public:
	QuasiRigidConstraint(Model& model, const DOFS& dofs, int masterId = UNAVAILABLE_MASTER,
			int original_id = NO_ORIGINAL_ID, const std::set<int>& slaveIds = std::set<int>());
	std::shared_ptr<Constraint> clone() const override;
	inline bool isCompletelyRigid() const {
		return this->dofs == DOFS::ALL_DOFS;
	}
	std::set<int> getSlaves() const override; /** Get the two nodes bound by this Quasi-Rigid constraint */
	virtual ~QuasiRigidConstraint() {
	}
};

class RigidConstraint: public HomogeneousConstraint {

public:
	RigidConstraint(Model& model, int masterId = UNAVAILABLE_MASTER, int original_id =
			NO_ORIGINAL_ID, const std::set<int>& slaveIds = std::set<int>());
	std::shared_ptr<Constraint> clone() const override;
	virtual ~RigidConstraint() {
	}
};

class RBE3: public HomogeneousConstraint {
	std::map<int, DOFS> slaveDofsByPosition;
	std::map<int, double> slaveCoefByPosition;
public:
	RBE3(Model& model, int masterId, const DOFS dofs = DOFS::ALL_DOFS, int original_id =
			NO_ORIGINAL_ID);
	void addSlave(int slaveId, DOFS slaveDOFS = DOFS::ALL_DOFS, double slaveCoef = 1);
	const DOFS getDOFSForNode(int nodePosition) const override;
	double getCoefForNode(int nodePosition) const;
	std::shared_ptr<Constraint> clone() const override;
	virtual ~RBE3() {
	}
};

class SinglePointConstraint: public Constraint {
	std::set<int> _nodePositions;
	std::array<ValueOrReference, 6> spcs;
public:
	//GC: static initialization order is undefined. A reference is needed here to
	//prevent undefined behaviour. 
	static const ValueOrReference& NO_SPC;
	SinglePointConstraint(const Model& model, const std::array<ValueOrReference, 6>& spcs, Group* group =
	nullptr, int original_id = NO_ORIGINAL_ID);

	SinglePointConstraint(const Model& model, const std::array<ValueOrReference, 3>& spcs, Group* group =
	nullptr, int original_id = NO_ORIGINAL_ID);

	SinglePointConstraint(const Model& model, Group* group = nullptr, int original_id = NO_ORIGINAL_ID);
	/**
	 * Constructor for backward compatibility.
	 * The value is assigned to all the dof present in DOFS.
	 */
	SinglePointConstraint(const Model& model, DOFS dofs, double value = 0, Group* group = nullptr,
			int original_id = NO_ORIGINAL_ID);
	Group* group;

	void addNodeId(int nodeId);
	void setDOF(const DOF& dof, const ValueOrReference& value);
	void setDOFS(const DOFS& dofs, const ValueOrReference& value);
	double getDoubleForDOF(const DOF& dof) const;
	std::shared_ptr<Value> getReferenceForDOF(const DOF& dof) const;
	virtual std::set<int> nodePositions() const override;
	void removeNode(int nodePosition) override;
	const DOFS getDOFSForNode(int nodePosition) const override;
	bool hasReferences() const;
	bool ineffective() const override;
	std::shared_ptr<Constraint> clone() const override;
};

class LinearMultiplePointConstraint: public Constraint {
public:
	class DofCoefs {
	private:
		double coefs[6];
	public:
		DofCoefs(double dx = 0, double dy = 0, double dz = 0, double rx = 0, double ry = 0,
				double rz = 0) {
			coefs[0] = dx;
			coefs[1] = dy;
			coefs[2] = dz;
			coefs[3] = rx, coefs[4] = ry;
			coefs[5] = rz;
		}
		DofCoefs& operator+=(const DofCoefs& rv) {
			for (int i = 0; i < 6; i++) {
				coefs[i] += rv.coefs[i];
			}
			return *this;
		}
		double operator[](const int i) {
			if (i < 0 || i > 5)
				return 0;
			return coefs[i];
		}
	};
private:
	std::map<int, DofCoefs> dofCoefsByNodePosition;
public:
	const double coef_impo;
	LinearMultiplePointConstraint(Model& model, double coef_impo = 0, int original_id =
			NO_ORIGINAL_ID);
	std::shared_ptr<Constraint> clone() const override;
	void addParticipation(int nodeId, double dx = 0, double dy = 0, double dz = 0, double rx = 0,
			double ry = 0, double rz = 0);
	DofCoefs getDoFCoefsForNode(int nodePosition) const;
	std::set<int> nodePositions() const override;
	const DOFS getDOFSForNode(int nodePosition) const override;
	void removeNode(int nodePosition) override;
	bool ineffective() const override;
};

class Gap: public Constraint {
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
	double initial_gap_opening = 0;
	virtual std::vector<std::shared_ptr<GapParticipation>> getGaps() const = 0;
	virtual ~Gap();
};

class GapTwoNodes: public Gap {
private:
	std::map<int, int> directionNodePositionByconstrainedNodePosition;
public:
	GapTwoNodes(Model& model, int original_id = NO_ORIGINAL_ID);
	std::shared_ptr<Constraint> clone() const override;
	void addGapNodes(int constrainedNodeId, int directionNodeId);
	std::set<int> nodePositions() const override;
	const DOFS getDOFSForNode(int nodePosition) const override;
	std::vector<std::shared_ptr<GapParticipation>> getGaps() const override;
	void removeNode(int nodePosition) override;
	bool ineffective() const override;
};

class GapNodeDirection: public Gap {
private:
	std::map<int, VectorialValue> directionBynodePosition;
public:
	GapNodeDirection(Model& model, int original_id = NO_ORIGINAL_ID);
	std::shared_ptr<Constraint> clone() const override;
	void addGapNodeDirection(int constrainedNodeId, double directionX, double directionY = 0,
			double directionZ = 0);
	std::set<int> nodePositions() const override;
	const DOFS getDOFSForNode(int nodePosition) const override;
	std::vector<std::shared_ptr<GapParticipation>> getGaps() const override;
	void removeNode(int nodePosition) override;
	bool ineffective() const override;
};
} /* namespace vega */

#endif /* CONSTRAINT_H_ */
