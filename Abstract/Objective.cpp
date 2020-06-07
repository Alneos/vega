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

Objective::Objective(Model& model, const std::shared_ptr<ObjectiveSet> objectiveSet, Objective::Type type, int original_id) :
        Identifiable(original_id), model(model), type(type), objectiveset(objectiveSet) {
    if (objectiveSet != nullptr) {
        model.addObjectiveIntoObjectiveSet(this->getReference(), objectiveSet->getReference());
    }
}

const string Objective::name = "Objective";

const map<Objective::Type, string> Objective::stringByType = {
        { Objective::Type::NODAL_DISPLACEMENT_ASSERTION, "NODAL_DISPLACEMENT_ASSERTION" },
        { Objective::Type::NODAL_COMPLEX_DISPLACEMENT_ASSERTION, "NODAL_COMPLEX_DISPLACEMENT_ASSERTION" },
        { Objective::Type::NODAL_CELL_VONMISES_ASSERTION, "NODAL_CELL_VONMISES_ASSERTION" },
        { Objective::Type::FREQUENCY_ASSERTION, "FREQUENCY_ASSERTION" },
        { Objective::Type::FREQUENCY_SEARCH, "FREQUENCY_SEARCH" },
        { Objective::Type::COMPLEX_FREQUENCY_METHOD, "COMPLEX_FREQUENCY_METHOD" },
        { Objective::Type::FREQUENCY_EXCIT, "FREQUENCY_EXCIT" },
        { Objective::Type::MODAL_DAMPING, "MODAL_DAMPING" },
        { Objective::Type::NONLINEAR_PARAMETERS, "NONLINEAR_PARAMETERS" },
        { Objective::Type::ARC_LENGTH_METHOD, "ARC_LENGTH_METHOD" },
        { Objective::Type::NODAL_DISPLACEMENT_OUTPUT, "NODAL_DISPLACEMENT_OUTPUT" },
        { Objective::Type::VONMISES_STRESS_OUTPUT, "VONMISES_STRESS_OUTPUT" },
        { Objective::Type::FREQUENCY_OUTPUT, "FREQUENCY_OUTPUT" },
};

ostream &operator<<(ostream &out, const Objective& objective) {
    out << to_str(objective);
    return out;
}

ObjectiveSet::ObjectiveSet(Model& model, Type type, int original_id) :
        Identifiable(original_id), model(model), type(type) {
}

ObjectiveSet::ObjectiveSet(Model& model, const Reference<ObjectiveSet>& objectiveSetRef) :
		Identifiable(objectiveSetRef.original_id), model(model), type(objectiveSetRef.type) {
}

void ObjectiveSet::add(const Reference<ObjectiveSet>& objectiveSetReference) {
    objectiveSetReferences.push_back(objectiveSetReference);
}

set<shared_ptr<Objective>, ptrLess<Objective> > ObjectiveSet::getObjectives() const {
    auto result = model.getObjectivesByObjectiveSet(this->getReference());
    for (const auto& objectiveSetReference : objectiveSetReferences) {
        const auto& setToInsert = model.getObjectivesByObjectiveSet(
                objectiveSetReference);
        result.insert(setToInsert.begin(), setToInsert.end());
    }
    return result;
}

set<shared_ptr<Objective>, ptrLess<Objective> > ObjectiveSet::getObjectivesByType(
        Objective::Type type) const {
    set<shared_ptr<Objective>, ptrLess<Objective> > result;
    for (const auto& objective : getObjectives()) {
        if (objective->type == type) {
            result.insert(objective);
        }
    }
    return result;
}

size_t ObjectiveSet::size() const {
    return getObjectives().size();
}

unique_ptr<ObjectiveSet> ObjectiveSet::clone() const {
	return make_unique<ObjectiveSet>(*this);
}

const string ObjectiveSet::name = "ObjectiveSet";

