/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Model.cpp
 *
 *  Created on: Jan 11, 2013
 *      Author: dallolio
 */

#include "Model.h"

#include <iostream>
#include <string>
#include <fstream>
#include <boost/assign.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <ciso646>

using namespace std;

namespace vega {

constexpr int LoadSet::COMMON_SET_ID; // required for C++11
constexpr int ConstraintSet::COMMON_SET_ID; // required for C++11

Model::Model(string name, string inputSolverVersion, SolverName inputSolver,
        const ModelConfiguration configuration, const vega::ConfigurationParameters::TranslationMode translationMode) :
        name(name), inputSolverVersion(inputSolverVersion), //
        inputSolver(inputSolver), //
        modelType(ModelType::TRIDIMENSIONAL_SI), //
        mesh{configuration.logLevel, name},
        configuration(configuration), translationMode(translationMode), //
        commonLoadSet(make_shared<LoadSet>(*this, LoadSet::Type::ALL, LoadSet::COMMON_SET_ID)), //
		commonConstraintSet(make_shared<ConstraintSet>(*this, ConstraintSet::Type::ALL, ConstraintSet::COMMON_SET_ID)) {
    //this->mesh = make_shared<Mesh>(configuration.logLevel, name);
    this->finished = false;
    this->onlyMesh = false;
}

void Model::add(const shared_ptr<Analysis>& analysis) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << *analysis << endl;
    }
    analyses.add(analysis);
}

void Model::add(const shared_ptr<Loading>& loading) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << *loading << endl;
    }
    loadings.add(loading);
}

void Model::add(const std::shared_ptr<LoadSet>& loadSet) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << *loadSet << endl;
    }
    loadSets.add(loadSet);
}

// This "add" function used shared_ptr because adding an object
// clone it, which does not keep the original id.
//TODO: same type of "add" than the other ?
void Model::add(const shared_ptr<Material>& material) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << *material << endl;
    }
    materials.add(material);
}

void Model::add(const shared_ptr<Constraint>& constraint) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << *constraint << endl;
    }
    constraints.add(constraint);
}

void Model::add(const shared_ptr<ConstraintSet>& constraintSet) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << *constraintSet << endl;
    }
    constraintSets.add(constraintSet);
}

void Model::add(const shared_ptr<Objective>& objective) {
    if (configuration.logLevel >= LogLevel::TRACE) {
        cout << "Adding " << *objective << endl;
    }
    objectives.add(objective);
}

void Model::add(const shared_ptr<NamedValue>& value) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << *value << endl;
    }
    values.add(value);
}

void Model::add(const shared_ptr<ElementSet>& elementSet) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << *elementSet << endl;
    }
    elementSets.add(elementSet);
}

void Model::add(const shared_ptr<Target>& target) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << *target << endl;
    }
    targets.add(target);
}

shared_ptr<Analysis> Model::getAnalysis(int id) const {
	return analyses.get(id);
}

shared_ptr<Loading> Model::getLoading(int id) const {
    return loadings.get(id);
}

shared_ptr<LoadSet> Model::getLoadSet(int id)  const {
    return loadSets.get(id);
}

shared_ptr<Constraint> Model::getConstraint(int id) const {
    return constraints.get(id);
}

shared_ptr<ConstraintSet> Model::getConstraintSet(int id) const {
    return constraintSets.get(id);
}

shared_ptr<Objective> Model::getObjective(int id) const {
    return objectives.get(id);
}

shared_ptr<NamedValue> Model::getValue(int id) const {
    return values.get(id);
}

shared_ptr<ElementSet> Model::getElementSet(int id) const {
    return elementSets.get(id);
}

shared_ptr<Material> Model::getMaterial(int id) const {
    return materials.get(id);
}

shared_ptr<Target> Model::getTarget(int id) const {
    return targets.get(id);
}

const vector<int> Model::getMaterialsId() const{
	vector<int> v;
	for (const auto& material : materials){
		v.push_back(material->getId());
	}
	return v;
}

const vector<int> Model::getElementSetsId() const{
	vector<int> v;
	for (const auto& elementSet : elementSets){
		v.push_back(elementSet->getId());
	}
	return v;
}

template<>
void Model::remove(const Reference<Constraint> constraintReference) {
    for (const auto& it : constraintReferences_by_constraintSet_ids) {
        for (const auto& it2 : it.second) {
            if (it2 == constraintReference) {
                constraintReferences_by_constraintSet_ids[it.first].erase(it2);
                break; // iterator is invalid now
            }
        }
    }
    for (const auto& it : constraintReferences_by_constraintSet_original_ids_by_constraintSet_type) {
        for (const auto& it2 : it.second) {
            for (const auto& it3 : it2.second) {
                if (it3 == constraintReference) {
                    constraintReferences_by_constraintSet_original_ids_by_constraintSet_type[it.first][it2.first].erase(
                            it3);
                    break; // iterator is invalid now
                }

            }
        }
    }
    constraints.erase(constraintReference);
}

void Model::remove(const Reference<Constraint> refC, const int idCS, const int originalIdCS, const ConstraintSet::Type csT) {

    const auto & cR = constraintReferences_by_constraintSet_ids[idCS];
    for (const auto& it2 : cR) {
        if (it2 == refC){
            constraintReferences_by_constraintSet_ids[idCS].erase(it2);
            break; // iterator is invalid now
        }
    }
    if (originalIdCS!= Identifiable<ConstraintSet>::NO_ORIGINAL_ID){
        const auto & cR2 = constraintReferences_by_constraintSet_original_ids_by_constraintSet_type[csT][originalIdCS];
        for (const auto& it3 : cR2) {
            if (it3 == refC){
                constraintReferences_by_constraintSet_original_ids_by_constraintSet_type[csT][originalIdCS].erase(it3);
                break; // iterator is invalid now
            }
        }
    }
    constraints.erase(refC);
}

template<>
void Model::remove(const Reference<Loading> loadingReference) {
    for (const auto& it : loadingReferences_by_loadSet_ids) {
        for (const auto& it2 : it.second) {
            if (it2 == loadingReference) {
                loadingReferences_by_loadSet_ids[it.first].erase(it2);
                break; // iterator is invalid now
            }
        }
    }
    for (const auto& it : loadingReferences_by_loadSet_original_ids_by_loadSet_type) {
        for (const auto& it2 : it.second) {
            for (const auto& it3 : it2.second) {
                if (it3 == loadingReference) {
                    loadingReferences_by_loadSet_original_ids_by_loadSet_type[it.first][it2.first].erase(
                            it3);
                            break; // iterator is invalid now
                }
            }
        }
    }
    loadings.erase(loadingReference);
}

template<>
void Model::remove(const Reference<LoadSet> loadSetReference) {
    shared_ptr<LoadSet> loadSet = loadSets.find(loadSetReference);
    for (const auto& analysis : analyses) {
        if (analysis->contains(loadSetReference)) {
            if (configuration.logLevel >= LogLevel::DEBUG)
                cout << "Disassociating empty " << *loadSet << " from " << *analysis << endl;
            analysis->remove(loadSetReference);
        }
    }
    loadSets.erase(loadSetReference);
}

template<>
void Model::remove(const Reference<ConstraintSet> constraintSetReference) {
    shared_ptr<ConstraintSet> constraintSet = constraintSets.find(constraintSetReference);
    for (const auto& analysis : analyses) {
        if (analysis->contains(constraintSetReference)) {
            if (configuration.logLevel >= LogLevel::DEBUG)
                cout << "Disassociating empty " << *constraintSet << " from " << *analysis << endl;
            analysis->remove(constraintSetReference);
        }
    }
    constraintSets.erase(constraintSetReference);
}

template<>
void Model::remove(const Reference<Objective> objectiveReference) {
    shared_ptr<Objective> objective = objectives.find(objectiveReference);
    if (objective && objective->isAssertion()) {
        for (const auto& analysis : analyses) {
            if (analysis->contains(objectiveReference)) {
                if (configuration.logLevel >= LogLevel::TRACE) {
                    cout << "Disassociating dangling " << *objective << " from  " << *analysis
                            << endl;
                }
                analysis->remove(objectiveReference);
            }
        }
    }
    objectives.erase(objectiveReference);
}

template<>
const shared_ptr<Objective> Model::find(const Reference<Objective> reference) const {
    return objectives.find(reference);
}

template<>
const shared_ptr<NamedValue> Model::find(const Reference<NamedValue> reference) const {
    return values.find(reference);
}

template<>
const shared_ptr<Loading> Model::find(const Reference<Loading> reference) const {
    return loadings.find(reference);
}

template<>
const shared_ptr<LoadSet> Model::find(const Reference<LoadSet> reference) const {
    return loadSets.find(reference);
}

template<>
const shared_ptr<Constraint> Model::find(const Reference<Constraint> reference) const {
    return constraints.find(reference);
}

template<>
const shared_ptr<ConstraintSet> Model::find(const Reference<ConstraintSet> reference) const {
    return constraintSets.find(reference);
}

template<>
const shared_ptr<Analysis> Model::find(const Reference<Analysis> reference) const {
    shared_ptr<Analysis> analysis;
    if (reference.type == Analysis::Type::UNKNOWN and reference.id == Reference<Analysis>::NO_ID)
        // retrieve by original_id
        analysis = analyses.find(reference.original_id);
    else
        analysis = analyses.find(reference);
    return analysis;
}

template<>
const shared_ptr<ElementSet> Model::find(const Reference<ElementSet> reference) const {
    shared_ptr<ElementSet> elementSet;
    if (reference.type == ElementSet::Type::UNKNOWN and reference.id == Reference<ElementSet>::NO_ID)
        // retrieve by original_id
        elementSet = elementSets.find(reference.original_id);
    else
        elementSet = elementSets.find(reference);
    return elementSet;
}

template<>
const shared_ptr<Target> Model::find(const Reference<Target> reference) const {
    shared_ptr<Target> target;
    if (reference.id == Reference<Target>::NO_ID)
        // retrieve by original_id
        target = targets.find(reference.original_id);
    else
        target = targets.find(reference);
    return target;
}

void Model::addLoadingIntoLoadSet(const Reference<Loading>& loadingReference,
        const Reference<LoadSet>& loadSetReference) {
    if (loadSetReference.has_id())
        loadingReferences_by_loadSet_ids[loadSetReference.id].insert(loadingReference);
    if (loadSetReference.has_original_id())
        loadingReferences_by_loadSet_original_ids_by_loadSet_type[loadSetReference.type][loadSetReference.original_id].insert(
                loadingReference);
    if (loadSetReference == commonLoadSet->getReference() && find(commonLoadSet->getReference()) == nullptr)
        add(commonLoadSet); // commonLoadSet is added to the model if needed
    if (this->find(loadSetReference) == nullptr) {
        const auto& loadSet = make_shared<LoadSet>(*this, loadSetReference.type, loadSetReference.original_id);
        this->add(loadSet);
    }
}


