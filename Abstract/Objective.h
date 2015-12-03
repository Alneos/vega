/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * Objective.h
 *
 *  Created on: 6 mars 2014
 *      Author: siavelis
 */

#ifndef OBJECTIVE_H_
#define OBJECTIVE_H_

#include <climits>
#include <memory>
#include <set>
#include <complex>
#include "Value.h"
#include "Object.h"
#include "Reference.h"
#include "Dof.h"

namespace vega {

using namespace std;
class Model;

class Objective: public Identifiable<Objective> {
private:
	friend ostream &operator<<(ostream &out, const Objective&);    //output
public:
	enum Type {
		NODAL_DISPLACEMENT_ASSERTION,
		NODAL_COMPLEX_DISPLACEMENT_ASSERTION,
		FREQUENCY_ASSERTION,
		FREQUENCY_TARGET,
		FREQUENCY_BAND,
		MODAL_DAMPING,
		NONLINEAR_STRATEGY
	};
protected:
	const Model & model;

public:
	const Type type;
	static const string name;
	static const map<Type, string> stringByType;
protected:
	Objective(const Model&, Objective::Type, int original_id = NO_ORIGINAL_ID);
public:
	virtual std::shared_ptr<Objective> clone() const=0;
	virtual bool isAssertion() const {
		return false;
	}
	;
	virtual ~Objective() {
	}
	;
};

class Assertion: public Objective {
protected:
	Assertion(const Model&, Type, double tolerance, int original_id = NO_ORIGINAL_ID);
public:
	const double tolerance;
	virtual const DOFS getDOFSForNode(int nodePosition) const = 0;
	virtual std::set<int> nodePositions() const = 0;
	bool isAssertion() const override {
		return true;
	}
	virtual ~Assertion() {
	}
};

class NodalAssertion: public Assertion {
protected:
	NodalAssertion(const Model&, Type, double tolerance, int nodeId, DOF dof,
			int original_id = NO_ORIGINAL_ID);
public:
	const int nodePosition;
	const DOF dof;
	const DOFS getDOFSForNode(int nodePosition) const override final;
	std::set<int> nodePositions() const override final;
	~NodalAssertion() {
	}
};

class NodalDisplacementAssertion: public NodalAssertion {
public:
	const double value;
	const double instant = -1;
	NodalDisplacementAssertion(const Model&, double tolerance, int nodeId, DOF dof,
			double value, double instant, int original_id = NO_ORIGINAL_ID);
	std::shared_ptr<Objective> clone() const {
		return std::shared_ptr<Objective>(new NodalDisplacementAssertion(*this));
	}
	~NodalDisplacementAssertion() {
	}
	;
};

class NodalComplexDisplacementAssertion: public NodalAssertion {
public:
	const complex<double> value;
	const double frequency = -1;
	NodalComplexDisplacementAssertion(const Model&, double tolerance, int nodeId, DOF dof,
			complex<double> value, double frequency, int original_id = NO_ORIGINAL_ID);
	std::shared_ptr<Objective> clone() const {
		return std::shared_ptr<Objective>(new NodalComplexDisplacementAssertion(*this));
	}
	~NodalComplexDisplacementAssertion() {
	}
	;
};

class FrequencyAssertion: public Assertion {

public:
	const int number;
	const double value;
	FrequencyAssertion(const Model&, int number, double value, double tolerance, int original_id =
			NO_ORIGINAL_ID);
	std::shared_ptr<Objective> clone() const;
	const DOFS getDOFSForNode(int nodePosition) const override final;
	std::set<int> nodePositions() const override final;
	~FrequencyAssertion() {
	}
	;
};

class AnalysisParameter: public Objective {
public:
	AnalysisParameter(const Model&, Type type, int original_id = NO_ORIGINAL_ID);
	std::shared_ptr<Objective> clone() const;
	~AnalysisParameter() {
	}
	;
};

class FrequencyValues: public AnalysisParameter {
protected:
	Reference<Value> step_range;
public:
	FrequencyValues(const Model&, const StepRange&, int original_id = NO_ORIGINAL_ID);
	FrequencyValues(const Model&, int step_range_id, int original_id = NO_ORIGINAL_ID);
	const std::shared_ptr<StepRange> getStepRange() const;
	const ValuePlaceHolder getStepRangePlaceHolder() const;
	std::shared_ptr<Objective> clone() const;
	~FrequencyValues() {
	}
	;
};

class FrequencyBand: public AnalysisParameter {
public:
	const double lower;
	const double upper;
	const int num_max;
	FrequencyBand(const Model& model, double lower, double upper, int num_max, int original_id =
			NO_ORIGINAL_ID);
	std::shared_ptr<Objective> clone() const;
	~FrequencyBand() {
	}
	;
};

class ModalDamping: public AnalysisParameter {
protected:
	Reference<Value> function_table;
public:
	std::shared_ptr<Value> function;
	ModalDamping(const Model& model, const FunctionTable& function_table, int original_id =
			NO_ORIGINAL_ID);
	ModalDamping(const Model& model, int function_table_id, int original_id = NO_ORIGINAL_ID);
	const std::shared_ptr<FunctionTable> getFunctionTable() const;
	const ValuePlaceHolder getFunctionTablePlaceHolder() const;
	std::shared_ptr<Objective> clone() const;
	~ModalDamping() {
	}
	;
};

class NonLinearStrategy: public AnalysisParameter {
public:
	const int number_of_increments;
	NonLinearStrategy(const Model& model, const int number_of_increments, int original_id =
			NO_ORIGINAL_ID);
	std::shared_ptr<Objective> clone() const;
	~NonLinearStrategy() {
	}
	;
};

} /* namespace vega */

#endif /* OBJECTIVE_H_ */