const map<ObjectiveSet::Type, string> ObjectiveSet::stringByType = {
    { ObjectiveSet::Type::FREQ, "FREQUENCY_EXCIT" },
    { ObjectiveSet::Type::METHOD, "FREQUENCY_SEARCH" },
    { ObjectiveSet::Type::CMETHOD, "CMETHOD" },
    { ObjectiveSet::Type::DISP, "DISPLACEMENT_OUTPUT" },
    { ObjectiveSet::Type::OFREQ, "FREQUENCY_OUTPUT" },
    { ObjectiveSet::Type::STRESS, "STRESS_OUTPUT" },
    { ObjectiveSet::Type::SDAMP, "STRUCTURAL_DAMPING" },
    { ObjectiveSet::Type::NONLINEAR_STRATEGY, "NONLINEAR_STRATEGY" },
    { ObjectiveSet::Type::ASSERTION, "ASSERTION" },
    { ObjectiveSet::Type::ALL, "ALL" },
    };

ostream &operator<<(ostream &out, const ObjectiveSet& objectiveSet) {
    out << to_str(objectiveSet);
    return out;
}

Assertion::Assertion(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, Type type, double tolerance, int original_id) :
        Objective(model, objectiveset, type, original_id), tolerance(tolerance) {
}

NodalAssertion::NodalAssertion(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, Type type, double tolerance, int nodeId,
        DOF dof, int original_id) :
        Assertion(model, objectiveset, type, tolerance, original_id), nodePosition(model.mesh.findOrReserveNode(nodeId)), nodeId(nodeId), dof(dof) {
}

DOFS NodalAssertion::getDOFSForNode(const int nodePosition) const {
    UNUSEDV(nodePosition);
    return dof;
}
set<int> NodalAssertion::nodePositions() const {
    return {nodePosition};
}

NodalDisplacementAssertion::NodalDisplacementAssertion(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, double tolerance,
        int nodeId, DOF dof, double value, double instant, int original_id) :
        NodalAssertion(model, objectiveset, Objective::Type::NODAL_DISPLACEMENT_ASSERTION, tolerance, nodeId, dof,
                original_id), value(value), instant(instant) {
}

ostream &operator<<(ostream &out, const NodalDisplacementAssertion& objective) {
    out << to_str(objective) << "Node Pos " << objective.nodePosition << " DOF " << objective.dof << " "
            << objective.value;
    return out;
}

NodalComplexDisplacementAssertion::NodalComplexDisplacementAssertion(Model& model,
        const std::shared_ptr<ObjectiveSet> objectiveset,
        double tolerance, int nodeId, DOF dof, complex<double> value, double frequency,
        int original_id) :
        NodalAssertion(model, objectiveset, Objective::Type::NODAL_COMPLEX_DISPLACEMENT_ASSERTION, tolerance, nodeId, dof,
                original_id), value(value), frequency(frequency) {
}

ostream &operator<<(ostream &out, const NodalComplexDisplacementAssertion& objective) {
    out << to_str(objective) << "Node Pos " << objective.nodePosition << " DOF " << objective.dof << " "
            << objective.value;
    return out;
}

FrequencyAssertion::FrequencyAssertion(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, int number, double cycles, double eigenValue, double generalizedMass, double generalizedStiffness,
        double tolerance, int original_id) :
        Assertion(model, objectiveset, Objective::Type::FREQUENCY_ASSERTION, tolerance, original_id), number(number), cycles(cycles), eigenValue(eigenValue), generalizedMass(generalizedMass), generalizedStiffness(generalizedStiffness) {
}

DOFS FrequencyAssertion::getDOFSForNode(const int nodePosition) const {
    UNUSEDV(nodePosition);
    return DOFS::NO_DOFS;
}
set<int> FrequencyAssertion::nodePositions() const {
    return {};
}

ostream &operator<<(ostream &out, const FrequencyAssertion& objective) {
    out << to_str(objective) << "Cycles " << objective.cycles;
    return out;
}

NodalCellVonMisesAssertion::NodalCellVonMisesAssertion(Model& model,
        const std::shared_ptr<ObjectiveSet> objectiveset,
        double tolerance, int cellId, int nodeId, double value, int original_id) :
        Assertion(model, objectiveset, Objective::Type::NODAL_CELL_VONMISES_ASSERTION, tolerance, original_id),
        nodePosition(model.mesh.findOrReserveNode(nodeId)), nodeId(nodeId), cellPosition(model.mesh.findCellPosition(cellId)), cellId(cellId), value(value) {
}

DOFS NodalCellVonMisesAssertion::getDOFSForNode(const int nodePosition) const {
    UNUSEDV(nodePosition);
    return DOFS::NO_DOFS;
}
set<int> NodalCellVonMisesAssertion::nodePositions() const {
    return {nodePosition};
}