shared_ptr<LoadSet> Model::getOrCreateLoadSet(int loadset_id, LoadSet::Type loadset_type){

    Reference<LoadSet> loadSetReference(loadset_type, loadset_id);
    shared_ptr<LoadSet> loadSetPtr = this->find(loadSetReference);
    if (loadSetPtr==nullptr){
        LoadSet loadSet(*this, loadset_type, loadset_id);
        loadSetPtr= loadSet.clone();
        this->loadSets.add(loadSetPtr);
    }
    return loadSetPtr;
}


const set<shared_ptr<Loading>> Model::getLoadingsByLoadSet(
        const Reference<LoadSet>& loadSetReference) const {
    set<shared_ptr<Loading>> result;
    auto itm = loadingReferences_by_loadSet_ids.find(loadSetReference.id);
    if (itm != loadingReferences_by_loadSet_ids.end()) {
        for (const auto& itm2 : itm->second) {
            shared_ptr<Loading> loading = find(itm2);
            if (loading == nullptr) {
                throw logic_error("Missing loading declared in loadingSet : " + to_str(itm2));
            }
            result.insert(loading);
        }
    }
    auto itm2 = loadingReferences_by_loadSet_original_ids_by_loadSet_type.find(
            loadSetReference.type);
    if (itm2 != loadingReferences_by_loadSet_original_ids_by_loadSet_type.end()) {
        auto itm3 = itm2->second.find(loadSetReference.original_id);
        if (itm3 != itm2->second.end()) {
            for (const auto& itm4 : itm3->second) {
                shared_ptr<Loading> loading = find(itm4);
            if (loading == nullptr) {
                throw logic_error("Missing loading declared in loadingSet : " + to_str(itm4));
            }
                result.insert(loading);
            }
        }
    }
    return result;
}

void Model::addConstraintIntoConstraintSet(const Reference<Constraint>& constraintReference,
        const Reference<ConstraintSet>& constraintSetReference) {
    if (constraintSetReference.has_id())
        constraintReferences_by_constraintSet_ids[constraintSetReference.id].insert(
                constraintReference);
    if (constraintSetReference.has_original_id())
        constraintReferences_by_constraintSet_original_ids_by_constraintSet_type[constraintSetReference.type][constraintSetReference.original_id].insert(
                constraintReference);
    if (constraintSetReference == commonConstraintSet->getReference()
            and find(commonConstraintSet->getReference()) == nullptr)
        add(commonConstraintSet); // commonConstraintSet is added to the model if needed
}

const set<shared_ptr<Constraint>> Model::getConstraintsByConstraintSet(
        const Reference<ConstraintSet>& constraintSetReference) const {
    set<shared_ptr<Constraint>> result;
    auto itm = constraintReferences_by_constraintSet_ids.find(constraintSetReference.id);
    if (itm != constraintReferences_by_constraintSet_ids.end()) {
        for (const auto& itm2 : itm->second) {
            result.insert(find(itm2));
        }
    }
    auto itm2 = constraintReferences_by_constraintSet_original_ids_by_constraintSet_type.find(
            constraintSetReference.type);
    if (itm2 != constraintReferences_by_constraintSet_original_ids_by_constraintSet_type.end()) {
        auto itm3 = itm2->second.find(constraintSetReference.original_id);
        if (itm3 != itm2->second.end()) {
            for (const auto& itm4 : itm3->second) {
                result.insert(find(itm4));
            }
        }
    }
    return result;
}

const set<shared_ptr<ConstraintSet>> Model::getConstraintSetsByConstraint(
        const Reference<Constraint>& constraintReference) const {
    set<shared_ptr<ConstraintSet>> result;
    for (const auto& it : constraintSets) {
        set<shared_ptr<Constraint>> constraints = getConstraintsByConstraintSet(it->getReference());
        bool found = false;
        for (const auto& constraint : constraints) {
            if (constraint == nullptr) {
                throw logic_error("Missing constraint declared in constraintSet : " + to_str(*it));
            }
            if (constraint->getReference() == constraintReference) {
                found = true;
                break;
            }
        }
        if (found) {
            result.insert(it);
        }
    }
    return result;
}

const vector<shared_ptr<ConstraintSet>> Model::getActiveConstraintSets() const {
    vector<shared_ptr<ConstraintSet>> result;
    set<shared_ptr<ConstraintSet>> set;
    for (const auto& analyse : analyses) {
        for (const auto& constraintSet : analyse->getConstraintSets()) {
            if (set.find(constraintSet) == set.end()) {
                set.insert(constraintSet);
                result.push_back(constraintSet);
            }
        }
    }
    return result;
}

const vector<shared_ptr<LoadSet>> Model::getActiveLoadSets() const {
    vector<shared_ptr<LoadSet>> result;
    set<shared_ptr<LoadSet>> set;
    for (const auto& analyse : analyses) {
        for (const auto& loadSet : analyse->getLoadSets()) {
            if (set.find(loadSet) == set.end()) {
                set.insert(loadSet);
                result.push_back(loadSet);
            }
        }
    }
    return result;
}

const vector<shared_ptr<ConstraintSet>> Model::getCommonConstraintSets() const {
    vector<shared_ptr<ConstraintSet>> result;
    map<shared_ptr<ConstraintSet>, int> map;
    for (const auto& analyse : analyses) {
        for (const auto& constraintSet : analyse->getConstraintSets()) {
            map[constraintSet] += 1;
        }
    }
    for (const auto& constraintSet : constraintSets) {
        auto it = map.find(constraintSet);
        if (it != map.end() && it->second == analyses.size())
            result.push_back(constraintSet);
    }
    return result;
}

const vector<shared_ptr<LoadSet>> Model::getCommonLoadSets() const {
    vector<shared_ptr<LoadSet>> result;
    map<shared_ptr<LoadSet>, int> map;
    for (const auto& analyse : analyses) {
        for (const auto& loadSet : analyse->getLoadSets()) {
            map[loadSet] += 1;
        }
    }
    for (const auto& loadSet : loadSets) {
        auto it = map.find(loadSet);
        // DLOAD is never common
        if (it != map.end() && it->second == analyses.size() && loadSet->type != LoadSet::Type::DLOAD)
            result.push_back(loadSet);
    }
    return result;
}

const set<shared_ptr<ConstraintSet>> Model::getUncommonConstraintSets() const {
    set<shared_ptr<ConstraintSet>> result;
    map<shared_ptr<ConstraintSet>, int> map;
    for (const auto& analyse : analyses) {
        for (const auto& constraintSet : analyse->getConstraintSets()) {
            map[constraintSet] += 1;
        }
    }
    for (const auto& constraintSet : constraintSets) {
        auto it = map.find(constraintSet);
        if (it != map.end() and it->second < analyses.size())
            result.insert(constraintSet);
    }
    return result;
}

const set<shared_ptr<LoadSet>> Model::getUncommonLoadSets() const {
    set<shared_ptr<LoadSet>> result;
    map<shared_ptr<LoadSet>, int> map;
    for (const auto& analyse : analyses) {
        for (const auto& loadSet : analyse->getLoadSets()) {
            map[loadSet] += 1;
        }
    }
    for (const auto& loadSet : loadSets) {
        auto it = map.find(loadSet);
        if ((it != map.end() and it->second < analyses.size()) and loadSet->type != LoadSet::Type::DLOAD)
            result.insert(loadSet);
    }
    return result;
}

void Model::generateDiscrets() {

    //rigid constraints
    shared_ptr<CellGroup> virtualDiscretTRGroup = nullptr;

    shared_ptr<CellGroup> virtualDiscretTGroup = nullptr;

    for (const Node& node : this->mesh.nodes) {
        DOFS missingDOFS;

        for (const auto& analysis : analyses) {

            const DOFS& requiredDOFS = analysis->findBoundaryDOFS(node.position);
            if (!node.dofs.containsAll(requiredDOFS)) {
                missingDOFS += requiredDOFS - node.dofs;
            }
        }

        DOFS addedDOFS;
        if (missingDOFS.size() != 0) {
            if (missingDOFS.containsAnyOf(DOFS::ROTATIONS)) {
                //extra dofs added by the DISCRET. They need to be blocked.
                addedDOFS = DOFS::ALL_DOFS - node.dofs - missingDOFS;
                if (virtualDiscretTRGroup == nullptr) {
                    const auto& virtualDiscretTR = make_shared<DiscretePoint>(*this, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
                    ostringstream oss;
                    oss << "created by generateDiscrets() because missing DOFs: " << missingDOFS << " and node: " << node;
                    virtualDiscretTRGroup = mesh.createCellGroup("VDiscrTR", Group::NO_ORIGINAL_ID, oss.str());
                    virtualDiscretTR->assignCellGroup(virtualDiscretTRGroup);
                    if (this->configuration.addVirtualMaterial) {
                        virtualDiscretTR->assignMaterial(getVirtualMaterial());
                    }
                    this->add(virtualDiscretTR);
                }
                vector<int> cellNodes;
                cellNodes.push_back(node.id);
                mesh.allowDOFS(node.position, DOFS::ALL_DOFS);
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::POINT1, cellNodes,
                        true);
                virtualDiscretTRGroup->addCellPosition(cellPosition);
            } else {
                addedDOFS = DOFS::TRANSLATIONS - node.dofs - missingDOFS;
                if (virtualDiscretTGroup == nullptr) {
                    const auto& virtualDiscretT = make_shared<DiscretePoint>(*this, 0.0, 0.0, 0.0);
                    ostringstream oss;
                    oss << "created by generateDiscrets() because missing DOFs: " << missingDOFS << " and node: " << node;
                    virtualDiscretTGroup = mesh.createCellGroup("VDiscrT", Group::NO_ORIGINAL_ID, oss.str());
                    virtualDiscretT->assignCellGroup(virtualDiscretTGroup);
                    if (this->configuration.addVirtualMaterial) {
                        virtualDiscretT->assignMaterial(getVirtualMaterial());
                    }
                    this->add(virtualDiscretT);
                }
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::POINT1, { node.id },
                        true);
                virtualDiscretTGroup->addCellPosition(cellPosition);
                mesh.allowDOFS(node.position, DOFS::TRANSLATIONS);
            }
        }

        for (const auto& analysis : analyses) {
            shared_ptr<ConstraintSet> spcSet = nullptr;

            const DOFS& requiredDOFS = analysis->findBoundaryDOFS(node.position);
            if (!node.dofs.containsAll(requiredDOFS)) {
                const DOFS& extraDOFS = addedDOFS - requiredDOFS - node.dofs;

                if (extraDOFS != DOFS::NO_DOFS) {
                    if (spcSet == nullptr) {
                        spcSet = make_shared<ConstraintSet>(*this, ConstraintSet::Type::SPC);
                        add(spcSet);
                    }
                    const auto& spc = make_shared<SinglePointConstraint>(*this, extraDOFS);
                    spc->addNodeId(node.id);
                    add(spc);
                    addConstraintIntoConstraintSet(*spc, *spcSet);
                    analysis->add(*spcSet);
                    if (configuration.logLevel >= LogLevel::DEBUG) {
                        cout << "Adding virtual spc on node: id: " << node.id << "for " << extraDOFS
                                << endl;
                    }
                }
            }
        }
    }
}

