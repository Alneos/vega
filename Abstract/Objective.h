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
#include "MeshComponents.h"
#include "Dof.h"

namespace vega {

class Model;
class ObjectiveSet;

class Objective: public Identifiable<Objective> {
private:
    friend std::ostream &operator<<(std::ostream &out, const Objective&);    //output
public:
    enum class Type {
        NODAL_DISPLACEMENT_ASSERTION,
        NODAL_COMPLEX_DISPLACEMENT_ASSERTION,
        NODAL_CELL_VONMISES_ASSERTION,
        FREQUENCY_ASSERTION,
        FREQUENCY_SEARCH,
        COMPLEX_FREQUENCY_METHOD,
        FREQUENCY_EXCIT,
        MODAL_DAMPING,
        NONLINEAR_PARAMETERS,
        ARC_LENGTH_METHOD,
        NODAL_DISPLACEMENT_OUTPUT,
        VONMISES_STRESS_OUTPUT,
        FREQUENCY_OUTPUT,
    };
protected:
    Model& model;

public:
    const Type type;
    static const std::string name;
    static const std::map<Type, std::string> stringByType;
    const std::shared_ptr<ObjectiveSet> objectiveset;
protected:
    Objective(Model&, const std::shared_ptr<ObjectiveSet>, Objective::Type, int original_id = NO_ORIGINAL_ID);
    Objective(const Objective& that) = delete;
public:
    virtual ~Objective() = default;
    virtual bool isAssertion() const {
        return false;
    }
    virtual bool isOutput() const {
        return false;
    }
};

/**
 * Set of objectives that are referenced by an analysis.
 */
class ObjectiveSet final: public Identifiable<ObjectiveSet> {
	Model& model;
	std::vector<Reference<ObjectiveSet>> objectiveSetReferences;
	friend std::ostream &operator<<(std::ostream&, const ObjectiveSet&);
public:
	enum class Type {
	    DISP,
		FREQ,
		METHOD,
		CMETHOD,
		OFREQ,
		STRESS,
		SDAMP,
		NONLINEAR_STRATEGY,
		ASSERTION,
		ALL
	};
	ObjectiveSet(Model&, Type type, int original_id = NO_ORIGINAL_ID);
	ObjectiveSet(Model&, const Reference<ObjectiveSet>& objectiveSetRef);
	static constexpr int COMMON_SET_ID = 0;
	const Type type;
	static const std::string name;
	static const std::map<Type, std::string> stringByType;
	void add(const Reference<ObjectiveSet>&);
	std::set<std::shared_ptr<Objective>, ptrLess<Objective>> getObjectives() const;
	std::set<std::shared_ptr<Objective>, ptrLess<Objective>> getObjectivesByType(Objective::Type) const;
	size_t size() const;
	inline bool empty() const noexcept {return size() == 0;};
	std::unique_ptr<ObjectiveSet> clone() const;
};

class Assertion: public Objective {
protected:
    Assertion(Model&, const std::shared_ptr<ObjectiveSet>, Type, double tolerance, int original_id = NO_ORIGINAL_ID);
public:
    const double tolerance;
    virtual DOFS getDOFSForNode(const int nodePosition) const = 0;
    virtual std::set<int> nodePositions() const = 0;
    bool isAssertion() const noexcept override {
        return true;
    }
};

class NodalAssertion: public Assertion {
protected:
    NodalAssertion(Model&, const std::shared_ptr<ObjectiveSet>, Type, double tolerance, int nodeId, DOF dof,
            int original_id = NO_ORIGINAL_ID);
public:
    const int nodePosition;
    const int nodeId;
    const DOF dof;
    DOFS getDOFSForNode(const int nodePosition) const override final;
    std::set<int> nodePositions() const override final;
};

class NodalDisplacementAssertion: public NodalAssertion {
public:
    const double value;
    const double instant = -1;
    NodalDisplacementAssertion(Model&, const std::shared_ptr<ObjectiveSet>, double tolerance, int nodeId, DOF dof,
            double value, double instant, int original_id = NO_ORIGINAL_ID);
    friend std::ostream& operator<<(std::ostream&, const NodalDisplacementAssertion&);
};

class NodalComplexDisplacementAssertion: public NodalAssertion {
public:
    const std::complex<double> value;
    const double frequency = -1;
    NodalComplexDisplacementAssertion(Model&, const std::shared_ptr<ObjectiveSet>, double tolerance, int nodeId, DOF dof,
            std::complex<double> value, double frequency, int original_id = NO_ORIGINAL_ID);
    friend std::ostream& operator<<(std::ostream&, const NodalComplexDisplacementAssertion&);
};

class FrequencyAssertion: public Assertion {

public:
    const int number;
    const double cycles;
    const double eigenValue;
    const double generalizedMass;
    const double generalizedStiffness;
    FrequencyAssertion(Model&, const std::shared_ptr<ObjectiveSet>, int number, double cycles, double eigenValue, double generalizedMass, double generalizedStiffness, double tolerance, int original_id =
            NO_ORIGINAL_ID);
    DOFS getDOFSForNode(const int nodePosition) const override final;
    std::set<int> nodePositions() const override final;
    friend std::ostream& operator<<(std::ostream&, const FrequencyAssertion&);
};

class NodalCellVonMisesAssertion: public Assertion {

public:
    const int nodePosition;
    const int nodeId;
    const int cellPosition;
    const int cellId;
    const double value;
    NodalCellVonMisesAssertion(Model&, const std::shared_ptr<ObjectiveSet>, double tolerance, int cellId, int nodeId, double value, int original_id =
            NO_ORIGINAL_ID);
    DOFS getDOFSForNode(const int nodePosition) const override final;
    std::set<int> nodePositions() const override final;
    friend std::ostream& operator<<(std::ostream&, const NodalCellVonMisesAssertion&);
};

class AnalysisParameter: public Objective {
public:
    AnalysisParameter(Model&, const std::shared_ptr<ObjectiveSet>, Type type, int original_id = NO_ORIGINAL_ID);
};

class FrequencySearch: public AnalysisParameter {
protected:
    Reference<NamedValue> namedValue;
public:

    enum class FrequencyType {
        BAND,
        STEP,
        LIST
    };

    enum class NormType {
        MASS,
        MAX
    };
    FrequencySearch(Model&, const std::shared_ptr<ObjectiveSet>, const FrequencyType frequencyType, const NamedValue&, const NormType norm = NormType::MASS, int original_id = NO_ORIGINAL_ID);
    const FrequencyType frequencyType;
    const NormType norm;  /**< Method for normalizing eigenvectors: MASS or MAX **/
    std::shared_ptr<NamedValue> getValue() const;
    FunctionPlaceHolder getValueRangePlaceHolder() const;
};

class ComplexFrequencyMethod: public AnalysisParameter {
public:

    enum class NormType {
        MASS,
        MAX
    };
    ComplexFrequencyMethod(Model&, const std::shared_ptr<ObjectiveSet>, const std::shared_ptr<ObjectiveSet>, const NormType norm = NormType::MASS, int original_id = NO_ORIGINAL_ID);
    const NormType norm;  /**< Method for normalizing eigenvectors: MASS or MAX **/
    const std::shared_ptr<ObjectiveSet> complexMethodSet;
};

class FrequencyExcit: public AnalysisParameter {
protected:
    Reference<NamedValue> namedValue;
public:

    enum class FrequencyType {
        BAND,
        STEP,
        LIST,
        INTERPOLATE,
        SPREAD
    };

    enum class NormType {
        MASS,
        MAX
    };
    FrequencyExcit(Model&, const std::shared_ptr<ObjectiveSet>, const FrequencyType frequencyType, const NamedValue&, const NormType norm = NormType::MASS, int original_id = NO_ORIGINAL_ID);
    const FrequencyType frequencyType;
    double spread = 0.0;
    const NormType norm;  /**< Method for normalizing eigenvectors: MASS or MAX **/
    std::shared_ptr<NamedValue> getValue() const;
    FunctionPlaceHolder getValueRangePlaceHolder() const;
};

class ModalDamping: public AnalysisParameter {
protected:
    Reference<NamedValue> function_table;
public:
    enum class DampingType {
        G,
        CRIT,
        Q
    };
    std::shared_ptr<Value> function = nullptr;
    DampingType dampingType;
    ModalDamping(Model& model, const std::shared_ptr<ObjectiveSet>, const FunctionTable& function_table, const DampingType dampingType, int original_id =
            NO_ORIGINAL_ID);
    ModalDamping(Model& model, const std::shared_ptr<ObjectiveSet>, int function_table_id, const DampingType dampingType, int original_id = NO_ORIGINAL_ID);
    std::shared_ptr<FunctionTable> getFunctionTable() const;
    FunctionPlaceHolder getFunctionTablePlaceHolder() const;
};

class NonLinearStrategy: public AnalysisParameter {
public:
    const int number_of_increments;
    NonLinearStrategy(Model& model, const std::shared_ptr<ObjectiveSet>, const int number_of_increments, int original_id =
            NO_ORIGINAL_ID);
};

class ArcLengthMethod: public AnalysisParameter {
public:
    ArcLengthMethod(Model& model, const std::shared_ptr<ObjectiveSet>, const Reference<Objective>& strategy_reference, int original_id = NO_ORIGINAL_ID);
    const Reference<Objective>& strategy_reference;
};

class Output: public Objective {
protected:
    Output(Model&, const std::shared_ptr<ObjectiveSet>, Type, int original_id = NO_ORIGINAL_ID);
public:
    bool isOutput() const override {
        return true;
    }
};

class NodalDisplacementOutput: public Output, public NodeContainer {
private:
    std::shared_ptr<Reference<NamedValue>> collection;
public:
    enum class ComplexOutputType {
        REAL_IMAGINARY,
        PHASE_MAGNITUDE
    };
    NodalDisplacementOutput(Model& model, const std::shared_ptr<ObjectiveSet>, std::shared_ptr<Reference<NamedValue>> collection = nullptr, int original_id = NO_ORIGINAL_ID);
    ComplexOutputType complexOutput;
    virtual std::vector<std::shared_ptr<NodeGroup>> getNodeGroups() const override final;
    virtual bool hasNodeGroups() const noexcept override final;
};

class VonMisesStressOutput: public Output { //, public CellContainer {
private:
    std::shared_ptr<Reference<NamedValue>> collection;
    std::shared_ptr<CellContainer> cellContainer;
public:
    VonMisesStressOutput(Model& model, const std::shared_ptr<ObjectiveSet>, std::shared_ptr<Reference<NamedValue>> collection = nullptr, int original_id = NO_ORIGINAL_ID);
    std::shared_ptr<CellContainer> getCellContainer() const;// override final;
    void addCellGroup(const std::string& groupName);
};

class FrequencyOutput: public Output {
private:
    std::shared_ptr<Reference<NamedValue>> collection;
public:
    FrequencyOutput(Model& model, const std::shared_ptr<ObjectiveSet>, std::shared_ptr<Reference<NamedValue>> collection = nullptr, int original_id = NO_ORIGINAL_ID);
    std::shared_ptr<NamedValue> getCollection() const;
};


} /* namespace vega */

#endif /* OBJECTIVE_H_ */