ostream &operator<<(ostream &out, const NodalCellVonMisesAssertion& objective) {
    out << to_str(objective) << "Cell Pos " << objective.cellPosition << "Node Pos " << objective.nodePosition << " Value "
            << objective.value;
    return out;
}

AnalysisParameter::AnalysisParameter(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, Type type, int original_id) :
        Objective(model, objectiveset, type, original_id) {
}

FrequencySearch::FrequencySearch(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, const FrequencyType frequencyType, const NamedValue& namedValue, NormType norm, int original_id) :
        AnalysisParameter(model, objectiveset, Objective::Type::FREQUENCY_SEARCH, original_id), namedValue(namedValue), frequencyType(frequencyType), norm(norm) {
}

shared_ptr<NamedValue> FrequencySearch::getValue() const {
    return model.find(namedValue);
}

FunctionPlaceHolder FrequencySearch::getValueRangePlaceHolder() const {
    return FunctionPlaceHolder(model, namedValue.type, namedValue.original_id, Function::ParaName::FREQ);
}

ComplexFrequencyMethod::ComplexFrequencyMethod(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, const std::shared_ptr<ObjectiveSet> complexMethodSet, NormType norm, int original_id) :
        AnalysisParameter(model, objectiveset, Objective::Type::COMPLEX_FREQUENCY_METHOD, original_id), norm(norm), complexMethodSet(complexMethodSet) {
}

FrequencyExcit::FrequencyExcit(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, const FrequencyType frequencyType, const NamedValue& namedValue, NormType norm, int original_id) :
        AnalysisParameter(model, objectiveset, Objective::Type::FREQUENCY_EXCIT, original_id), namedValue(namedValue), frequencyType(frequencyType), norm(norm) {
}

shared_ptr<NamedValue> FrequencyExcit::getValue() const {
    return model.find(namedValue);
}

FunctionPlaceHolder FrequencyExcit::getValueRangePlaceHolder() const {
    return FunctionPlaceHolder(model, namedValue.type, namedValue.original_id, Function::ParaName::FREQ);
}

ModalDamping::ModalDamping(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, const FunctionTable& function_table, const DampingType dampingType , int original_id) :
        AnalysisParameter(model, objectiveset, Objective::Type::MODAL_DAMPING, original_id), function_table(Value::Type::FUNCTION_TABLE,
                Reference<NamedValue>::NO_ID, function_table.getId()), dampingType(dampingType) {
}

ModalDamping::ModalDamping(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, int function_table_original_id, const DampingType dampingType, int original_id) :
        AnalysisParameter(model, objectiveset, Objective::Type::MODAL_DAMPING, original_id), function_table(Value::Type::FUNCTION_TABLE,
                function_table_original_id), dampingType(dampingType) {
}

shared_ptr<FunctionTable> ModalDamping::getFunctionTable() const {
    return static_pointer_cast<FunctionTable>(model.find(function_table));
}

FunctionPlaceHolder ModalDamping::getFunctionTablePlaceHolder() const {
    return FunctionPlaceHolder(model, function_table.type, function_table.original_id, Function::ParaName::FREQ);
}

NonLinearStrategy::NonLinearStrategy(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, int number_of_increments, int original_id) :
        AnalysisParameter(model, objectiveset, Objective::Type::NONLINEAR_PARAMETERS, original_id), number_of_increments(
                number_of_increments) {
}

ArcLengthMethod::ArcLengthMethod(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, const Reference<Objective>& strategy_reference, int original_id) :
        AnalysisParameter(model, objectiveset, Objective::Type::ARC_LENGTH_METHOD, original_id), strategy_reference(strategy_reference) {
}

Output::Output(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, Type type, int original_id) :
        Objective(model, objectiveset, type, original_id) {
}

NodalDisplacementOutput::NodalDisplacementOutput(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, shared_ptr<Reference<NamedValue>> collection, int original_id) :
        Output(model, objectiveset, Objective::Type::NODAL_DISPLACEMENT_OUTPUT, original_id), NodeContainer(model.mesh), collection(collection) {
}