shared_ptr<Material> Model::getOrCreateMaterial(int material_id, bool createIfNotExists) {
    shared_ptr<Material> result = materials.find(material_id);
    if (not result and createIfNotExists) {
        result = make_shared<Material>(*this, material_id);
        this->add(result);
    }
    return result;
}

const CellContainer Model::getMaterialAssignment(int materialId) const {
    auto it = material_assignment_by_material_id.find(materialId);
    if (it != material_assignment_by_material_id.end()) {
        return it->second;
    } else {
        //return empty cell container if no assigmnent is found
        return CellContainer(mesh);
    }
}

void Model::assignMaterial(int material_id, const CellContainer& materialAssign) {
    auto it = material_assignment_by_material_id.find(material_id);
    if (it != material_assignment_by_material_id.end()) {
        it->second.add(materialAssign);
    } else {
        material_assignment_by_material_id.insert(make_pair(material_id, materialAssign));
    }
}

shared_ptr<Material> Model::getVirtualMaterial() {
    if (not virtualMaterial) {
        virtualMaterial = this->getOrCreateMaterial(Material::NO_ORIGINAL_ID);
        virtualMaterial->addNature(make_shared<ElasticNature>(*this, 1e-12, 0.0));
    }
    return virtualMaterial;
}

const vector<shared_ptr<Beam>> Model::getBeams() const {
    vector<shared_ptr<Beam>> result;
    for (const auto& elementSet : elementSets) {
        if (elementSet->isBeam()) {
            shared_ptr<Beam> beam = dynamic_pointer_cast<Beam>(elementSet);
            result.push_back(beam);
        }
    }
    return result;
}

const vector<shared_ptr<Beam>> Model::getTrusses() const {
    vector<shared_ptr<Beam>> result;
    for (const auto& elementSet : elementSets) {
        if (elementSet->isTruss()) {
            shared_ptr<Beam> bar = dynamic_pointer_cast<Beam>(elementSet);
            result.push_back(bar);
        }
    }
    return result;
}

bool Model::needsLargeDisplacements() const {
    double largeDisp = 0;
    auto it = this->parameters.find(Model::Parameter::LARGE_DISPLACEMENTS);
    if (it != this->parameters.end()) {
        largeDisp = it->second;
    }
    return not is_zero(largeDisp);
}

void Model::generateSkin() {

    for (const auto& loading : loadings) {
        if (not loading->isCellLoading()) {
            continue;
        }
        auto cellLoading = static_pointer_cast<CellLoading>(loading);
        cellLoading->createSkin();
    }

    for (const auto& target : targets) {
        if (not target->isCellTarget()) {
            continue;
        }
        auto cellTarget = static_pointer_cast<CellTarget>(target);
        cellTarget->createSkin();
    }

}

void Model::emulateLocalDisplacementConstraint() {
    // first pass : create LinearMultiplePoint constraints for each constraint that need it
    for (const auto& constraint : constraints) {
        if (constraint->type == Constraint::Type::SPC) {
            shared_ptr<SinglePointConstraint> spc = dynamic_pointer_cast<SinglePointConstraint>(
                    constraint);
            spc->emulateLocalDisplacementConstraint();
            if (spc->nodePositions().size() == 0)
                this->remove(spc->getReference());
        }
    }

}

void Model::emulateAdditionalMass() {
    vector<shared_ptr<ElementSet>> newElementSets;
    for (const auto& elementSet : elementSets) {
        double rho = elementSet->getAdditionalRho();
        if (!is_zero(rho)) {
            // copy elementSet
            const auto& newElementSet = elementSet->clone();
            newElementSet->resetId();
            newElementSets.push_back(newElementSet);
            // assign new material
            shared_ptr<Material> newMaterial = make_shared<Material>(*this);
            newMaterial->addNature(make_shared<ElasticNature>(*this, 0, 0, 0, rho));
            materials.add(newMaterial);
            newElementSet->assignMaterial(newMaterial);
            // copy and assign new cellGroup
            ostringstream oss;
            oss << "created by emulateAdditionalMass() because of elementSet: " << elementSet << " additional rho:" << rho;
            shared_ptr<CellGroup> newCellGroup = mesh.createCellGroup(
                    "VAM_" + to_string(newElementSets.size()), Group::NO_ORIGINAL_ID, oss.str());
            newElementSet->assignCellGroup(newCellGroup);
            vector<Cell> cells = elementSet->cellGroup->getCells();
            for (const auto& cell : cells) {
                int cellPosition = mesh.addCell(Cell::AUTO_ID, cell.type, cell.nodeIds, cell.isvirtual,
                        cell.cspos, cell.elementId);
                newCellGroup->addCellPosition(cellPosition);
            }
        }
    }
    for (const auto& elementSet : newElementSets)
        elementSets.add(elementSet);
}

void Model::generateBeamsToDisplayHomogeneousConstraint() {

    shared_ptr<CellGroup> virtualGroupRigid = nullptr;
    shared_ptr<CellGroup> virtualGroupRBE3 = nullptr;

    vector<shared_ptr<ConstraintSet>> activeConstraintSets = getActiveConstraintSets();
    for (const auto& constraintSet : activeConstraintSets) {
        set<shared_ptr<Constraint>> constraints = constraintSet->getConstraints();
        for (const auto& constraint : constraints) {
            switch (constraint->type) {
            case Constraint::Type::RIGID: {
                if (!virtualGroupRigid) {
                    const auto& virtualBeam = make_shared<CircularSectionBeam>(*this, 0.001, Beam::BeamModel::EULER, 0.0);
                    if (this->configuration.addVirtualMaterial) {
                        virtualBeam->assignMaterial(getVirtualMaterial());
                    }
                    ostringstream oss;
                    oss << "created by generateBeamsToDisplayHomogeneousConstraint() because of rigid constraint: " << constraint;
                    virtualGroupRigid = mesh.createCellGroup("VRigid", Group::NO_ORIGINAL_ID, oss.str());
                    virtualBeam->assignCellGroup(virtualGroupRigid);
                    this->add(virtualBeam);
                }
                shared_ptr<RigidConstraint> rigid = dynamic_pointer_cast<RigidConstraint>(
                        constraint);
                vector<int> nodes = { 0, 0 };
                nodes[0] = mesh.findNodeId(rigid->getMaster());
                mesh.allowDOFS(rigid->getMaster(), DOFS::ALL_DOFS);
                for (int slaveNode : rigid->getSlaves()) {
                    nodes[1] = mesh.findNodeId(slaveNode);
                    int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, nodes, true);
                    virtualGroupRigid->addCellPosition(cellPosition);
                    mesh.allowDOFS(slaveNode, DOFS::ALL_DOFS);
                }
                break;
            }
            case Constraint::Type::RBE3: {
                if (!virtualGroupRBE3) {
                    const auto& virtualBeam = make_shared<CircularSectionBeam>(*this, 0.001, Beam::BeamModel::EULER, 0.0);
                    if (configuration.addVirtualMaterial) {
                        virtualBeam->assignMaterial(getVirtualMaterial());
                    }
                    ostringstream oss;
                    oss << "created by generateBeamsToDisplayHomogeneousConstraint() because of rbe3 constraint: " << constraint;
                    virtualGroupRBE3 = mesh.createCellGroup("VRBE3", Group::NO_ORIGINAL_ID, oss.str());
                    virtualBeam->assignCellGroup(virtualGroupRBE3);
                    this->add(virtualBeam);
                }
                shared_ptr<RBE3> rbe3 = dynamic_pointer_cast<RBE3>(constraint);
                vector<int> nodes = { 0, 0 };
                nodes[0] = mesh.findNodeId(rbe3->getMaster());
                mesh.allowDOFS(rbe3->getMaster(), DOFS::ALL_DOFS);
                for (int slaveNode : rbe3->getSlaves()) {
                    nodes[1] = mesh.findNodeId(slaveNode);
                    int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, nodes, true);
                    mesh.allowDOFS(slaveNode, DOFS::ALL_DOFS);
                    virtualGroupRBE3->addCellPosition(cellPosition);
                }
                break;
            }
            default: {
            }
            }
        }
    }
}

void Model::generateMaterialAssignments() {
    if (configuration.partitionModel) {
        if (this->material_assignment_by_material_id.size() > 0) {
            cerr << "generateMaterialAssignments with PartitionModel is not "
                    << " yet implemented. " << endl
                    << "This method should partition the elementSets"
                    << " and assign materials to elementSets. Useful if output=Nastran" << endl;
        }
    } else {
        //generate or update material assignments from elementSets
        for (shared_ptr<ElementSet> element : elementSets) {
            if (element->material) {
                int mat_id = element->material->getId();
                auto it = material_assignment_by_material_id.find(mat_id);
                if (element->cellGroup != nullptr) {
                    if (it != material_assignment_by_material_id.end()) {
                        it->second.add(*(element->cellGroup));
                    } else {
                        CellContainer assignment(mesh);
                        assignment.add(*(element->cellGroup));
                        material_assignment_by_material_id.insert(make_pair(mat_id, assignment));
                    }
                }
            }
        }
    }
}

void Model::removeIneffectives() {
    // remove ineffective loadings from the model
    vector<shared_ptr<Loading>> loadingsToRemove;
    for (const auto& loading : loadings) {
        if (loading->ineffective())
            loadingsToRemove.push_back(loading);
    }
    for (const auto& loading : loadingsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG)
            cout << "Removed ineffective " << *loading << endl;
        remove(Reference<Loading>(*loading));
    }

    // remove empty loadSets from the model
    vector<Reference<LoadSet>> loadSetSetsToRemove;
    for (const auto& loadSet : this->loadSets) {
        if (loadSet->size() == 0) {
            loadSetSetsToRemove.push_back(loadSet->getReference());
        }
    }
    for (Reference<LoadSet> loadSetRef : loadSetSetsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG)
            cout << "Removed empty loadset " << loadSetRef << endl;
        remove(loadSetRef);
    }

    // remove ineffective constraints from the model
    vector<shared_ptr<Constraint>> constraintsToRemove;
    for (const auto& constraint : constraints) {
        if (constraint->ineffective())
            constraintsToRemove.push_back(constraint);
    }
    for (const auto& constraint : constraintsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG)
            cout << "Removed ineffective " << *constraint << endl;
        remove(Reference<Constraint>(*constraint));
    }

    // remove empty constraintSets from the model
    vector<Reference<ConstraintSet>> constraintSetSetsToRemove;
    for (const auto& constraintSet : this->constraintSets) {
        if (constraintSet->empty()) {
            constraintSetSetsToRemove.push_back(constraintSet->getReference());
        }
    }
    for (Reference<ConstraintSet> constraintSetRef : constraintSetSetsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG)
            cout << "Removed empty " << constraintSetRef << endl;
        remove(constraintSetRef);
    }

    // remove empty elementSets from the model
    vector<shared_ptr<ElementSet>> elementSetsToRemove;
    for (const auto& elementSet : elementSets) {
        if (elementSet->cellGroup and elementSet->cellGroup->empty())
            elementSetsToRemove.push_back(elementSet);
    }
    for (const auto& elementSet : elementSetsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG)
            cout << "Removed empty " << *elementSet << endl;
        this->elementSets.erase(Reference<ElementSet>(*elementSet));
    }
}

