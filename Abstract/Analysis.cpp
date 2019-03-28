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
 *
 * Analysis.cpp
 *
 *  Created on: Sep 4, 2013
 *      Author: devel
 */

#if defined VDEBUG && defined __GNUC__ && !defined(_WIN32)
#include <valgrind/memcheck.h>
#endif
#include "Analysis.h"
#include "Model.h"
#include "BoundaryCondition.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

namespace vega {

using namespace std;

Analysis::Analysis(Model& model, const Type Type, const string original_label, int original_id) :
        Identifiable(original_id), label(original_label), model(model), type(Type) {
}

const string Analysis::name = "Analysis";

const map<Analysis::Type, string> Analysis::stringByType = {
        { Analysis::Type::LINEAR_MECA_STAT, "LINEAR_MECA_STAT" },
        { Analysis::Type::LINEAR_MODAL, "LINEAR_MODAL" },
        { Analysis::Type::LINEAR_BUCKLING, "LINEAR_BUCKLING" },
        { Analysis::Type::LINEAR_DYNA_DIRECT_FREQ, "LINEAR_DYNA_DIRECT_FREQ" },
        { Analysis::Type::LINEAR_DYNA_MODAL_FREQ, "LINEAR_DYNA_MODAL_FREQ" },
        { Analysis::Type::NONLINEAR_MECA_STAT, "NONLINEAR_MECA_STAT" },
        { Analysis::Type::COMBINATION, "COMBINATION" },
        { Analysis::Type::UNKNOWN, "UNKNOWN" }
};

map<string,string> Analysis::to_map() const {
    map<string, string> infos = Identifiable<Analysis>::to_map();
    if (!label.empty())
        infos["label"] = label;
    return infos;
}

ostream &operator<<(ostream &out, const Analysis& analysis) {
    out << to_str(analysis);
    return out;
}

void Analysis::add(const Reference<LoadSet>& loadSetReference) {
    this->loadSet_references.push_back(loadSetReference);
}

const vector<shared_ptr<LoadSet>> Analysis::getLoadSets() const {
    vector<shared_ptr<LoadSet>> result;
    shared_ptr<LoadSet> commonLoadSet = model.commonLoadSet;
    if (commonLoadSet != nullptr and not commonLoadSet->empty())
        result.push_back(commonLoadSet);
    for (const auto& loadSetReference : this->loadSet_references) {
        const auto& loadSet = model.find(loadSetReference);
        if (loadSet != nullptr)
            result.push_back(loadSet);
    }
    return result;
}

const vector<shared_ptr<BoundaryCondition>> Analysis::getBoundaryConditions() const {
    vector<shared_ptr<BoundaryCondition>> result;
    for (shared_ptr<ConstraintSet> constraintSet : getConstraintSets()) {
        if (constraintSet == nullptr) {
            continue;
        }
        for (const auto& constraint : constraintSet->getConstraints()) {
            if (constraint != nullptr) {
                result.push_back(constraint);
            }
        }
    }
    for (shared_ptr<LoadSet> loadSet : getLoadSets()) {
        if (loadSet == nullptr) {
            continue;
        }
        for (const auto& loading : loadSet->getLoadings()) {
            if (loading != nullptr) {
                result.push_back(loading);
            }
        }
    }
    return result;
}

void Analysis::add(const Reference<ConstraintSet>& constraintSetReference) {
    this->constraintSet_references.push_back(constraintSetReference);
}

void Analysis::remove(const Reference<LoadSet> loadSetReference) {
    auto it = loadSet_references.begin();
    while (it != loadSet_references.end()) {
        if (*it == loadSetReference) {
            it = loadSet_references.erase(it);
        } else {
            ++it;
        }
    }
}

void Analysis::remove(const Reference<ConstraintSet> constraintSetReference) {
    auto it = constraintSet_references.begin();
    while (it != constraintSet_references.end()) {
        if (*it == constraintSetReference) {
            it = constraintSet_references.erase(it);
        } else {
            ++it;
        }
    }
}

void Analysis::remove(const Reference<Objective> objectiveReference) {
    auto it = objectiveReferences.begin();
    while (it != objectiveReferences.end()) {
        if (*it == objectiveReference) {
            it = objectiveReferences.erase(it);
        } else {
            ++it;
        }
    }
}

bool Analysis::contains(const Reference<LoadSet> loadSetRef) const {
    for (const auto& loadset : getLoadSets()) {
        if (loadSetRef == loadset->getReference()) {
            return true;
        }
    }
    return false;
}

bool Analysis::contains(const Reference<ConstraintSet> constraintSetRef) const {
    for (const auto& constraintset : getConstraintSets()) {
        if (constraintSetRef == constraintset->getReference()) {
            return true;
        }
    }
    return false;
}

bool Analysis::contains(const Reference<Objective> objectiveRef) const {
    for (const auto& objective: getObjectives()) {
        if (objectiveRef == objective->getReference()) {
            return true;
        }
    }
    return false;
}

bool Analysis::contains(const LoadSet::Type type) const {
    for (const auto& loadset : getLoadSets()) {
        if (type == loadset->type) {
            return true;
        }
    }
    return false;
}

bool Analysis::contains(const ConstraintSet::Type type) const {
    for (const auto& constraintset : this->getConstraintSets()) {
        if (type == constraintset->type) {
            return true;
        }
    }
    return false;
}

bool Analysis::contains(const Objective::Type type) const {
    for (const auto& objective : this->getObjectives()) {
        if (type == objective->type) {
            return true;
        }
    }
    return false;
}


const vector<shared_ptr<ConstraintSet>> Analysis::getConstraintSets() const {
    vector<shared_ptr<ConstraintSet>> result;
    shared_ptr<ConstraintSet> commonConstraintSet = model.commonConstraintSet;
    if (commonConstraintSet != nullptr and not commonConstraintSet->empty())
        result.push_back(commonConstraintSet);
    for (const auto& constraintSetReference : this->constraintSet_references) {
        result.push_back(model.find(constraintSetReference));
    }

    return result;
}

void Analysis::add(const Reference<Objective>& assertionReference) {
    objectiveReferences.push_back(assertionReference);
}

const vector<shared_ptr<Assertion>> Analysis::getAssertions() const {
    vector<shared_ptr<Assertion>> assertions;
    for (const auto& assertion_reference : objectiveReferences) {
        shared_ptr<Objective> objective = model.find(assertion_reference);
        if (not objective->isAssertion()) continue;
        assertions.push_back(dynamic_pointer_cast<Assertion>(objective));
    }
    return assertions;
}

const vector<shared_ptr<Objective>> Analysis::getObjectives() const {
    vector<shared_ptr<Objective>> objectives;
    for (const auto& objective_reference : objectiveReferences) {
        shared_ptr<Objective> objective = model.find(objective_reference);
        objectives.push_back(objective);
    }
    return objectives;
}

bool Analysis::hasSPC() const{

    vector<std::shared_ptr<ConstraintSet>> allcs = this->getConstraintSets();
    for (const auto& cs : allcs){

        switch(cs->type){

        // All those types are SPC of one form or another
        case (ConstraintSet::Type::SPC):
        case (ConstraintSet::Type::MPC):{
            return true;
            break;
        }
        // For other type, we check each constraint
        default:{
            for (const auto & co : cs->getConstraints()){
                switch(co->type){
                case (Constraint::Type::SPC):
                case (Constraint::Type::LMPC):{
                    return true;
                    break;
                }
                default:{
                    continue;
                }
                }
            }
        }
        }
    }

    return false;
}

bool Analysis::validate() const {
    bool result = true;
    for (const auto& constraintSetReference : this->constraintSet_references) {
        if (model.find(constraintSetReference) == nullptr) {
            if (model.configuration.logLevel >= LogLevel::INFO) {
                cout << "Missing constraintset reference:" << constraintSetReference << endl;
            }
            result = false;
        }
    }
    for (const auto& loadSetReference : this->loadSet_references) {
        if (model.find(loadSetReference) == nullptr) {
            if (model.configuration.logLevel >= LogLevel::INFO) {
                cout << "Missing loadset reference:" << loadSetReference << endl;
            }
            result = false;
        }
    }
    return result;
}


void Analysis::removeSPCNodeDofs(SinglePointConstraint& spc, int nodePosition,  const DOFS dofsToRemove) {
    const DOFS& remainingDofs = spc.getDOFSForNode(nodePosition) - dofsToRemove;
    const int nodeId = model.mesh.findNodeId(nodePosition);
    set<shared_ptr<ConstraintSet>> affectedConstraintSets =
            model.getConstraintSetsByConstraint(
                    spc);
    if (remainingDofs.size() != 0) {
        const auto& remainingSpc = make_shared<SinglePointConstraint>(this->model);
        remainingSpc->addNodeId(nodeId);
        for (const DOF& remainingDof : remainingDofs) {
            remainingSpc->setDOF(remainingDof, spc.getDoubleForDOF(remainingDof));
        }
        model.add(remainingSpc);
        if (model.configuration.logLevel >= LogLevel::TRACE) {
            cout << "Created spc : " << *remainingSpc << " for node id : "
                    << nodeId
                    << " to handle dofs : " << remainingDofs << endl;
        }
        for (const auto& constraintSet : affectedConstraintSets) {
            model.addConstraintIntoConstraintSet(*remainingSpc, *constraintSet);
        }
    }
    if (model.analyses.size() >= 2) {
        const auto& otherAnalysesCS = make_shared<ConstraintSet>(model, ConstraintSet::Type::SPC);
        model.add(otherAnalysesCS);
        const auto& otherAnalysesSpc = make_shared<SinglePointConstraint>(this->model);
        otherAnalysesSpc->addNodeId(nodeId);
        for (DOF removedDof : dofsToRemove) {
            otherAnalysesSpc->setDOF(removedDof, spc.getDoubleForDOF(removedDof));
        }
        model.add(otherAnalysesSpc);
        model.addConstraintIntoConstraintSet(*otherAnalysesSpc, *otherAnalysesCS);
        if (this->model.configuration.logLevel >= LogLevel::TRACE) {
            cout << "Created spc : " << *otherAnalysesSpc << " for node id : "
                    << nodeId
                    << " to handle dofs : " << dofsToRemove << endl;
        }
        for (const auto& otherAnalysis : this->model.analyses) {
            if (this->getReference() == otherAnalysis->getReference()) {
                continue;
            }
            for (const auto& constraintSet : otherAnalysis->getConstraintSets()) {
                if (affectedConstraintSets.find(constraintSet) == affectedConstraintSets.end()) {
                    continue;
                }
                otherAnalysis->add(*otherAnalysesCS);
                break;
            }
        }
    }
    spc.removeNode(nodePosition);
}

void Analysis::addBoundaryDOFS(int nodePosition, const DOFS dofs) {
    boundaryDOFSByNodePosition[nodePosition] = DOFS(boundaryDOFSByNodePosition[nodePosition]) + dofs;
}

const DOFS Analysis::findBoundaryDOFS(int nodePosition) const {
    const auto& entry = boundaryDOFSByNodePosition.find(nodePosition);
    if (entry == boundaryDOFSByNodePosition.end()) {
        return DOFS::NO_DOFS;
    } else {
        return entry->second;
    }
}

const set<int> Analysis::boundaryNodePositions() const {
    set<int> result;
    for (const auto& entry : boundaryDOFSByNodePosition) {
        result.insert(entry.first);
    }
    return result;
}

void Analysis::copyInto(Analysis& other) const {
    for(const auto& loadSetRef : loadSet_references) {
        other.add(loadSetRef);
    }
    for(const auto& constraintSetRef : constraintSet_references) {
        other.add(constraintSetRef);
    }
    for(const auto& objectiveRef : objectiveReferences) {
        other.add(objectiveRef);
    }
}

void Analysis::createGraph(ostream& dot_ofs) {
    struct VertexProps { int id; string name; };
    struct EdgeProps   { string name; };
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexProps, EdgeProps> Graph;
    typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    //typedef boost::graph_traits<Graph>::edge_descriptor edge_descriptor;

    Graph g;
    map<int,vertex_descriptor> vertexPosByElementId;
    for (auto& elementSet : model.elementSets) {
        vertex_descriptor vpos = boost::add_vertex(g);
        auto& v = g[vpos];
        v.id = elementSet->getOriginalId();
        v.name = to_str(*elementSet);
        vertexPosByElementId[v.id] = vpos;
    }

    for (auto& entry : model.mesh.nodes.cellPartsByNodePart) {
        if (entry.second.size() == 1) continue;
        int nodePart = entry.first;
        vertex_descriptor vnpos = boost::add_vertex(g);
        auto& v = g[vnpos];
        v.id = nodePart;
        v.name = "Nodal connexion";
        for (int cellPartId: entry.second) {
            vertex_descriptor vepos = vertexPosByElementId[cellPartId];
            boost::add_edge(vnpos, vepos, g);
        }
    }

    for (const auto& constraintSet : getConstraintSets()) {
        vertex_descriptor vcpos = boost::add_vertex(g);
        auto& v = g[vcpos];
        v.name = to_str(*constraintSet);
        set<int> nodeParts;
        for(const auto& constraint : constraintSet->getConstraints()) {
            for(int nodePosition : constraint->nodePositions()) {
                nodeParts.insert(model.mesh.findNodePartId(nodePosition));
            }
        }
        set<int> cellParts;
        for(int nodePart : nodeParts) {
            const auto& cellParts2 = model.mesh.nodes.cellPartsByNodePart[nodePart];
            cellParts.insert(cellParts2.begin(), cellParts2.end());
        }
        for(int cellPartId : cellParts) {
            vertex_descriptor vepos = vertexPosByElementId[cellPartId];
            boost::add_edge(vcpos, vepos, g);
        }
    }

    for (const auto& loadSet : getLoadSets()) {
        vertex_descriptor vlpos = boost::add_vertex(g);
        auto& v = g[vlpos];
        v.name = to_str(*loadSet);
        set<int> nodeParts;
        for(const auto& loading : loadSet->getLoadings()) {
            for(int nodePosition : loading->nodePositions()) {
                nodeParts.insert(model.mesh.findNodePartId(nodePosition));
            }
        }
        set<int> cellParts;
        for(int nodePart : nodeParts) {
            const auto& cellParts2 = model.mesh.nodes.cellPartsByNodePart[nodePart];
            cellParts.insert(cellParts2.begin(), cellParts2.end());
        }
        for(int cellPartId : cellParts) {
            vertex_descriptor vepos = vertexPosByElementId[cellPartId];
            boost::add_edge(vlpos, vepos, g);
        }
    }

    boost::write_graphviz(dot_ofs, g, boost::make_label_writer(get(&VertexProps::name, g)),
            boost::make_label_writer(get(&EdgeProps::name, g)));
    std::cerr << "Graph node name:" << g[0].name << "\n";
}


Combination::Combination(Model& model, const string original_label, const int original_id) :
        Analysis(model, Analysis::Type::COMBINATION, original_label, original_id) {
}

LinearMecaStat::LinearMecaStat(Model& model, const string original_label, const int original_id) :
        Analysis(model, Analysis::Type::LINEAR_MECA_STAT, original_label, original_id) {

}

NonLinearMecaStat::NonLinearMecaStat(Model& model, const int strategy_id,
        const string original_label, const int original_id) :
        Analysis(model, Analysis::Type::NONLINEAR_MECA_STAT, original_label, original_id), strategy_reference(
                Objective::Type::NONLINEAR_STRATEGY, strategy_id) {

}

bool NonLinearMecaStat::validate() const {
    return Analysis::validate();
}

LinearModal::LinearModal(Model& model, const Reference<Objective>& frequencySearchRef,
        const string original_label, const int original_id, const Type type) :
        Analysis(model, type, original_label, original_id), frequencySearchRef(frequencySearchRef) {
}

LinearModal::LinearModal(Model& model, const int frequency_band_original_id,
        const string original_label, const int original_id, const Type type) :
        Analysis(model, type, original_label, original_id), frequencySearchRef(Objective::Type::FREQUENCY_SEARCH,
                frequency_band_original_id) {
}

shared_ptr<FrequencySearch> LinearModal::getFrequencySearch() const {
    return dynamic_pointer_cast<FrequencySearch>(model.find(frequencySearchRef));
}

bool LinearModal::validate() const {
    bool isValid = Analysis::validate();
    if (!getFrequencySearch()) {
        if (model.configuration.logLevel >= LogLevel::INFO) {
            cout << "Modal analysis is not valid: cannot find frenquency search:" << frequencySearchRef << endl;
        }
        isValid = false;
    }
    return isValid;
}

LinearBuckling::LinearBuckling(Model& model, const Reference<Objective>& frequencySearchRef,
        const string original_label, const int original_id) :
        LinearModal(model, frequencySearchRef, original_label, original_id, Type::LINEAR_BUCKLING) {
}

LinearDynaModalFreq::LinearDynaModalFreq(Model& model, const int frequency_band_original_id,
        const int modal_damping_original_id, const int frequency_value_original_id,
        const bool residual_vector, const string original_label, const int original_id) :
        LinearModal(model, frequency_band_original_id, original_label, original_id,
                Analysis::Type::LINEAR_DYNA_MODAL_FREQ), modal_damping_reference(Objective::Type::MODAL_DAMPING,
                modal_damping_original_id), frequencyExcitationRef(Objective::Type::FREQUENCY_EXCIT,
                frequency_value_original_id), residual_vector(residual_vector) {
}

shared_ptr<ModalDamping> LinearDynaModalFreq::getModalDamping() const {
    return dynamic_pointer_cast<ModalDamping>(model.find(modal_damping_reference));
}

shared_ptr<FrequencyExcit> LinearDynaModalFreq::getExcitationFrequencies() const {
    return dynamic_pointer_cast<FrequencyExcit>(model.find(frequencyExcitationRef));
}

bool LinearDynaModalFreq::validate() const {
    bool isValid = Analysis::validate();
    if (getExcitationFrequencies() == nullptr) {
        if (model.configuration.logLevel >= LogLevel::INFO) {
            cout << "Modal analysis is not valid: cannot find frenquency excitation:" << frequencyExcitationRef << endl;
        }
        isValid = false;
    }
    return isValid;
}

LinearDynaDirectFreq::LinearDynaDirectFreq(Model& model,
        const int frequency_value_original_id,
        const string original_label, const int original_id) :
        Analysis(model, Analysis::Type::LINEAR_DYNA_DIRECT_FREQ, original_label, original_id),
                frequencyExcitationRef(Objective::Type::FREQUENCY_EXCIT,
                frequency_value_original_id) {
}

shared_ptr<FrequencyExcit> LinearDynaDirectFreq::getExcitationFrequencies() const {
    return dynamic_pointer_cast<FrequencyExcit>(model.find(frequencyExcitationRef));
}

bool LinearDynaDirectFreq::validate() const {
    bool isValid = Analysis::validate();
    if (getExcitationFrequencies() == nullptr) {
        if (model.configuration.logLevel >= LogLevel::INFO) {
            cout << "Direct analysis is not valid: cannot find frenquency excitation:" << frequencyExcitationRef << endl;
        }
        isValid = false;
    }
    return isValid;
}

} /* namespace vega */