vector<shared_ptr<NodeGroup>> NodalDisplacementOutput::getNodeGroups() const {
    auto nodeGroups = NodeContainer::getNodeGroups();
    if (collection != nullptr) {
        const auto& setValue = dynamic_pointer_cast<SetValue<int>>(model.find(*collection));
        if (setValue == nullptr)
            throw logic_error("Cannot find set of displacement output nodes");
        const string& groupName = "SET_" + to_string(setValue->getOriginalId());
        shared_ptr<NodeGroup> nodeGroup = nullptr;
        if (not model.mesh.hasGroup(groupName)) {
            nodeGroup = model.mesh.findOrCreateNodeGroup(groupName, NodeGroup::NO_ORIGINAL_ID, "SET");
            for (const int nodeId : setValue->getSet()) {
                nodeGroup->addNodeId(nodeId);
            }
            setValue->markAsWritten();
        } else {
            nodeGroup = model.mesh.findOrCreateNodeGroup(groupName, NodeGroup::NO_ORIGINAL_ID, "SET");
        }
        nodeGroups.push_back(nodeGroup);
    }
    // TODO LD Hack for NastranWriter, need to rethink all this, should be totally transparent (and NOT lazy)
    const auto& cellGroups = NodeContainer::getCellGroups();
    if (not cellGroups.empty()) {
        const string& groupName = "DISP_" + to_string(this->getId());
        auto nodeGroup = model.mesh.findOrCreateNodeGroup(groupName, NodeGroup::NO_ORIGINAL_ID, "DISP");
        for (const auto& cellGroup : cellGroups) {
            for (const int nodePosition : cellGroup->nodePositions()) {
                nodeGroup->addNodeByPosition(nodePosition);
            }
        }
        nodeGroups.push_back(nodeGroup);
    }
    return nodeGroups;
}

bool NodalDisplacementOutput::hasNodeGroups() const noexcept {
    return collection != nullptr or NodeContainer::hasNodeGroups();
}

VonMisesStressOutput::VonMisesStressOutput(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, shared_ptr<Reference<NamedValue>> collection, int original_id) :
        Output(model, objectiveset, Objective::Type::VONMISES_STRESS_OUTPUT, original_id), /*CellContainer(model.mesh),*/ collection(collection) {
    cellContainer = make_shared<CellContainer>(model.mesh);
}

shared_ptr<CellContainer> VonMisesStressOutput::getCellContainer() const {
    shared_ptr<CellContainer> lazyCellContainer = nullptr;
    if (collection != nullptr) {
        // LD TODO : move this part at finish() ?
        const auto& setValueBase = dynamic_pointer_cast<SetValueBase>(model.find(*collection));
        if (setValueBase == nullptr)
            throw logic_error("Cannot find set of von mises output cells");
        if (setValueBase->isAll()) {
            lazyCellContainer = make_shared<CellContainer>(model.mesh.allCells);
        } else {
            lazyCellContainer = make_shared<CellContainer>(model.mesh);
            lazyCellContainer->add(*cellContainer);
            const auto& setValue = dynamic_pointer_cast<SetValue<int>>(setValueBase);
            if (setValue == nullptr)
                throw logic_error("Cannot find set of von mises output cells (ints)");
            const string& groupName = "SET_" + to_string(setValue->getOriginalId());
            shared_ptr<CellGroup> cellGroup;
            if (not model.mesh.hasGroup(groupName)) {
                cellGroup = model.mesh.createCellGroup(groupName, CellGroup::NO_ORIGINAL_ID, "SET");
                for (const int cellId : setValue->getSet()) {
                    cellGroup->addCellId(cellId);
                }
                setValue->markAsWritten();
            }
            lazyCellContainer->add(*cellGroup);
        }
    } else {
        lazyCellContainer = cellContainer;
    }
    return lazyCellContainer;
}

void VonMisesStressOutput::addCellGroup(const string& groupName) {
    cellContainer->addCellGroup(groupName);
}

FrequencyOutput::FrequencyOutput(Model& model, const std::shared_ptr<ObjectiveSet> objectiveset, shared_ptr<Reference<NamedValue>> collection, int original_id) :
        Output(model, objectiveset, Objective::Type::FREQUENCY_OUTPUT, original_id), collection(collection) {
}

std::shared_ptr<NamedValue> FrequencyOutput::getCollection() const {
    if (collection != nullptr) {
        return model.find(*collection);
    }
    return nullptr;
}

} /* namespace vega */