void Model::removeUnassignedMaterials() {
    vector<shared_ptr<Material>> materialsToRemove;
    //for (const auto& material : materials) {
        /*
         * if the model is configured to assign materials to cells directly check
         * that for every material at lest a cell or cellgroup is assigned
         */
        //if (!configuration.partitionModel and material->getAssignment().empty())
        //        materialsToRemove.push_back(material);
    //}
    for (const auto& material : materialsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG)
            cout << "Removed unassigned " << *material << endl;
        this->materials.erase(Reference<Material>(*material));
    }
}

void Model::replaceCombinedLoadSets() {
    for (const auto& loadSet : this->loadSets) {
        for (const auto& kv : loadSet->embedded_loadsets) {
            shared_ptr<LoadSet> otherloadSet = this->find(kv.first);
            if (!otherloadSet) {
                throw logic_error("CombinedLoadSet: missing loadSet " + to_str(kv.first));
            }
            double coefficient = kv.second;
            for (shared_ptr<Loading> loading : otherloadSet->getLoadings()) {
                shared_ptr<Loading> newLoading = loading->clone();
                newLoading->resetId();
                newLoading->scale(coefficient);
                this->add(newLoading);
                this->addLoadingIntoLoadSet(*newLoading, *loadSet);
                if (configuration.logLevel >= LogLevel::DEBUG) {
                    cout << "Cloned " << *loading << " into " << *newLoading << " and scaled by "
                            << coefficient << " and assigned to " << *loadSet << endl;
                }
            }
        }
        loadSet->embedded_loadsets.clear();
    }
}

void Model::removeAssertionsMissingDOFS()
{
    vector<shared_ptr<Objective> > objectivesToRemove;
    for (const auto& analysis : analyses) {
        for(const auto& assertion : analysis->getAssertions()) {
            for(int nodePosition: assertion->nodePositions()) {
                const DOFS& assertionDOFS = assertion->getDOFSForNode(nodePosition);
                if (assertionDOFS.size() >= 1) {
                    const Node& node = mesh.findNode(nodePosition);
                    const DOFS& availableDOFS = node.dofs + analysis->findBoundaryDOFS(nodePosition);
                    if (!availableDOFS.containsAll(assertionDOFS)) {
                        objectivesToRemove.push_back(assertion);
                    }
                }
            }
        }
    }
    for (const auto& objective : objectivesToRemove) {
        if (configuration.logLevel >= LogLevel::TRACE)
            cout << "Removed ineffective " << *objective << endl;

        remove(Reference<Objective>(*objective));
    }
}

void Model::addDefaultAnalysis()
{
    if (this->analyses.size() == 0 && (loadings.size() > 0 || constraints.size() > 0)) {
        //add an automatic linear analysis
        const auto& analysis = make_shared<LinearMecaStat>(*this, "", 1);
        add(analysis);
        if (this->configuration.logLevel >= LogLevel::DEBUG) {
            cout << "Default linear analysis added." << endl;
        }
    }
}

void Model::replaceDirectMatrices()
{
    vector<shared_ptr<ElementSet> > elementSetsToRemove;
    int matrix_count = 0;
    map<int, DOFS> addedDofsByNode;
    map<int, DOFS> requiredDofsByNode;
    map<int, DOFS> ownedDofsByNode;
    for (const auto& elementSetM : elementSets) {
        if (!elementSetM->isMatrixElement()) {
            continue;
        }
        shared_ptr<MatrixElement> matrix = dynamic_pointer_cast<MatrixElement>(elementSetM);
        for (int nodePosition : matrix->nodePositions()) {
            requiredDofsByNode[nodePosition] = DOFS();
            const int nodeId = mesh.findNodeId(nodePosition);
            DOFS owned;
            for (const auto elementSetI : elementSets) {
                if (elementSetI->cellGroup == nullptr) {
                    continue;
                }
                for (const Cell& cell : elementSetI->cellGroup->getCells()) {
                    for (int cellNodeId : cell.nodeIds) {
                        if (cellNodeId == nodeId) {
                            if (elementSetI->isBeam() or elementSetI->isShell()) {
                                owned += DOFS::ALL_DOFS;
                            } else {
                                owned += DOFS::TRANSLATIONS;
                            }
                            break;
                        }
                    }
                }
            }
            ownedDofsByNode[nodePosition] = owned;
        }
        for (const auto& pair : matrix->nodePairs()) {
            if (pair.first == pair.second) {
                if (matrix->findInPairs(pair.first).size() != 0) {
                    continue; // will be handled by a segment cell with another node
                }
                // single node
                int nodePosition = pair.first;
                const int nodeId = mesh.findNodeId(nodePosition);
                DOFS requiredDofs = requiredDofsByNode.find(nodePosition)->second;
                shared_ptr<DOFMatrix> submatrix = matrix->findSubmatrix(nodePosition, nodePosition);
                const auto& discrete = make_shared<DiscretePoint>(*this);
                for (const auto& kv : submatrix->componentByDofs) {
                    double value = kv.second;
                    const vega::DOF& dof1 = kv.first.first;
                    const vega::DOF& dof2 = kv.first.second;
                    if (!is_equal(value, 0)) {
                        switch (matrix->type) {
                        case ElementSet::Type::STIFFNESS_MATRIX:
                            discrete->addStiffness(dof1, dof2, value);
                            break;
                        default:
                            throw logic_error("Not yet implemented");
                        }
                        requiredDofs += dof1;
                        requiredDofs += dof2;
                    }
                }
                if (configuration.addVirtualMaterial) {
                    discrete->assignMaterial(getVirtualMaterial());
                }
                matrix_count++;
                ostringstream oss;
                oss << "created by replaceDirectMatrices() because of matrix element on same node: " << matrix;
                shared_ptr<CellGroup> matrixGroup = mesh.createCellGroup(
                        "MTN" + to_string(matrix_count), Group::NO_ORIGINAL_ID, oss.str());
                discrete->assignCellGroup(matrixGroup);
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, { nodeId }, true);
                matrixGroup->addCellPosition(cellPosition);
                if (discrete->hasRotations()) {
                    addedDofsByNode[nodePosition] = DOFS::ALL_DOFS;
                    mesh.allowDOFS(nodePosition, DOFS::ALL_DOFS);
                } else {
                    addedDofsByNode[nodePosition] = DOFS::TRANSLATIONS;
                    mesh.allowDOFS(nodePosition, DOFS::TRANSLATIONS);
                }
                if (this->configuration.logLevel >= LogLevel::DEBUG) {
                    cout << "Creating discrete : " << *discrete << " over node id : "
                            << to_string(nodeId) << endl;
                }
                this->add(discrete);
            } else {
                // node couple
                int rowNodePosition = pair.first;
                int colNodePosition = pair.second;
                const int rowNodeId = mesh.findNodeId(rowNodePosition);
                const int colNodeId = mesh.findNodeId(colNodePosition);
                DOFS requiredRowDofs = requiredDofsByNode.find(rowNodePosition)->second;
                DOFS requiredColDofs = requiredDofsByNode.find(colNodePosition)->second;

                const auto& discrete = make_shared<DiscreteSegment>(*this);
                ostringstream oss;
                oss << "created by replaceDirectMatrices() because of matrix element on node couple: " << matrix;
                shared_ptr<CellGroup> matrixGroup = mesh.createCellGroup(
                        "MTL" + to_string(matrix_count), Group::NO_ORIGINAL_ID, oss.str());
                matrix_count++;
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, { rowNodeId,
                        colNodeId }, true);
                matrixGroup->addCellPosition(cellPosition);
                if (configuration.addVirtualMaterial) {
                    discrete->assignMaterial(getVirtualMaterial());
                }
                discrete->assignCellGroup(matrixGroup);
                if (discrete->hasRotations()) {
                    addedDofsByNode[rowNodePosition] = DOFS::ALL_DOFS;
                    mesh.allowDOFS(rowNodePosition, DOFS::ALL_DOFS);
                    addedDofsByNode[colNodePosition] = DOFS::ALL_DOFS;
                    mesh.allowDOFS(colNodePosition, DOFS::ALL_DOFS);
                } else {
                    addedDofsByNode[rowNodePosition] = DOFS::TRANSLATIONS;
                    mesh.allowDOFS(rowNodePosition, DOFS::TRANSLATIONS);
                    addedDofsByNode[colNodePosition] = DOFS::TRANSLATIONS;
                    mesh.allowDOFS(colNodePosition, DOFS::TRANSLATIONS);
                }
                for (int row_index = 0; row_index < 2; ++row_index) {
                    for (int col_index = 0; col_index < 2; ++col_index) {
                        int rowNodePosition2;
                        int colNodePosition2;
                        if (row_index == 0) {
                            rowNodePosition2 = rowNodePosition;
                        } else {
                            rowNodePosition2 = colNodePosition;
                        }
                        if (col_index == 0) {
                            colNodePosition2 = rowNodePosition;
                        } else {
                            colNodePosition2 = colNodePosition;
                        }
                        shared_ptr<DOFMatrix> submatrix = matrix->findSubmatrix(rowNodePosition2,
                                colNodePosition2);
                        for (const auto& kv : submatrix->componentByDofs) {
                            // We are disassembling the matrix, so we must divide the value by the segments
                            double value = kv.second / static_cast<int>(matrix->findInPairs(pair.first).size());
                            const DOF& rowDof = kv.first.first;
                            const DOF& colDof = kv.first.second;
                            if (!is_equal(value, 0)) {
                                switch (matrix->type) {
                                case ElementSet::Type::STIFFNESS_MATRIX:
                                    discrete->addStiffness(row_index, col_index, rowDof, colDof,
                                            value);
                                    break;
                                default:
                                    throw logic_error("Not yet implemented");
                                }
                                requiredRowDofs += rowDof;
                                requiredColDofs += colDof;
                            }
                        }
                    }
                }
                if (this->configuration.logLevel >= LogLevel::DEBUG) {
                    cout << "Creating discrete : " << *discrete << " over node ids : "
                            << to_string(rowNodeId) << " and : " << to_string(colNodeId) << endl;
                }
                this->add(discrete);
            }
        }
        elementSetsToRemove.push_back(elementSetM);
    }
    for (const auto& kv : addedDofsByNode) {
        int nodePosition = kv.first;
        const int nodeId = this->mesh.findNodeId(nodePosition);
        const DOFS& added = kv.second;
        DOFS required;
        auto it = requiredDofsByNode.find(nodePosition);
        if (it == requiredDofsByNode.end()) {
            required = DOFS::NO_DOFS;
        } else {
            required = it->second;
        }

        DOFS owned;
        auto it2 = ownedDofsByNode.find(nodePosition);
        if (it2 == ownedDofsByNode.end()) {
            owned = DOFS::NO_DOFS;
        } else {
            owned = it2->second;
        }

        for (const auto loading : loadings) {
            for (int nodePosition2 : loading->nodePositions()) {
                required += loading->getDOFSForNode(nodePosition2);
            }
        }
        for (const auto constraint : constraints) {
            set<int> constraintNodes = constraint->nodePositions();
            if (constraintNodes.find(nodePosition) == constraintNodes.end()) {
                continue;
            }
            required += constraint->getDOFSForNode(nodePosition);
        }
        const DOFS& extra = added - owned - required;
        if (extra != DOFS::NO_DOFS) {
            const auto& spc = make_shared<SinglePointConstraint>(*this, extra);
            spc->addNodeId(nodeId);
            add(spc);
            addConstraintIntoConstraintSet(*spc, *commonConstraintSet);
            if (configuration.logLevel >= LogLevel::DEBUG) {
                cout << "Adding virtual spc on node id: " << nodeId << "for " << extra
                        << endl;
            }
        }
    }
    for (const auto& elementSet : elementSetsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG) {
            cout << "Replaced " << *elementSet << endl;
        }
        this->elementSets.erase(elementSet->getReference());
    }
}

