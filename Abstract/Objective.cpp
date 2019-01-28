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
 * Objective.cpp
 *
 *  Created on: 6 mars 2014
 *      Author: siavelis
 */

#include "Objective.h"
#include "Model.h"

using namespace std;

namespace vega {

Objective::Objective(const Model& model, Objective::Type type, int original_id) :
        Identifiable(original_id), model(model), type(type) {
}

const string Objective::name = "Objective";

const map<Objective::Type, string> Objective::stringByType = {
        { Objective::Type::NODAL_DISPLACEMENT_ASSERTION, "NODAL_DISPLACEMENT_ASSERTION" },
        { Objective::Type::FREQUENCY_ASSERTION, "FREQUENCY_ASSERTION" },
        { Objective::Type::FREQUENCY_SEARCH, "FREQUENCY_SEARCH" },
        { Objective::Type::FREQUENCY_EXCIT, "FREQUENCY_EXCIT" },
        { Objective::Type::MODAL_DAMPING, "MODAL_DAMPING" },
        { Objective::Type::NONLINEAR_STRATEGY, "NONLINEAR_STRATEGY" }
};

ostream &operator<<(ostream &out, const Objective& objective) {
    out << to_str(objective);
    return out;
}

Assertion::Assertion(const Model& model, Type type, double tolerance, int original_id) :
        Objective(model, type, original_id), tolerance(tolerance) {
}

NodalAssertion::NodalAssertion(const Model& model, Type type, double tolerance, int nodeId,
        DOF dof, int original_id) :
        Assertion(model, type, tolerance, original_id), nodePosition(model.mesh->findOrReserveNode(nodeId)), dof(dof) {
}

const DOFS NodalAssertion::getDOFSForNode(const int nodePosition) const {
    UNUSEDV(nodePosition);
    return dof;
}
set<int> NodalAssertion::nodePositions() const {
    set<int> result;
    result.insert(nodePosition);
    return result;
}

NodalDisplacementAssertion::NodalDisplacementAssertion(const Model& model, double tolerance,
        int nodeId, DOF dof, double value, double instant, int original_id) :
        NodalAssertion(model, Objective::Type::NODAL_DISPLACEMENT_ASSERTION, tolerance, nodeId, dof,
                original_id), value(value), instant(instant) {
}

ostream &operator<<(ostream &out, const NodalDisplacementAssertion& objective) {
    out << to_str(objective) << "Node Pos " << objective.nodePosition << " DOF " << objective.dof << " "
            << objective.value;
    return out;
}

NodalComplexDisplacementAssertion::NodalComplexDisplacementAssertion(const Model& model,
        double tolerance, int nodeId, DOF dof, complex<double> value, double frequency,
        int original_id) :
        NodalAssertion(model, Objective::Type::NODAL_COMPLEX_DISPLACEMENT_ASSERTION, tolerance, nodeId, dof,
                original_id), value(value), frequency(frequency) {
}

ostream &operator<<(ostream &out, const NodalComplexDisplacementAssertion& objective) {
    out << to_str(objective) << "Node Pos " << objective.nodePosition << " DOF " << objective.dof << " "
            << objective.value;
    return out;
}

FrequencyAssertion::FrequencyAssertion(const Model& model, int number, double cycles, double eigenValue,
        double tolerance, int original_id) :
        Assertion(model, Objective::Type::FREQUENCY_ASSERTION, tolerance, original_id), number(number), cycles(cycles), eigenValue(eigenValue) {
}

const DOFS FrequencyAssertion::getDOFSForNode(const int nodePosition) const {
    UNUSEDV(nodePosition);
    return DOFS::NO_DOFS;
}
set<int> FrequencyAssertion::nodePositions() const {
    return set<int>();
}

shared_ptr<Objective> FrequencyAssertion::clone() const {
    return make_shared<FrequencyAssertion>(*this);
}

AnalysisParameter::AnalysisParameter(const Model& model, Type type, int original_id) :
        Objective(model, type, original_id) {
}

FrequencySearch::FrequencySearch(const Model& model, const FrequencyType frequencyType, const NamedValue& namedValue, NormType norm, int original_id) :
        AnalysisParameter(model, Objective::Type::FREQUENCY_SEARCH, original_id), namedValue(namedValue), frequencyType(frequencyType), norm(norm) {
}

const shared_ptr<NamedValue> FrequencySearch::getValue() const {
    return dynamic_pointer_cast<NamedValue>(model.find(namedValue));
}

const FunctionPlaceHolder FrequencySearch::getValueRangePlaceHolder() const {
    return FunctionPlaceHolder(model, namedValue.type, namedValue.original_id, Function::ParaName::FREQ);
}

shared_ptr<Objective> FrequencySearch::clone() const {
    return make_shared<FrequencySearch>(*this);
}

FrequencyExcit::FrequencyExcit(const Model& model, const FrequencyType frequencyType, const NamedValue& namedValue, NormType norm, int original_id) :
        AnalysisParameter(model, Objective::Type::FREQUENCY_EXCIT, original_id), namedValue(namedValue), frequencyType(frequencyType), norm(norm) {
}

const shared_ptr<NamedValue> FrequencyExcit::getValue() const {
    return dynamic_pointer_cast<NamedValue>(model.find(namedValue));
}

const FunctionPlaceHolder FrequencyExcit::getValueRangePlaceHolder() const {
    return FunctionPlaceHolder(model, namedValue.type, namedValue.original_id, Function::ParaName::FREQ);
}

shared_ptr<Objective> FrequencyExcit::clone() const {
    return make_shared<FrequencyExcit>(*this);
}

ModalDamping::ModalDamping(const Model& model, const FunctionTable& function_table, int original_id) :
        AnalysisParameter(model, Objective::Type::MODAL_DAMPING, original_id), function_table(Value::Type::FUNCTION_TABLE,
                Reference<NamedValue>::NO_ID, function_table.getId()) {
}

ModalDamping::ModalDamping(const Model& model, int function_table_original_id, int original_id) :
        AnalysisParameter(model, Objective::Type::MODAL_DAMPING, original_id), function_table(Value::Type::FUNCTION_TABLE,
                function_table_original_id) {
}

const shared_ptr<FunctionTable> ModalDamping::getFunctionTable() const {
    return dynamic_pointer_cast<FunctionTable>(model.find(function_table));
}

const FunctionPlaceHolder ModalDamping::getFunctionTablePlaceHolder() const {
    return FunctionPlaceHolder(model, function_table.type, function_table.original_id, Function::ParaName::FREQ);
}

shared_ptr<Objective> ModalDamping::clone() const {
    return make_shared<ModalDamping>(*this);
}

NonLinearStrategy::NonLinearStrategy(const Model& model, int number_of_increments, int original_id) :
        AnalysisParameter(model, Objective::Type::NONLINEAR_STRATEGY, original_id), number_of_increments(
                number_of_increments) {
}

shared_ptr<Objective> NonLinearStrategy::clone() const {
    return make_shared<NonLinearStrategy>(*this);
}

ArcLengthMethod::ArcLengthMethod(const Model& model, const Reference<Objective>& strategy_reference, int original_id) :
        AnalysisParameter(model, Objective::Type::ARC_LENGTH_METHOD, original_id), strategy_reference(strategy_reference) {
}

shared_ptr<Objective> ArcLengthMethod::clone() const {
    return make_shared<ArcLengthMethod>(*this);
}

} /* namespace vega */

