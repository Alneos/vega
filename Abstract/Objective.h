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
#include "Dof.h"

namespace vega {

class Model;

class Objective: public Identifiable<Objective> {
private:
    friend std::ostream &operator<<(std::ostream &out, const Objective&);    //output
public:
    enum class Type {
        NODAL_DISPLACEMENT_ASSERTION,
        NODAL_COMPLEX_DISPLACEMENT_ASSERTION,
        FREQUENCY_ASSERTION,
        FREQUENCY_TARGET,
        MODAL_DAMPING,
        NONLINEAR_STRATEGY,
        ARC_LENGTH_METHOD
    };
protected:
    const Model & model;

public:
    const Type type;
    static const std::string name;
    static const std::map<Type, std::string> stringByType;
protected:
    Objective(const Model&, Objective::Type, int original_id = NO_ORIGINAL_ID);
public:
    virtual std::shared_ptr<Objective> clone() const=0;
    virtual bool isAssertion() const {
        return false;
    }
};

class Assertion: public Objective {
protected:
    Assertion(const Model&, Type, double tolerance, int original_id = NO_ORIGINAL_ID);
public:
    const double tolerance;
    virtual const DOFS getDOFSForNode(const int nodePosition) const = 0;
    virtual std::set<int> nodePositions() const = 0;
    bool isAssertion() const override {
        return true;
    }
    virtual std::shared_ptr<Objective> clone() const=0;
};

class NodalAssertion: public Assertion {
protected:
    NodalAssertion(const Model&, Type, double tolerance, int nodeId, DOF dof,
            int original_id = NO_ORIGINAL_ID);
public:
    const int nodePosition;
    const DOF dof;
    const DOFS getDOFSForNode(const int nodePosition) const override final;
    std::set<int> nodePositions() const override final;
};

class NodalDisplacementAssertion: public NodalAssertion {
public:
    const double value;
    const double instant = -1;
    NodalDisplacementAssertion(const Model&, double tolerance, int nodeId, DOF dof,
            double value, double instant, int original_id = NO_ORIGINAL_ID);
    friend std::ostream& operator<<(std::ostream&, const NodalDisplacementAssertion&);
    std::shared_ptr<Objective> clone() const {
        return std::make_shared<NodalDisplacementAssertion>(*this);
    }
};

class NodalComplexDisplacementAssertion: public NodalAssertion {
public:
    const std::complex<double> value;
    const double frequency = -1;
    NodalComplexDisplacementAssertion(const Model&, double tolerance, int nodeId, DOF dof,
            std::complex<double> value, double frequency, int original_id = NO_ORIGINAL_ID);
    friend std::ostream& operator<<(std::ostream&, const NodalComplexDisplacementAssertion&);
    std::shared_ptr<Objective> clone() const {
        return std::make_shared<NodalComplexDisplacementAssertion>(*this);
    }
};

class FrequencyAssertion: public Assertion {

public:
    const int number;
    const double cycles;
    const double eigenValue;
    FrequencyAssertion(const Model&, int number, double cycles, double eigenValue, double tolerance, int original_id =
            NO_ORIGINAL_ID);
    std::shared_ptr<Objective> clone() const;
    const DOFS getDOFSForNode(const int nodePosition) const override final;
    std::set<int> nodePositions() const override final;
};

class AnalysisParameter: public Objective {
public:
    AnalysisParameter(const Model&, Type type, int original_id = NO_ORIGINAL_ID);
    virtual std::shared_ptr<Objective> clone() const=0;
};

class FrequencyTarget: public AnalysisParameter {
protected:
    Reference<NamedValue> namedValue;
public:

    enum class FrequencyType {
        BAND,
        STEP,
        LIST,
        SPREAD
    };

    enum class NormType {
        MASS,
        MAX
    };
    FrequencyTarget(const Model&, const FrequencyType frequencyType, const NamedValue&, const NormType norm = NormType::MASS, int original_id = NO_ORIGINAL_ID);
    const FrequencyType frequencyType;
    const NormType norm;  /**< Method for normalizing eigenvectors: MASS or MAX **/
    const std::shared_ptr<NamedValue> getValue() const;
    const FunctionPlaceHolder getValueRangePlaceHolder() const;
    std::shared_ptr<Objective> clone() const;
};

class ModalDamping: public AnalysisParameter {
protected:
    Reference<NamedValue> function_table;
public:
    std::shared_ptr<Value> function;
    ModalDamping(const Model& model, const FunctionTable& function_table, int original_id =
            NO_ORIGINAL_ID);
    ModalDamping(const Model& model, int function_table_id, int original_id = NO_ORIGINAL_ID);
    const std::shared_ptr<FunctionTable> getFunctionTable() const;
    const FunctionPlaceHolder getFunctionTablePlaceHolder() const;
    std::shared_ptr<Objective> clone() const;
};

class NonLinearStrategy: public AnalysisParameter {
public:
    const int number_of_increments;
    NonLinearStrategy(const Model& model, const int number_of_increments, int original_id =
            NO_ORIGINAL_ID);
    std::shared_ptr<Objective> clone() const;
};

class ArcLengthMethod: public AnalysisParameter {
public:
    ArcLengthMethod(const Model& model, const Reference<Objective>& strategy_reference, int original_id = NO_ORIGINAL_ID);
    const Reference<Objective>& strategy_reference;
    std::shared_ptr<Objective> clone() const;
};

} /* namespace vega */

#endif /* OBJECTIVE_H_ */