void Model::replaceRigidSegments()
{
    for (const auto& elementSet : elementSets.filter(ElementSet::Type::STRUCTURAL_SEGMENT)) {
        shared_ptr<StructuralSegment> segment = dynamic_pointer_cast<StructuralSegment>(elementSet);
        if (not segment->isDiagonalRigid()) {
            continue;
        }
        for (const auto& cell : segment->cellGroup->getCells()) {
            const int masterId = cell.nodeIds.front();
            const set<int> slaveNodeIds{cell.nodeIds.begin()+1, cell.nodeIds.end()};
            const auto& rigid = make_shared<RigidConstraint>(*this, masterId, RigidConstraint::NO_ORIGINAL_ID, slaveNodeIds);
            this->add(rigid);
            addConstraintIntoConstraintSet(*rigid, *commonConstraintSet);
            if (configuration.logLevel >= LogLevel::DEBUG) {
                cout << "Added contraint: " << *rigid << " to replace a cell of: " << *segment << endl;
            }
        }

        segment->setAllZero(); // make the rigid segment... no more rigid.
    }
}

void Model::removeRedundantSpcs() {
    for (const auto& analysis : this->analyses) {
        unordered_map<pair<int, DOF>, double, boost::hash<pair<int, int> > > spcvalueByNodeAndDof;
        for (const auto& constraintSet : analysis->getConstraintSets()) {
            const set<shared_ptr<Constraint> > spcs = constraintSet->getConstraintsByType(
                    Constraint::Type::SPC);
            if (spcs.size() == 0) {
                continue;
            }
            for (shared_ptr<Constraint> constraint : spcs) {
                shared_ptr<SinglePointConstraint> spc = dynamic_pointer_cast<SinglePointConstraint>(
                        constraint);
                for (int nodePosition : spc->nodePositions()) {
                    DOFS dofsToRemove;
                    DOFS blockedDofs = spc->getDOFSForNode(nodePosition);
                    for (const DOF dof : blockedDofs) {
                        pair<int, DOF> key = make_pair(nodePosition, dof);
                        double spcValue = spc->getDoubleForDOF(dof);
                        auto entry = spcvalueByNodeAndDof.find(key);
                        if (entry == spcvalueByNodeAndDof.end()) {
                            spcvalueByNodeAndDof[key] = spcValue;
                        } else if (!is_equal(spcValue, entry->second)) {
                            const int nodeId = this->mesh.findNodeId(nodePosition);
                            throw logic_error(
                                    "In analysis : " + to_str(*analysis) + ", spc : " + to_str(*spc)
                                            + " value : " + to_string(spcValue)
                                            + " different by other spc value : "
                                            + to_string(entry->second) + " on same node id : "
                                            + to_string(nodeId) + " and dof : " + dof.label);
                        } else {
                            dofsToRemove = dofsToRemove + dof;
                        }
                    }
                    if (dofsToRemove.size() >= 1) {
                        analysis->removeSPCNodeDofs(*spc, nodePosition, dofsToRemove);
                        if (configuration.logLevel >= LogLevel::DEBUG) {
                            cout << "Removed redundant dofs : " << dofsToRemove << " from node id : " << this->mesh.findNodeId(nodePosition)
                                    << " from spc : " << *spc << " for analysis : " << *analysis << endl;
                        }
                    }
                }
            }
        }
    }
}

void Model::removeConstrainedImposed() {
    for (const auto& analysis : this->analyses) {
        unordered_map<int, DOFS> imposedDofsByNodeId;
        for (const auto& loadSet : analysis->getLoadSets()) {
            const set<shared_ptr<Loading> > loads = loadSet->getLoadingsByType(
                    Loading::Type::IMPOSED_DISPLACEMENT);
            if (loads.size() == 0) {
                continue;
            }
            for (shared_ptr<Loading> load : loads) {
                shared_ptr<ImposedDisplacement> impo = dynamic_pointer_cast<ImposedDisplacement>(
                        load);
                for (int nodePosition : impo->nodePositions()) {
                    imposedDofsByNodeId[nodePosition] = impo->getDOFSForNode(nodePosition);
                }
            }
        }
        for (const auto& constraintSet : analysis->getConstraintSets()) {
            const set<shared_ptr<Constraint> > spcs = constraintSet->getConstraintsByType(
                    Constraint::Type::SPC);
            if (spcs.size() == 0) {
                continue;
            }
            for (shared_ptr<Constraint> constraint : spcs) {
                shared_ptr<SinglePointConstraint> spc = dynamic_pointer_cast<SinglePointConstraint>(
                        constraint);
                for (int nodePosition : spc->nodePositions()) {
                    DOFS imposedDofs = imposedDofsByNodeId[nodePosition];
                    DOFS blockedDofs = spc->getDOFSForNode(nodePosition);
                    DOFS dofsToRemove = imposedDofs.intersection(blockedDofs);
                    if (dofsToRemove.size() >= 1) {
                        analysis->removeSPCNodeDofs(*spc, nodePosition, dofsToRemove);
                        if (configuration.logLevel >= LogLevel::DEBUG) {
                            cout << "Removed imposed dofs : " << dofsToRemove << " from node id : " << this->mesh.findNodeId(nodePosition)
                                    << " from spc : " << *spc << " for analysis : " << *analysis << endl;
                        }
                    }
                }
            }
        }
    }
}


void Model::splitDirectMatrices(const unsigned int sizeMax){


    if (sizeMax<2){
        throw logic_error("Model can't split matrices to a size under 2.");
    }

    vector<shared_ptr<ElementSet>> esToErase;
    vector<shared_ptr<ElementSet>> esToAdd;
    const int sizeStack = static_cast<int>(sizeMax/2);

    for (const auto& elementSetM : elementSets) {
        if (!elementSetM->isMatrixElement()) {
            //TODO: Display informative message in debug mode.
            continue;
        }

        shared_ptr<MatrixElement> matrix = dynamic_pointer_cast<MatrixElement>(elementSetM);

        // If matrix is small enough, we do nothing
        if (matrix->nodePositions().size()<=sizeMax){
            //TODO: Display informative in debug mode.
            continue;
        }

        // It's too big, we have work to do
        esToErase.push_back(elementSetM);
        map<int, int> nodeIdOfElement;
        for (const auto& v : matrix->nodePositions()){
            nodeIdOfElement[v] = mesh.findNodeId(v);
        }

        // Dummy ElementSet for the splitting
        shared_ptr<ElementSet> dummyElement = elementSetM->clone();
        shared_ptr<MatrixElement> dummyMatrix = dynamic_pointer_cast<MatrixElement>(dummyElement);
        dummyMatrix->clear();


        map<int,int> stackOfNodesByNodes;
        map<pair<int, int>, shared_ptr<ElementSet>>  esToAddByStackNumber;

        // Splitting the matrices, pairs of nodes by pairs of node (I,J).
        for (const auto np : matrix->nodePairs()){

            // We attribute a stack of sizeStack to each node.
            int sI, sJ;
            auto it = stackOfNodesByNodes.find(np.first);
            if (it == stackOfNodesByNodes.end()){
                sI = static_cast<int>(stackOfNodesByNodes.size()/sizeStack);
                stackOfNodesByNodes[np.first]=sI;
            }else{
                sI = it->second;
            }
            it = stackOfNodesByNodes.find(np.second);
            if (it == stackOfNodesByNodes.end()){
                sJ = static_cast<int>(stackOfNodesByNodes.size()/sizeStack);
                stackOfNodesByNodes[np.second]=sJ;
            }else{
                sJ = it->second;
            }

            // We attribute a elementSet to the pair (sI, sJ), and create it if needed
            pair<int, int> ps;
            if (sI<sJ){
                ps = make_pair(sI,sJ);
            }else{
                ps = make_pair(sJ,sI);
            }
            auto it2 = esToAddByStackNumber.find(ps);
            shared_ptr<ElementSet> newElementSet = nullptr;
            if (it2 == esToAddByStackNumber.end()){

                newElementSet = dummyElement->clone();
                newElementSet->resetId();
                esToAdd.push_back(newElementSet);
                esToAddByStackNumber[ps]=newElementSet;

                // Special add for the submatrices on the diagonal:
                // One submatrix corresponds to 3 pairs of stacks id
                int stF = ps.first - ps.first%2;
                int stS = ps.second - ps.second%2;
                if (stF == stS){
                    esToAddByStackNumber[make_pair(stF,stF)]=newElementSet;
                    esToAddByStackNumber[make_pair(stF,stF+1)]=newElementSet;
                    esToAddByStackNumber[make_pair(stF+1,stF+1)]=newElementSet;
                }
            }else{
                newElementSet = it2->second;
            }

            // We copy the values
            shared_ptr<MatrixElement> nM = dynamic_pointer_cast<MatrixElement>(newElementSet);
            shared_ptr<DOFMatrix> dM = matrix->findSubmatrix(np.first, np.second);
            for (const auto dof: dM->componentByDofs){
                nM->addComponent(nodeIdOfElement[np.first], dof.first.first, nodeIdOfElement[np.second], dof.first.second, dof.second);
            }

        }

        if (configuration.logLevel >= LogLevel::DEBUG) {
            cout << "Element Matrix "<<matrix->bestId()<< " has been split into the smaller matrices ";
            for (const auto& es : esToAddByStackNumber){
                cout << es.second->bestId()<<" ";
            }
            cout <<endl;
        }

    }

    // We remove the elementSets corresponding to big matrices
    for (const auto& es : esToErase){
        this->elementSets.erase(*es);
    }

    // We add the elementSets corresponding to small matrices
    for (const auto& es : esToAdd){
        this->add(es);
    }
}



