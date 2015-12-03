/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * Analysis.h
 *
 *  Created on: Sep 4, 2013
 *      Author: devel
 */

#ifndef ANALYSIS_H_
#define ANALYSIS_H_

#include <climits>

#include <map>
#include <set>
#include <utility>
#include <list>
#include <set>
#include "MeshComponents.h"
#include "Loading.h"
#include "Constraint.h"
#include "Objective.h"
#include "Value.h"
#include "Object.h"

namespace vega {

using namespace std;
class Model;
class ModelConfiguration;

/**
 * Responsible of keeping options to use in a finite element calculation
 */
class Analysis: public Identifiable<Analysis> {
private:
	friend ostream &operator<<(ostream &out, const Analysis& analysis);    //output
	std::map<int, char> boundaryDOFSByNodePosition;
public:
	enum Type {
		LINEAR_MECA_STAT,
		LINEAR_MODAL,
		LINEAR_DYNA_MODAL_FREQ,
		NONLINEAR_MECA_STAT,
		UNKNOWN,
	};
protected:
	list<std::shared_ptr<Reference<LoadSet>>>loadSet_references;
	list<std::shared_ptr<Reference<ConstraintSet>>> constraintSet_references;
	list<std::shared_ptr<Reference<Objective>>> assertion_references;

public:
	Model& model;
	const Type type;
	static const string name;
	static const map<Type,string> stringByType;
	std::shared_ptr<Analysis> previousAnalysis;

	Analysis(Model& model, const Type Type, const int original_id = NO_ORIGINAL_ID);

	void add(const Reference<LoadSet>&);
	void add(const Reference<ConstraintSet>&);
	void add(const Reference<Objective>&);

	bool contains(const Reference<LoadSet>) const;
	bool contains(const Reference<ConstraintSet>) const;
	bool contains(const Reference<Objective>) const;

	void remove(const Reference<LoadSet>);
	void remove(const Reference<ConstraintSet>);
	void remove(const Reference<Objective>);

	/**
	 * retrieve the ConstraintSets specific to this analysis
	 * plus eventually the common ConstraintSet of all analyzes of the model
	 */
	const vector<std::shared_ptr<ConstraintSet>> getConstraintSets() const;

	/**
	 * retrieve the LoadSets specific to this analysis
	 * plus eventually the common LoadSet of all analyzes of the model
	 */
	const vector<std::shared_ptr<LoadSet>> getLoadSets() const;
	const vector<std::shared_ptr<BoundaryCondition>> getBoundaryConditions() const;
	const vector<std::shared_ptr<Assertion>> getAssertions() const;

	void removeSPCNodeDofs(SinglePointConstraint& spc, int nodePosition, const DOFS dofs);
	void addBoundaryDOFS(int nodePosition, const DOFS dofs);
	const DOFS findBoundaryDOFS(int nodePosition) const;
	const set<int> boundaryNodePositions() const;

	virtual std::shared_ptr<Analysis> clone() const =0;
	bool validate() const override;

	virtual ~Analysis();

};

class LinearMecaStat: public Analysis {
public:
	LinearMecaStat(Model& model, const int original_id = NO_ORIGINAL_ID);
	std::shared_ptr<Analysis> clone() const;
};

class NonLinearMecaStat: public Analysis {
public:
	Reference<Objective> strategy_reference;
	NonLinearMecaStat(Model& model, const NonLinearStrategy& strategy, const int original_id =
			NO_ORIGINAL_ID);
	NonLinearMecaStat(Model& model, const int strategy_original_id, const int original_id =
			NO_ORIGINAL_ID);
	std::shared_ptr<Analysis> clone() const;
	bool validate() const override;
};

class LinearModal: public Analysis {
protected:
	Reference<Objective> frequency_band_reference;
public:
	LinearModal(Model& model, const FrequencyBand& frequency_band, const int original_id =
			NO_ORIGINAL_ID, const Type type = LINEAR_MODAL);
	LinearModal(Model& model, const int frequency_band_original_id, const int original_id =
			NO_ORIGINAL_ID, const Type type = LINEAR_MODAL);
	std::shared_ptr<FrequencyBand> getFrequencyBand() const;
	std::shared_ptr<Analysis> clone() const;
	bool validate() const override;
};

class LinearDynaModalFreq: public LinearModal {
protected:
	Reference<Objective> modal_damping_reference;
	Reference<Objective> frequency_values_reference;
public:
	LinearDynaModalFreq(Model& model, const FrequencyBand& frequency_band,
			const ModalDamping& modal_damping, const FrequencyValues& frequency_values,
			const bool residual_vector = false, const int original_id = NO_ORIGINAL_ID);
	LinearDynaModalFreq(Model& model, const int frequency_band_original_id,
			const int modal_damping_original_id, const int frequency_value_original_id,
			const bool residual_vector = false, const int original_id = NO_ORIGINAL_ID);
	const bool residual_vector;
	std::shared_ptr<ModalDamping> getModalDamping() const;
	std::shared_ptr<FrequencyValues> getFrequencyValues() const;
	std::shared_ptr<Analysis> clone() const;
	bool validate() const override;
};

} /* namespace vega */
#endif /* ANALYSIS_H_ */