void Model::makeCellsFromDirectMatrices(){

    int idM=0;
    for (const auto& elementSetM : elementSets) {
        if (!elementSetM->isMatrixElement()) {
            //TODO: Display informative message in debug mode.
            continue;
        }
        // If cells already exists, we do nothing
        if (elementSetM->cellGroup != nullptr){
            //TODO: Display informative message in debug mode.
            continue;
        }

        shared_ptr<MatrixElement> matrix = dynamic_pointer_cast<MatrixElement>(elementSetM);

        // If matrix is void, we do nothing (should not happen)
        if (matrix->nodePositions().size()==0){
            //TODO: Display informative message in debug mode.
            continue;
        }

        // Create a Cell Group
        idM++;
        shared_ptr<CellGroup> matrixGroup = mesh.createCellGroup("DM" + to_string(idM), Group::NO_ORIGINAL_ID, "Direct Matrix "+ elementSetM->name);
        matrix->assignCellGroup(matrixGroup);

        // Create a Cell and add it to the Cell Group
        CellType cellType = CellType::polyType(static_cast<unsigned int>(matrix->nodePositions().size()));
        vector<int> vNodeIds;
        for (int nodePosition : matrix->nodePositions()){
            const int nodeId = mesh.findNodeId(nodePosition);
            vNodeIds.push_back(nodeId);
        }

        int cellPosition = mesh.addCell(Cell::AUTO_ID, cellType, vNodeIds, true);
        matrixGroup->addCellPosition(cellPosition);

        if (configuration.logLevel >= LogLevel::DEBUG){
           cout << "Built cells, in cellgroup "<<matrixGroup->getName()<<", for Matrix Elements in "<< elementSetM->name<<"."<<endl;
        }

    }
}


void Model::makeCellsFromLMPC(){

    shared_ptr<Material> materialLMPC= nullptr;

    for (const auto& analysis : this->analyses) {
        map< vector<DOFCoefs>, shared_ptr<CellGroup>> groupBySetOfCoefs;
        for (const auto& constraintSet : analysis->getConstraintSets()) {

            set<shared_ptr<Constraint>> constraints = constraintSet->getConstraintsByType(Constraint::Type::LMPC);
            for (const auto& constraint : constraints) {
                const shared_ptr<LinearMultiplePointConstraint> lmpc = dynamic_pointer_cast<LinearMultiplePointConstraint>(constraint);

                // We sort the Coeffs in order to fuse various LMPC into the same ElementSet/CellGroup
                vector<int> sortedNodesPosition= lmpc->sortNodePositionByCoefs();
                vector<DOFCoefs> sortedCoefs;
                for (int n : sortedNodesPosition){
                    sortedCoefs.push_back(lmpc->getDoFCoefsForNode(n));
                }

                // Looking for the CellGroup corresponding to the current DOFCoefs.
                shared_ptr<CellGroup> group =nullptr;
                const auto it = groupBySetOfCoefs.find(sortedCoefs);
                if(it != groupBySetOfCoefs.end()){
                    group = it->second;
                }else{
                    // If not found, creating an ElementSet, a CellGroup and a (single) dummy rigid material
                    if (materialLMPC==nullptr){
                        materialLMPC = make_shared<Material>(*this);
                        materialLMPC->addNature(make_shared<RigidNature>(*this, 1));
                        this->add(materialLMPC);
                    }
                    group = mesh.createCellGroup("MPC_"+to_string(analysis->bestId())+"_"+to_string(constraint->bestId()), CellGroup::NO_ORIGINAL_ID, "MPC");
                    const auto& elementsetLMPC = make_shared<Lmpc>(*this, analysis->getId());
                    elementsetLMPC->assignCellGroup(group);
                    elementsetLMPC->assignMaterial(materialLMPC);
                    elementsetLMPC->assignDofCoefs(sortedCoefs);
                    this->add(elementsetLMPC);
                    groupBySetOfCoefs[sortedCoefs]= group;
                }

                // Creating a cell and adding it to the CellGroup
                vector<int> nodes;
                for (int position : sortedNodesPosition){
                    const int nodeId = mesh.findNodeId(position);
                    nodes.push_back(nodeId);
                }
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::polyType(static_cast<unsigned int>(nodes.size())), nodes, true);
                group->addCellPosition(cellPosition);
                if (configuration.logLevel >= LogLevel::DEBUG){
                    cout << "Building cells in group "<<group->getName()<<" from "<< *lmpc<<"."<<endl;
                }
            }

        }
    }

    // After the translation, we remove all LMPC constraints
    // We don't do this during the loop because some LMPC may be used by several analysis.
    for (const auto& analysis : this->analyses) {
        for (const auto& constraintSet : analysis->getConstraintSets()) {
            const int idConstraintSet = constraintSet->getId();
            const int originalIdConstraintSet = constraintSet->getOriginalId();
            const ConstraintSet::Type natConstraintSet = constraintSet->type;
            set<shared_ptr<Constraint>> constraints = constraintSet->getConstraintsByType(Constraint::Type::LMPC);
            for (const auto& constraint : constraints) {
                remove(constraint->getReference(), idConstraintSet, originalIdConstraintSet, natConstraintSet);
            }
        }
    }
}

void Model::makeCellsFromRBE(){

    vector<shared_ptr<ConstraintSet>> commonConstraintSets = this->getCommonConstraintSets();

    for (const auto& constraintSet : commonConstraintSets) {

        const int idConstraintSet = constraintSet->getId();
        const int originalIdConstraintSet = constraintSet->getOriginalId();
        const ConstraintSet::Type natConstraintSet = constraintSet->type;


        // Translation of RBAR and RBE2 (RBE2 are viewed as an assembly of RBAR)
        // See Systus Reference Analysis Manual: RIGID BODY Element (page 498)
        set<shared_ptr<Constraint>> constraints = constraintSet->getConstraintsByType(Constraint::Type::RIGID);
        vector<shared_ptr<Constraint>> toBeRemoved;
        for (const auto& constraint : constraints) {
            const shared_ptr<RigidConstraint> rbe2 = dynamic_pointer_cast<RigidConstraint>(constraint);

            // Creating an elementset, a CellGroup and a dummy rigid material
            shared_ptr<Material> materialRBE2 = make_shared<Material>(*this);
            materialRBE2->addNature(make_shared<RigidNature>(*this, 1));
            this->add(materialRBE2);

            shared_ptr<CellGroup> group = mesh.createCellGroup("RBE2_"+to_string(constraint->bestId()), CellGroup::NO_ORIGINAL_ID, "RBE2");
            const auto& elementsetRbe2 = make_shared<Rbar>(*this, mesh.findNodeId(rbe2->getMaster()));
            elementsetRbe2->assignCellGroup(group);
            elementsetRbe2->assignMaterial(materialRBE2);
            this->add(elementsetRbe2);

            // Creating cells and adding them to the CellGroup
            const int masterId = mesh.findNodeId(rbe2->getMaster());
            for (int position : rbe2->getSlaves()){
                const int slaveId = mesh.findNodeId(position);
                vector<int> nodes = {masterId, slaveId};
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, nodes, true);
                group->addCellPosition(cellPosition);
            }

            // Removing the constraint from the model.
            toBeRemoved.push_back(constraint);
            if (configuration.logLevel >= LogLevel::DEBUG){
                cout << "Building cells in cellgroup "<<group->getName()<<" from "<< *rbe2<<"."<<endl;
            }
        }

        constraints = constraintSet->getConstraintsByType(Constraint::Type::QUASI_RIGID);
        for (const auto& constraint : constraints) {
            shared_ptr<QuasiRigidConstraint> rbar = dynamic_pointer_cast<QuasiRigidConstraint>(constraint);

            if (!(rbar->isCompletelyRigid())){
                cerr << "QUASI_RIGID constraint not available yet. Constraint "+to_string(constraint->bestId())+ " translated as rigid constraint."<<endl;
            }
            if (rbar->getSlaves().size()!=2){
               throw logic_error("QUASI_RIGID constraint must have exactly two slaves.");
            }

            // Creating an elementset, a CellGroup and a dummy rigid material
            shared_ptr<Material> materialRBAR = make_shared<Material>(*this);
            materialRBAR->addNature(make_shared<RigidNature>(*this, 1));
            this->add(materialRBAR);

            // Master Node : first one. Slave Node : second and last one
            const int masterNodeId = mesh.findNodeId(*rbar->getSlaves().begin());
            const int slaveNodeId = mesh.findNodeId(*rbar->getSlaves().rbegin());
            shared_ptr<CellGroup> group = mesh.createCellGroup("RBAR_"+to_string(constraint->bestId()), CellGroup::NO_ORIGINAL_ID, "RBAR");

            const auto& elementsetRBAR = make_shared<Rbar>(*this, masterNodeId);
            elementsetRBAR->assignCellGroup(group);
            elementsetRBAR->assignMaterial(materialRBAR);
            this->add(elementsetRBAR);

            // Creating a cell and adding it to the CellGroup
            vector<int> nodes = {masterNodeId, slaveNodeId};
            int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, nodes, true);
            group->addCellPosition(cellPosition);

            // Removing the constraint from the model.
            toBeRemoved.push_back(constraint);
            if (configuration.logLevel >= LogLevel::DEBUG){
                cout << "Building cells in cellgroup "<<group->getName()<<" from "<< *rbar<<"."<<endl;
            }
        }


        /* Translation of RBE3
         * See Systus Reference Analysis Manual, Section 8.8 "Special Elements",
         * Subsection "Use of Averaging Type Solid Elements", p500.
         */
        constraints = constraintSet->getConstraintsByType(Constraint::Type::RBE3);
        for (const auto& constraint : constraints) {

            const shared_ptr<RBE3>& rbe3 = dynamic_pointer_cast<RBE3>(constraint);
            const int masterId = mesh.findNodeId(rbe3->getMaster());
            const DOFS& mDOFS = rbe3->getDOFS();

            int nbParts=0;
            map<DOFS, map<double, shared_ptr<CellGroup>>> groupByCoefByDOFS;

            for (int position : rbe3->getSlaves()){

                const int slaveId = mesh.findNodeId(position);
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, {masterId, slaveId}, true);

                /* We build a material for each value of "Slave DOFS" and "Slave Coeff" */
                const DOFS& sDOFS = rbe3->getDOFSForNode(position);
                const double sCoef = rbe3->getCoefForNode(position);

                shared_ptr<CellGroup> groupRBE3 = nullptr;
                auto it = groupByCoefByDOFS.find(sDOFS);
                if (it !=groupByCoefByDOFS.end()){
                    auto it2 = it->second.find(sCoef);
                    if (it2 != it->second.end())
                        groupRBE3 = it2->second;
                }

                if (groupRBE3==nullptr){

                    // Creating an elementset, a CellGroup and a dummy rigid material
                    nbParts++;
                    shared_ptr<Material> materialRBE3 = make_shared<Material>(*this);
                    materialRBE3->addNature(make_shared<RigidNature>(*this, Nature::UNAVAILABLE_DOUBLE, sCoef));
                    this->add(materialRBE3);

                    shared_ptr<CellGroup> group = mesh.createCellGroup("RBE3_"+to_string(nbParts)+"_"+to_string(constraint->bestId()), CellGroup::NO_ORIGINAL_ID, "RBE3");
                    const auto& elementsetRbe3 = make_shared<Rbe3>(*this, masterId, mDOFS, sDOFS);
                    elementsetRbe3->assignCellGroup(group);
                    elementsetRbe3->assignMaterial(materialRBE3);
                    this->add(elementsetRbe3);

                    if (configuration.logLevel >= LogLevel::DEBUG){
                        cout << "Building cells in CellGroup "<<group->getName()<<" from "<< *rbe3<<"."<<endl;
                    }
                    groupByCoefByDOFS[sDOFS][sCoef]= group;
                }
                groupRBE3 = groupByCoefByDOFS[sDOFS][sCoef];
                groupRBE3->addCellPosition(cellPosition);
            }

            // Removing the constraint from the model.
            toBeRemoved.push_back(constraint);
        }

        for(const auto& constraint: toBeRemoved) {
            remove(constraint->getReference(), idConstraintSet, originalIdConstraintSet, natConstraintSet);
        }
    }
}

void Model::makeCellsFromSurfaceSlide() {
    namespace bg = boost::geometry;
    namespace bgi = boost::geometry::index;
    using node_point = bg::model::point<double, 3, bg::cs::cartesian>;
    using node_polygon = bg::model::polygon<node_point >;
    using node_box = bg::model::box<node_point>;
    using value = pair<node_box, size_t>;
    for (const auto& constraintSet : this->getCommonConstraintSets()) {

        const int idConstraintSet = constraintSet->getId();
        const int originalIdConstraintSet = constraintSet->getOriginalId();
        const ConstraintSet::Type natConstraintSet = constraintSet->type;
        auto constraints = constraintSet->getConstraintsByType(Constraint::Type::SURFACE_SLIDE_CONTACT);

        vector<shared_ptr<Constraint>> toBeRemoved;
        for (const auto& constraint : constraints) {
            shared_ptr<CellGroup> group = mesh.createCellGroup("SURF_"+to_string(constraint->bestId()), CellGroup::NO_ORIGINAL_ID, "SURF");
            const auto& elementsetSlide = make_shared<SurfaceSlideSet>(*this);
            elementsetSlide->assignCellGroup(group);
            this->add(elementsetSlide);

            shared_ptr<const SurfaceSlide> surface = dynamic_pointer_cast<const SurfaceSlide>(constraint);
		    shared_ptr<const BoundaryElementFace> masterSurface = dynamic_pointer_cast<const BoundaryElementFace>(this->find(surface->master));
		    vector<node_polygon> masterFaces;
		    vector<vector<int>> masterFaceNodeIds;
		    bgi::rtree< value, bgi::rstar<16> > rtree;
		    for (const auto& faceInfo : masterSurface->faceInfos) {
                const Cell& masterCell = this->mesh.findCell(this->mesh.findCellPosition(faceInfo.cellId));
                const vector<int>& faceIds = masterCell.faceids_from_two_nodes(faceInfo.nodeid1, faceInfo.nodeid2);
                if (faceIds.size() > 0) {
                    node_polygon masterFace;
                    for (const int faceId : faceIds) {
                        int nodePosition = this->mesh.findNodePosition(faceId);
                        const Node& node = this->mesh.findNode(nodePosition);
                        const node_point np{node.x, node.y, node.z};
                        masterFace.outer().push_back(np);
                    }
                    masterFaces.push_back(masterFace);
                    masterFaceNodeIds.push_back(faceIds);
                    for (size_t i = 0 ; i < masterFaces.size() ; ++i) {
                        node_box b = bg::return_envelope<node_box>(masterFaces[i]);
                        rtree.insert(std::make_pair(b, i));
                    }
                }
		    }
		    shared_ptr<const BoundaryElementFace> slaveSurface = dynamic_pointer_cast<const BoundaryElementFace>(this->find(surface->slave));
            for (const auto& faceInfo : slaveSurface->faceInfos) {
                const Cell& slaveCell = this->mesh.findCell(this->mesh.findCellPosition(faceInfo.cellId));
                const vector<int>& faceIds = slaveCell.faceids_from_two_nodes(faceInfo.nodeid1, faceInfo.nodeid2);
                for (const int faceId : faceIds) {
                    int nodePosition = this->mesh.findNodePosition(faceId);
                    const Node& node = this->mesh.findNode(nodePosition);
                    vector<value> result_n;
                    rtree.query(bgi::nearest(node_point{node.x, node.y, node.z}, 1), std::back_inserter(result_n));
                    size_t masterFaceIndex = result_n[0].second;
                    const node_polygon& masterFace = masterFaces[masterFaceIndex];
                    vector<int> contactNodeIds = masterFaceNodeIds[masterFaceIndex];
                    contactNodeIds.push_back(faceId);
                    size_t num_master_points = bg::num_points(masterFace);
                    switch(num_master_points) {
                    case 3: {
                        int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::TETRA4, contactNodeIds , true);
                        group->addCellPosition(cellPosition);
                        break;
                    }
                    case 4: {
                        int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::PYRA5, contactNodeIds , true);
                        group->addCellPosition(cellPosition);
                        break;
                    }
                    default:
                        throw logic_error("Slide element not yet implemented:" + to_string(num_master_points));
                    }

                }
		    }

            toBeRemoved.push_back(constraint);
        }

        for(const auto& constraint: toBeRemoved) {
            remove(constraint->getReference(), idConstraintSet, originalIdConstraintSet, natConstraintSet);
        }
    }
}

void Model::splitElementsByDOFS(){

    vector<shared_ptr<ScalarSpring>> elementSetsToAdd;
    vector<shared_ptr<ElementSet>> elementSetsToRemove;

    for (const auto& elementSet : elementSets.filter(ElementSet::Type::SCALAR_SPRING)) {
        shared_ptr<ScalarSpring> ss = static_pointer_cast<ScalarSpring>(elementSet);
        if (ss->getNbDOFSSpring()>1) {
            int i =1;
            const double stiffness = ss->getStiffness();
            const double damping = ss->getDamping();
            const string name = ss->cellGroup->getName();
            const string comment = ss->cellGroup->getComment();
            if (configuration.logLevel >= LogLevel::DEBUG)
                cout<< *elementSet << " spring must be split."<<endl;
            for (const auto & it : ss->getCellPositionByDOFS()){
                const auto& scalarSpring = make_shared<ScalarSpring>(*this, Identifiable<ElementSet>::NO_ORIGINAL_ID, stiffness, damping);
                shared_ptr<CellGroup> cellGroup = this->mesh.createCellGroup(name+"_"+to_string(i), Group::NO_ORIGINAL_ID, comment);
                scalarSpring->assignCellGroup(cellGroup);
                for (const int cellPosition : it.second){
                    scalarSpring->addSpring(cellPosition, it.first.first, it.first.second);
                    cellGroup->addCellPosition(cellPosition);
                }
                elementSetsToAdd.push_back(scalarSpring);
                i++;
            }
            elementSetsToRemove.push_back(elementSet);
            this->mesh.removeGroup(name);
        }
    }

    for (const auto& elementSet : elementSetsToRemove){
        this->elementSets.erase(Reference<ElementSet>(*elementSet));
    }
    for (const auto& elementSet : elementSetsToAdd){
        this->add(elementSet);
    }
}

void Model::makeBoundarySegments() {
    for (const auto& constraintSet : this->getCommonConstraintSets()) {
        auto constraints = constraintSet->getConstraintsByType(Constraint::Type::SLIDE);
        for (const auto& constraint : constraints) {
            shared_ptr<SlideContact> slide = dynamic_pointer_cast<SlideContact>(constraint);
            shared_ptr<CellGroup> masterCellGroup = mesh.createCellGroup("SLIDE_M_"+to_string(slide->bestId()), Group::NO_ORIGINAL_ID, "created by makeBoundarySegments() for the master in a SLIDE contact");
            shared_ptr<CellGroup> slaveCellGroup = mesh.createCellGroup("SLIDE_S_"+to_string(slide->bestId()), Group::NO_ORIGINAL_ID, "created by makeBoundarySegments() for the slave in a SLIDE contact");
            shared_ptr<BoundaryNodeLine> masterNodeLine = dynamic_pointer_cast<BoundaryNodeLine>(this->find(slide->master));
            const list<int>& masterNodeIds = masterNodeLine->nodeids;
            auto it = masterNodeIds.begin();
            for(unsigned int i = 0; i < masterNodeIds.size() - 1;++i) {
                int nodeId1 = *it;
                int nodeId2 = *(++it);
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, {nodeId1, nodeId2}, true);
                masterCellGroup->addCellPosition(cellPosition);
            }
            slide->masterCellGroup = masterCellGroup;
            shared_ptr<BoundaryNodeLine> slaveNodeLine = dynamic_pointer_cast<BoundaryNodeLine>(this->find(slide->slave));
            const list<int>& slaveNodeIds = slaveNodeLine->nodeids;
            auto it2 = slaveNodeIds.begin();
            for(unsigned int i = 0; i < slaveNodeIds.size() - 1;++i) {
                int nodeId1 = *it2;
                int nodeId2 = *(++it2);
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, {nodeId1, nodeId2}, true);
                slaveCellGroup->addCellPosition(cellPosition);
            }
            slide->slaveCellGroup = slaveCellGroup;
        }
    }
}

void Model::makeBoundarySurfaces() {
    for (const auto& constraintSet : this->getCommonConstraintSets()) {
        auto constraints = constraintSet->getConstraintsByType(Constraint::Type::SURFACE_CONTACT);
        for (const auto& constraint : constraints) {
            shared_ptr<SurfaceContact> surface = dynamic_pointer_cast<SurfaceContact>(constraint);
            shared_ptr<CellGroup> masterCellGroup = mesh.createCellGroup("SURFCONT_M_"+to_string(surface->bestId()), Group::NO_ORIGINAL_ID, "created by makeBoundarySurfaces() for the master in a SURFACE_CONTACT");
            shared_ptr<CellGroup> slaveCellGroup = mesh.createCellGroup("SURFCONT_S_"+to_string(surface->bestId()), Group::NO_ORIGINAL_ID, "created by makeBoundarySurfaces() for the master in a SURFACE_CONTACT");
            shared_ptr<BoundaryNodeSurface> slaveNodeSurface = dynamic_pointer_cast<BoundaryNodeSurface>(this->find(surface->slave));
            if (slaveNodeSurface == nullptr) {
                throw logic_error("Cannot find master node list");
            }
            const list<int>& slaveNodeIds = slaveNodeSurface->nodeids;
            auto it2 = slaveNodeIds.begin();
            for(unsigned int i = 0; i < slaveNodeIds.size();i+=4) {
                int nodeId1 = *(it2++);
                int nodeId2 = *(it2++);
                int nodeId3 = *(it2++);
                int nodeId4 = *(it2++);
                vector<int> connectivity {nodeId1, nodeId2, nodeId3};
                CellType cellType {CellType::TRI3};
                if (nodeId4 != 0) {
                    connectivity.push_back(nodeId4);
                    cellType = CellType::QUAD4;
                }
                int cellPosition = mesh.addCell(Cell::AUTO_ID, cellType, connectivity, true);
                slaveCellGroup->addCellPosition(cellPosition);
            }
            surface->slaveCellGroup = slaveCellGroup;
            shared_ptr<BoundaryNodeSurface> masterNodeSurface = dynamic_pointer_cast<BoundaryNodeSurface>(this->find(surface->master));
            if (masterNodeSurface == nullptr) {
                throw logic_error("Cannot find master node list");
            }
            const list<int>& masterNodeIds = masterNodeSurface->nodeids;
            auto it = masterNodeIds.begin();
            for(unsigned int i = 0; i < masterNodeIds.size();i+=4) {
                int nodeId1 = *(it++);
                int nodeId2 = *(it++);
                int nodeId3 = *(it++);
                int nodeId4 = *(it++);
                vector<int> connectivity {nodeId1, nodeId2, nodeId3};
                CellType cellType {CellType::TRI3};
                if (nodeId4 != 0) {
                    connectivity.push_back(nodeId4);
                    cellType = CellType::QUAD4;
                }
                int cellPosition = mesh.addCell(Cell::AUTO_ID, cellType, connectivity, true);
                masterCellGroup->addCellPosition(cellPosition);
            }
            surface->masterCellGroup = masterCellGroup;
        }
    }
}

void Model::addAutoAnalysis() {
    bool linearStatic = true;
    // LD very basic implementation of analysis detection. Should also look for non linear materials etc ?
    for(const auto& nonLinearStrategy : objectives.filter(Objective::Type::NONLINEAR_STRATEGY)) {
        const auto& analysis = make_shared<NonLinearMecaStat>(*this, nonLinearStrategy->getOriginalId());
        this->add(analysis);
        linearStatic = false;
    }
    for(const auto& modalStrategy : objectives.filter(Objective::Type::FREQUENCY_SEARCH)) {
        const auto& analysis = make_shared<LinearModal>(*this, modalStrategy->getOriginalId());
        this->add(analysis);
        linearStatic = false;
    }
    if (linearStatic) {
        const auto& analysis = make_shared<LinearMecaStat>(*this);
        this->add(analysis);
    }
}

void Model::createGraph() {
    struct VertexProps { std::string name; };
    struct EdgeProps   { std::string name; };
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexProps, EdgeProps> Graph;

    Graph g;

    for (auto& elementSet : elementSets) {
        auto vpos = boost::add_vertex(g);
        auto v = g[vpos];
        v.name = to_str(*elementSet);
    }

    for (auto& constraintSet : constraintSets) {
        long unsigned vpos = boost::add_vertex(g);
        auto v = g[vpos];
        v.name = to_str(*constraintSet);
    }

    // Add edges
//    g.add_edge(v0, v1);
//    g.add_edge(v1, v2);
    boost::write_graphviz(cout, g, boost::make_label_writer(get(&VertexProps::name, g)),
            boost::make_label_writer(get(&EdgeProps::name, g)));
    std::cerr << "Graph node name:" << g[0].name << "\n";
}

void Model::finish() {
    if (finished) {
        return;
    }

    /* Build the coordinate systems from their definition points */
    for (const auto& coordinateSystemEntry : mesh.coordinateSystemStorage.coordinateSystemByRef) {
        coordinateSystemEntry.second->build();
    }

    for (shared_ptr<ElementSet> elementSet : elementSets) {
        for (int nodePosition : elementSet->nodePositions()) {
            mesh.allowDOFS(nodePosition,elementSet->getDOFSForNode(nodePosition));
        }
    }

    if (this->configuration.autoDetectAnalysis and analyses.size() == 0) {
        addAutoAnalysis();
    }
//    if (this->configuration.autoDetectAnalysis and analyses.size() == 0) {
//        addAutoAnalysis();
//    } else {
//        int i = 1;
//        for (shared_ptr<Analysis> analysis : analyses) {
//            if (analysis->isLinear() and analysis->isStatic() and constraintSets.contains(ConstraintSet::Type::CONTACT)) {
//                cout << "Transforming linear static analysis " + to_str(*analysis) + " in non-linear because of contact.";
//                NonLinearStrategy nonLinearStrategy(*this, 1, i++);
//                this->add(nonLinearStrategy);
//                NonLinearMecaStat nonLinAnalysis(*this, nonLinearStrategy.getOriginalId());
//                analysis->copyInto(nonLinAnalysis);
//                this->add(nonLinAnalysis);
//
//                this->analyses.erase(analysis->getReference());
//            }
//        }
//    }

    if (this->configuration.createSkin) {
        generateSkin();
    }

    for (shared_ptr<Analysis> analysis : analyses) {
        for (const auto& boundaryCondition : analysis->getBoundaryConditions()) {
            for(int nodePosition: boundaryCondition->nodePositions()) {
                analysis->addBoundaryDOFS(nodePosition,
                        boundaryCondition->getDOFSForNode(nodePosition));
            }
        }
    }

    removeAssertionsMissingDOFS();

    if (this->configuration.makeBoundaryCells) {
        makeBoundarySegments();
        makeBoundarySurfaces();
    }

    if (this->configuration.emulateLocalDisplacement) {
        emulateLocalDisplacementConstraint();
    }

    if (this->configuration.displayHomogeneousConstraint) {
        generateBeamsToDisplayHomogeneousConstraint();
    }

    if (this->configuration.emulateAdditionalMass) {
        emulateAdditionalMass();
    }

    if (this->configuration.replaceCombinedLoadSets) {
        replaceCombinedLoadSets();
    }

    if (this->configuration.replaceDirectMatrices) {
        replaceDirectMatrices();
    }

    if (this->configuration.replaceRigidSegments) {
        replaceRigidSegments();
    }

    if (this->configuration.removeRedundantSpcs) {
        removeRedundantSpcs();
    }

    if (this->configuration.removeConstrainedImposed) {
        removeConstrainedImposed();
    }

    if (this->configuration.removeIneffectives) {
        removeIneffectives();
    }

    if (this->configuration.virtualDiscrets) {
        generateDiscrets();
    }

    if (this->configuration.splitDirectMatrices){
        splitDirectMatrices(this->configuration.sizeDirectMatrices);
    }

    if (this->configuration.makeCellsFromDirectMatrices){
        makeCellsFromDirectMatrices();
    }

    if (this->configuration.makeCellsFromLMPC){
        makeCellsFromLMPC();
    }

    if (this->configuration.makeCellsFromRBE){
        makeCellsFromRBE();
    }

    if (this->configuration.makeCellsFromSurfaceSlide){
        makeCellsFromSurfaceSlide();
    }

    if (this->configuration.splitElementsByDOFS){
        splitElementsByDOFS();
    }

    if (this->configuration.addVirtualMaterial) {
        assignVirtualMaterial();
    }

    assignElementsToCells();
    generateMaterialAssignments();

    if (this->configuration.removeIneffectives) {
        removeUnassignedMaterials();
    }

    addDefaultAnalysis();

    this->mesh.finish();
    finished = true;
}

bool Model::validate() {
    bool meshValid = mesh.validate();

    // Sizes are stocked now, because validation remove invalid objects.
    int sizeMat = materials.size();     string sMat = ( (sizeMat > 1) ? "s are " : " is ");
    int sizeEle = elementSets.size();   string sEle = ( (sizeEle > 1) ? "s are " : " is ");
    int sizeLoa = loadings.size();      string sLoa = ( (sizeLoa > 1) ? "s are " : " is ");
    int sizeLos = loadSets.size();      string sLos = ( (sizeLos > 1) ? "s are " : " is ");
    int sizeCon = constraints.size();   string sCon = ( (sizeCon > 1) ? "s are " : " is ");
    int sizeCos = constraintSets.size();string sCos = ( (sizeCos > 1) ? "s are " : " is ");
    int sizeAna = analyses.size();      string sAna = ( (sizeAna > 1) ? "es are " : "is is ");

    bool validMat = materials.validate();
    bool validEle = elementSets.validate();
    bool validLoa = loadings.validate();
    bool validLos = loadSets.validate();
    bool validCon = constraints.validate();
    bool validCos = constraintSets.validate();
    bool validAna = analyses.validate();

    if (configuration.logLevel >= LogLevel::DEBUG) {
       cout << "The " << sizeMat << " material"     << sMat << (validMat ? "" : "NOT ") << "valid." << endl;
       cout << "The " << sizeEle << " elementSet"   << sEle << (validEle ? "" : "NOT ") << "valid." << endl;
       cout << "The " << sizeLoa << " loading"      << sLoa << (validLoa ? "" : "NOT ") << "valid." << endl;
       cout << "The " << sizeLos << " loadSet"      << sLos << (validLos ? "" : "NOT ") << "valid." << endl;
       cout << "The " << sizeCon << " constraint"   << sCon << (validCon ? "" : "NOT ") << "valid." << endl;
       cout << "The " << sizeCos << " constraintSet"<< sCos << (validCos ? "" : "NOT ") << "valid." << endl;
       cout << "The " << sizeAna << " analys"      << sAna << (validAna ? "" : "NOT ") << "valid." << endl;
    }
    bool allValid = meshValid && validMat && validEle && validLoa && validLos && validCon
            && validCos && validAna;
    this->afterValidation = true;
    return allValid;
}

void Model::assignVirtualMaterial() {
    for (shared_ptr<ElementSet> element : elementSets.filter(ElementSet::Type::STRUCTURAL_SEGMENT)) {
        element->assignMaterial(getVirtualMaterial());
    }
    for (shared_ptr<ElementSet> element : elementSets.filter(ElementSet::Type::NODAL_MASS)) {
        element->assignMaterial(getVirtualMaterial());
    }
}

void Model::assignElementsToCells() {
    for (shared_ptr<ElementSet> element : elementSets) {
        if (element->cellGroup != nullptr) {
            CellContainer container(mesh);
            container.add(*element->cellGroup);
            mesh.assignElementId(container, element->getId());
        }
    }
}

} /* namespace vega */
