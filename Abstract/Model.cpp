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
#include <ciso646>

using namespace std;

namespace vega {

constexpr int LoadSet::COMMON_SET_ID; // required for C++11
constexpr int ConstraintSet::COMMON_SET_ID; // required for C++11
constexpr int ObjectiveSet::COMMON_SET_ID; // required for C++11

Model::Model(string name, string inputSolverVersion, SolverName inputSolver,
        const ModelConfiguration configuration, const vega::ConfigurationParameters::TranslationMode translationMode) :
        name(name), inputSolverVersion(inputSolverVersion), //
        inputSolver(inputSolver), //
        modelType(ModelType::TRIDIMENSIONAL_SI), //
        mesh{configuration.logLevel, name},
        configuration(configuration), translationMode(translationMode), //
        commonLoadSet(make_shared<LoadSet>(*this, LoadSet::Type::ALL, LoadSet::COMMON_SET_ID)), //
		commonConstraintSet(make_shared<ConstraintSet>(*this, ConstraintSet::Type::ALL, ConstraintSet::COMMON_SET_ID)), //
        commonObjectiveSet(make_shared<ObjectiveSet>(*this, ObjectiveSet::Type::ALL, ObjectiveSet::COMMON_SET_ID)) { //
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
    if (configuration.logLevel >= LogLevel::TRACE) {
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

void Model::add(const shared_ptr<Material>& material) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << *material << endl;
    }
    materials.add(material);
}

void Model::add(const shared_ptr<Constraint>& constraint) {
    if (configuration.logLevel >= LogLevel::TRACE) {
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

void Model::add(const shared_ptr<ObjectiveSet>& objectiveSet) {
    if (configuration.logLevel >= LogLevel::TRACE) {
        cout << "Adding " << *objectiveSet << endl;
    }
    objectiveSets.add(objectiveSet);
}

void Model::add(const shared_ptr<NamedValue>& value) {
    if (configuration.logLevel >= LogLevel::TRACE) {
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

vector<int> Model::getMaterialsId() const{
	vector<int> v;
	for (const auto& material : materials){
		v.push_back(material->getId());
	}
	return v;
}

vector<int> Model::getElementSetsId() const{
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

void Model::remove(const Reference<Constraint> refC, const Reference<ConstraintSet> refCSet) {

    const auto & cR = constraintReferences_by_constraintSet_ids[refCSet.id];
    for (const auto& it2 : cR) {
        if (it2 == refC){
            constraintReferences_by_constraintSet_ids[refCSet.id].erase(it2);
            break; // iterator is invalid now
        }
    }
    if (refCSet.has_original_id()){
        const auto & cR2 = constraintReferences_by_constraintSet_original_ids_by_constraintSet_type[refCSet.type][refCSet.original_id];
        for (const auto& it3 : cR2) {
            if (it3 == refC){
                constraintReferences_by_constraintSet_original_ids_by_constraintSet_type[refCSet.type][refCSet.original_id].erase(it3);
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
    for (const auto& analysis : analyses) {
        if (analysis->contains(loadSetReference)) {
            if (configuration.logLevel >= LogLevel::TRACE) {
                const auto& loadSet = loadSets.find(loadSetReference);
                cout << "Disassociating empty " << *loadSet << " from " << *analysis << endl;
            }
            analysis->remove(loadSetReference);
        }
    }
    loadSets.erase(loadSetReference);
}

template<>
void Model::remove(const Reference<ConstraintSet> constraintSetReference) {
    for (const auto& analysis : analyses) {
        if (analysis->contains(constraintSetReference)) {
            if (configuration.logLevel >= LogLevel::TRACE) {
                const auto& constraintSet = constraintSets.find(constraintSetReference);
                cout << "Disassociating empty " << *constraintSet << " from " << *analysis << endl;
            }
            analysis->remove(constraintSetReference);
        }
    }
    constraintSets.erase(constraintSetReference);
}

template<>
void Model::remove(const Reference<ObjectiveSet> objectiveSetReference) {
    for (const auto& analysis : analyses) {
        if (analysis->contains(objectiveSetReference)) {
            if (configuration.logLevel >= LogLevel::TRACE) {
                const auto& objectiveSet = objectiveSets.find(objectiveSetReference);
                cout << "Disassociating empty " << *objectiveSet << " from " << *analysis << endl;
            }
            analysis->remove(objectiveSetReference);
        }
    }
    objectiveSets.erase(objectiveSetReference);
}

template<>
void Model::remove(const Reference<Objective> objectiveReference) {
    for (const auto& it : objectiveReferences_by_objectiveSet_ids) {
        for (const auto& it2 : it.second) {
            if (it2 == objectiveReference) {
                objectiveReferences_by_objectiveSet_ids[it.first].erase(it2);
                break; // iterator is invalid now
            }
        }
    }
    for (const auto& it : objectiveReferences_by_objectiveSet_original_ids_by_objectiveSet_type) {
        for (const auto& it2 : it.second) {
            for (const auto& it3 : it2.second) {
                if (it3 == objectiveReference) {
                    objectiveReferences_by_objectiveSet_original_ids_by_objectiveSet_type[it.first][it2.first].erase(
                            it3);
                            break; // iterator is invalid now
                }
            }
        }
    }
    objectives.erase(objectiveReference);
}

template<>
shared_ptr<Objective> Model::find(const Reference<Objective> reference) const {
    return objectives.find(reference);
}

template<>
shared_ptr<ObjectiveSet> Model::find(const Reference<ObjectiveSet> reference) const {
    return objectiveSets.find(reference);
}

template<>
shared_ptr<NamedValue> Model::find(const Reference<NamedValue> reference) const {
    return values.find(reference);
}

template<>
shared_ptr<Loading> Model::find(const Reference<Loading> reference) const {
    return loadings.find(reference);
}

template<>
shared_ptr<LoadSet> Model::find(const Reference<LoadSet> reference) const {
    return loadSets.find(reference);
}

template<>
shared_ptr<Constraint> Model::find(const Reference<Constraint> reference) const {
    return constraints.find(reference);
}

template<>
shared_ptr<ConstraintSet> Model::find(const Reference<ConstraintSet> reference) const {
    return constraintSets.find(reference);
}

template<>
shared_ptr<Analysis> Model::find(const Reference<Analysis> reference) const {
    shared_ptr<Analysis> analysis;
    if (reference.type == Analysis::Type::UNKNOWN and reference.id == Reference<Analysis>::NO_ID)
        // retrieve by original_id
        analysis = analyses.find(reference.original_id);
    else
        analysis = analyses.find(reference);
    return analysis;
}

template<>
shared_ptr<ElementSet> Model::find(const Reference<ElementSet> reference) const {
    shared_ptr<ElementSet> elementSet;
    if (reference.type == ElementSet::Type::UNKNOWN and reference.id == Reference<ElementSet>::NO_ID)
        // retrieve by original_id
        elementSet = elementSets.find(reference.original_id);
    else
        elementSet = elementSets.find(reference);
    return elementSet;
}

template<>
shared_ptr<Target> Model::find(const Reference<Target> reference) const {
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

void Model::addObjectiveIntoObjectiveSet(const Reference<Objective>& objectiveReference,
        const Reference<ObjectiveSet>& objectiveSetReference) {
    if (objectiveReference.has_id())
        objectiveReferences_by_objectiveSet_ids[objectiveSetReference.id].insert(objectiveReference);
    if (objectiveReference.has_original_id())
        objectiveReferences_by_objectiveSet_original_ids_by_objectiveSet_type[objectiveSetReference.type][objectiveSetReference.original_id].insert(
                objectiveReference);
    if (objectiveSetReference == commonObjectiveSet->getReference() && find(commonObjectiveSet->getReference()) == nullptr)
        add(commonObjectiveSet); // commonObjectiveSet is added to the model if needed
    if (this->find(objectiveSetReference) == nullptr) {
        const auto& objectiveSet = make_shared<ObjectiveSet>(*this, objectiveSetReference.type, objectiveSetReference.original_id);
        this->add(objectiveSet);
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

shared_ptr<ObjectiveSet> Model::getOrCreateObjectiveSet(int objectiveset_id, ObjectiveSet::Type objectiveset_type){

    Reference<ObjectiveSet> objectiveSetReference(objectiveset_type, objectiveset_id);
    shared_ptr<ObjectiveSet> objectiveSetPtr = this->find(objectiveSetReference);
    if (objectiveSetPtr == nullptr){
        ObjectiveSet objectiveSet(*this, objectiveset_type, objectiveset_id);
        objectiveSetPtr = objectiveSet.clone();
        this->objectiveSets.add(objectiveSetPtr);
    }
    return objectiveSetPtr;
}

set<shared_ptr<Loading>, ptrLess<Loading>> Model::getLoadingsByLoadSet(
        const Reference<LoadSet>& loadSetReference) const {
    set<shared_ptr<Loading>, ptrLess<Loading>> result;
    auto itm = loadingReferences_by_loadSet_ids.find(loadSetReference.id);
    if (itm != loadingReferences_by_loadSet_ids.end()) {
        for (const auto& itm2 : itm->second) {
            const auto& loading = find(itm2);
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

set<shared_ptr<Constraint>, ptrLess<Constraint>> Model::getConstraintsByConstraintSet(
        const Reference<ConstraintSet>& constraintSetReference) const {
    set<shared_ptr<Constraint>, ptrLess<Constraint>> result;
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

set<shared_ptr<ConstraintSet>, ptrLess<ConstraintSet>> Model::getConstraintSetsByConstraint(
        const Reference<Constraint>& constraintReference) const {
    set<shared_ptr<ConstraintSet>, ptrLess<ConstraintSet>> result;
    for (const auto& it : constraintSets) {
        const auto& constraints = getConstraintsByConstraintSet(it->getReference());
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

set<shared_ptr<Objective>, ptrLess<Objective>> Model::getObjectivesByObjectiveSet(
        const Reference<ObjectiveSet>& objectiveSetReference) const {
    set<shared_ptr<Objective>, ptrLess<Objective>> result;
    auto itm = objectiveReferences_by_objectiveSet_ids.find(objectiveSetReference.id);
    if (itm != objectiveReferences_by_objectiveSet_ids.end()) {
        for (const auto& itm2 : itm->second) {
            result.insert(find(itm2));
        }
    }
    auto itm2 = objectiveReferences_by_objectiveSet_original_ids_by_objectiveSet_type.find(
            objectiveSetReference.type);
    if (itm2 != objectiveReferences_by_objectiveSet_original_ids_by_objectiveSet_type.end()) {
        auto itm3 = itm2->second.find(objectiveSetReference.original_id);
        if (itm3 != itm2->second.end()) {
            for (const auto& itm4 : itm3->second) {
                result.insert(find(itm4));
            }
        }
    }
    return result;
}

vector<shared_ptr<ConstraintSet>> Model::getActiveConstraintSets() const {
    vector<shared_ptr<ConstraintSet>> result;
    set<shared_ptr<ConstraintSet>, ptrLess<ConstraintSet>> cset;
    for (const auto& analyse : analyses) {
        for (const auto& constraintSet : analyse->getConstraintSets()) {
            if (cset.find(constraintSet) == cset.end()) {
                cset.insert(constraintSet);
                result.push_back(constraintSet);
            }
        }
    }
    return result;
}

vector<shared_ptr<LoadSet>> Model::getActiveLoadSets() const {
    vector<shared_ptr<LoadSet>> result;
    set<shared_ptr<LoadSet>, ptrLess<LoadSet>> lset;
    for (const auto& analyse : analyses) {
        for (const auto& loadSet : analyse->getLoadSets()) {
            if (lset.find(loadSet) == lset.end()) {
                lset.insert(loadSet);
                result.push_back(loadSet);
            }
        }
    }
    return result;
}

vector<shared_ptr<ConstraintSet>> Model::getCommonConstraintSets() const {
    vector<shared_ptr<ConstraintSet>> result;
    map<shared_ptr<ConstraintSet>, size_t> map;
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

vector<shared_ptr<LoadSet>> Model::getCommonLoadSets() const {
    vector<shared_ptr<LoadSet>> result;
    map<shared_ptr<LoadSet>, size_t> map;
    for (const auto& analyse : analyses) {
        for (const auto& loadSet : analyse->getLoadSets()) {
            map[loadSet] += 1;
        }
    }
    for (const auto& loadSet : loadSets) {
        auto it = map.find(loadSet);
        if (it == map.end())
            continue;
        if (it->second != analyses.size())
            continue; // Check if load is in all analyses ?
        if (loadSet->type != LoadSet::Type::DLOAD) // DLOAD is never common
            continue;
        result.push_back(loadSet);
    }
    return result;
}

set<shared_ptr<ConstraintSet>, ptrLess<ConstraintSet>> Model::getUncommonConstraintSets() const {
    set<shared_ptr<ConstraintSet>, ptrLess<ConstraintSet>> result;
    map<shared_ptr<ConstraintSet>, size_t, ptrLess<ConstraintSet>> amap;
    for (const auto& analyse : analyses) {
        for (const auto& constraintSet : analyse->getConstraintSets()) {
            amap[constraintSet] += 1;
        }
    }
    for (const auto& constraintSet : constraintSets) {
        auto it = amap.find(constraintSet);
        if (it != amap.end() and it->second < analyses.size())
            result.insert(constraintSet);
    }
    return result;
}

set<shared_ptr<LoadSet>, ptrLess<LoadSet>> Model::getUncommonLoadSets() const {
    set<shared_ptr<LoadSet>, ptrLess<LoadSet>> result;
    map<shared_ptr<LoadSet>, size_t, ptrLess<LoadSet>> lmap;
    for (const auto& analyse : analyses) {
        for (const auto& loadSet : analyse->getLoadSets()) {
            lmap[loadSet] += 1;
        }
    }
    for (const auto& loadSet : loadSets) {
        auto it = lmap.find(loadSet);
        if ((it != lmap.end() and it->second < analyses.size()) and loadSet->type != LoadSet::Type::DLOAD)
            result.insert(loadSet);
    }
    return result;
}

void Model::generateDiscrets() {

    //rigid constraints
    shared_ptr<CellGroup> virtualDiscretTRGroup = nullptr;

    shared_ptr<CellGroup> virtualDiscretTGroup = nullptr;

    // LD TODO : possible optimisation, only "altered nodes" (nodes with rotations, nodes with boundary conditions)
    for (const Node& node : this->mesh.nodes) {
        DOFS missingDOFS;

        for (const auto& analysis : analyses) {

            const DOFS& requiredDOFS = analysis->findBoundaryDOFS(node.position);
            if (!node.dofs.containsAll(requiredDOFS)) {
                missingDOFS += requiredDOFS - node.dofs;
            }
        }

        DOFS addedDOFS;
        if (not missingDOFS.empty()) {
            if (missingDOFS.containsAnyOf(DOFS::ROTATIONS)) {
                //extra dofs added by the DISCRET. They need to be blocked.
                addedDOFS = DOFS::ALL_DOFS - node.dofs - missingDOFS;
                if (virtualDiscretTRGroup == nullptr) {
                    const auto& virtualDiscretTR = make_shared<DiscretePoint>(*this, MatrixType::DIAGONAL);
                    for (DOF dof: DOFS::ALL_DOFS) {
                        virtualDiscretTR->addStiffness(dof, dof, 0.0);
                    }
                    ostringstream oss;
                    oss << "created by generateDiscrets() because missing DOFs: " << missingDOFS << " and node: " << node;
                    virtualDiscretTRGroup = mesh.createCellGroup("VDiscrTR", Group::NO_ORIGINAL_ID, oss.str());
                    virtualDiscretTR->add(*virtualDiscretTRGroup);
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
                    const auto& virtualDiscretT = make_shared<DiscretePoint>(*this, MatrixType::DIAGONAL);
                    for (DOF dof: DOFS::TRANSLATIONS) {
                        virtualDiscretT->addStiffness(dof, dof, 0.0);
                    }
                    ostringstream oss;
                    oss << "created by generateDiscrets() because missing DOFs: " << missingDOFS << " and node: " << node;
                    virtualDiscretTGroup = mesh.createCellGroup("VDiscrT", Group::NO_ORIGINAL_ID, oss.str());
                    virtualDiscretT->add(*virtualDiscretTGroup);
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
                    spc->addNodePosition(node.position);
                    add(spc);
                    addConstraintIntoConstraintSet(*spc, *spcSet);
                    analysis->add(*spcSet);
                    if (configuration.logLevel >= LogLevel::DEBUG) {
                        cout << "Adding virtual spc on node: id: " << node.id << " for " << extraDOFS
                                << endl;
                    }
                }
            }
        }
    }
}

shared_ptr<Material> Model::getOrCreateMaterial(int material_id, bool createIfNotExists) {
    shared_ptr<Material> result = materials.find(material_id);
    if (result == nullptr and createIfNotExists) {
        result = make_shared<Material>(*this, material_id);
        this->add(result);
    }
    return result;
}

shared_ptr<CellContainer> Model::getMaterialAssignment(const Reference<Material>& materialRef) const {
    const auto& it = material_assignment_by_materialRef.find(materialRef);
    if (it != material_assignment_by_materialRef.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

bool Model::hasMaterialAssignment(const Reference<Material>& materialRef) const {

    const auto& it = material_assignment_by_materialRef.find(materialRef);
    if (it != material_assignment_by_materialRef.end()) {
        return not it->second->empty();
    } else {
        return false;
    }
}

void Model::assignMaterial(const Reference<Material>& materialRef, const CellContainer& materialAssign) {
    if (configuration.logLevel >= LogLevel::TRACE)
        cout << "Previous material assignments:" << material_assignment_by_materialRef << endl;
    const auto& it = material_assignment_by_materialRef.find(materialRef);
    if (it != material_assignment_by_materialRef.end()) {
        if (configuration.logLevel >= LogLevel::TRACE)
            cout << "Adding another assignment:" << materialAssign.to_str() << " to material:" << materialRef << endl;
        it->second->add(materialAssign);
    } else {
        if (configuration.logLevel >= LogLevel::TRACE)
            cout << "Creating first assignment:" << materialAssign.to_str() << " to material:" << materialRef << endl;
        material_assignment_by_materialRef[materialRef] = make_shared<CellContainer>(materialAssign);
    }
    if (configuration.logLevel >= LogLevel::TRACE)
        cout << "Current material assignments:" << material_assignment_by_materialRef << endl;
}

shared_ptr<Material> Model::getVirtualMaterial() {
    if (not virtualMaterial) {
        virtualMaterial = this->getOrCreateMaterial(Material::NO_ORIGINAL_ID);
        virtualMaterial->addNature(make_shared<ElasticNature>(*this, 1e-12, 0.0));
    }
    return virtualMaterial;
}

vector<shared_ptr<Beam>> Model::getBeams() const {
    vector<shared_ptr<Beam>> result;
    for (const auto& elementSet : elementSets) {
        if (elementSet->isBeam()) {
            const auto& beam = static_pointer_cast<Beam>(elementSet);
            result.push_back(beam);
        }
    }
    return result;
}

vector<shared_ptr<Beam>> Model::getTrusses() const {
    vector<shared_ptr<Beam>> result;
    for (const auto& elementSet : elementSets) {
        if (elementSet->isTruss()) {
            const auto& bar = static_pointer_cast<Beam>(elementSet);
            result.push_back(bar);
        }
    }
    return result;
}

void Model::setParameter(const ModelParameter& parameter, const string& value) noexcept {
    parameters[parameter] = value;
}

bool Model::contains(const ModelParameter& parameter) const noexcept {
    return this->parameters.find(parameter) != this->parameters.end();
}

string Model::getParameter(const ModelParameter& parameter) const noexcept {
    const auto& modelParameter = this->parameters.find(parameter);
    if (modelParameter != this->parameters.end())
        return modelParameter->second;
    return "";
}

bool Model::needsLargeDisplacements() const {
    int largeDisp = 0;
    auto it = this->parameters.find(ModelParameter::LARGE_DISPLACEMENTS);
    if (it != this->parameters.end()) {
        largeDisp = stoi(it->second);
    }
    return largeDisp != 0;
}

std::shared_ptr<Analysis> Model::reusableAnalysisFor(const shared_ptr<Analysis>& analysis) const noexcept {
    for (const auto& previousAnalysis : this->analyses) {
        if (*previousAnalysis == *analysis)
            break; // only looking at previous analysis
        if (analysis->canReuse(previousAnalysis)) {
            return previousAnalysis;
        }
    }
    return nullptr;
}

bool Model::canBeReused(const shared_ptr<Analysis>& analysis) const noexcept {
    bool isAfter = false;
    for (const auto& otherAnalysis : this->analyses) {
        if (*otherAnalysis == *analysis) {
            isAfter = true;
            continue;
        }
        if (not isAfter) {
            continue; // only looking at following analysis
        }
//        if (otherAnalysis->canReuse(analysis)) {
//            return true;
//        }
        const auto& reusableAnalysis = reusableAnalysisFor(otherAnalysis);
        if (reusableAnalysis == nullptr) {
            continue;
        }
        if (*reusableAnalysisFor(otherAnalysis) == *analysis) {
            return true;
        }
    }
    return false;
}

void Model::generateSkin() {

    for (const auto& loading : loadings) {
        if (not loading->isCellLoading()) {
            continue;
        }
        const auto& cellLoading = static_pointer_cast<CellLoading>(loading);
        cellLoading->createSkin();
    }

    for (const auto& target : targets) {
        if (not target->isCellTarget()) {
            continue;
        }
        auto cellTarget = static_pointer_cast<CellTarget>(target);
        cellTarget->createSkin();
        cellTarget->markAsWritten();
    }

}

void Model::emulateLocalDisplacementConstraint() {

    for (const auto& constraint : constraints.filter(Constraint::Type::SPC)) {
        const auto& spc = static_pointer_cast<SinglePointConstraint>(   constraint);
        spc->emulateLocalDisplacementConstraint();
        if (spc->empty()) {
            this->remove(spc->getReference());
        }
        spc->markAsWritten();
    }

}

void Model::emulateAdditionalMass() {
    vector<shared_ptr<ElementSet>> newElementSets;
    for (const auto& elementSet : elementSets) {
        double rho = elementSet->getAdditionalRho();
        if (!is_zero(rho)) {
            // copy elementSet
            const shared_ptr<ElementSet>& newElementSet = elementSet->clone();
            newElementSet->resetId();
            const auto& cellElementSet = dynamic_pointer_cast<CellElementSet>(newElementSet);
            if (cellElementSet == nullptr)
                throw logic_error("additional mass but elementset is not a cellcontainer?");

            // assign new material
            const auto& newMaterial = make_shared<Material>(*this);
            newMaterial->addNature(make_shared<ElasticNature>(*this, 0, 0, 0, rho));
            materials.add(newMaterial);
            cellElementSet->assignMaterial(newMaterial);
            // copy and assign new cellGroup
            ostringstream oss;
            oss << "created by emulateAdditionalMass() because of elementSet: " << elementSet << " additional rho:" << rho;
            const auto& newCellGroup = mesh.createCellGroup(
                    "VAM_" + to_string(newElementSets.size()), Group::NO_ORIGINAL_ID, oss.str());
            const auto& cellContainer = dynamic_pointer_cast<CellContainer>(elementSet);
            cellContainer->add(*newCellGroup);
            newElementSets.push_back(newElementSet);
            for (const int cellPosition : elementSet->cellPositions()) {
                const Cell& cell = mesh.findCell(cellPosition);
                int newCellPosition = mesh.addCell(Cell::AUTO_ID, cell.type, cell.nodeIds, cell.isvirtual,
                        cell.cspos, cell.elementId);
                newCellGroup->addCellPosition(newCellPosition);
            }
        }
    }
    for (const auto& elementSet : newElementSets)
        elementSets.add(elementSet);
}

void Model::generateBeamsToDisplayMasterSlaveConstraint() {

    shared_ptr<CellGroup> virtualGroupRigid = nullptr;
    shared_ptr<CellGroup> virtualGroupRBE3 = nullptr;

    const auto& activeConstraintSets = getActiveConstraintSets();
    for (const auto& constraintSet : activeConstraintSets) {
        const auto& constraints = constraintSet->getConstraints();
        for (const auto& constraint : constraints) {
            switch (constraint->type) {
            case Constraint::Type::RIGID: {
                if (virtualGroupRigid == nullptr) {
                    const auto& virtualBeam = make_shared<CircularSectionBeam>(*this, 0.001, Beam::BeamModel::EULER, 0.0);
                    if (this->configuration.addVirtualMaterial) {
                        virtualBeam->assignMaterial(getVirtualMaterial());
                    }
                    ostringstream oss;
                    oss << "created by generateBeamsToDisplayMasterSlaveConstraint() because of rigid constraint: " << constraint;
                    virtualGroupRigid = mesh.createCellGroup("VRigid", Group::NO_ORIGINAL_ID, oss.str());
                    virtualBeam->add(*virtualGroupRigid);
                    this->add(virtualBeam);
                }
                const auto& rigid = static_pointer_cast<RigidConstraint>(constraint);
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
                if (virtualGroupRBE3 == nullptr) {
                    const auto& virtualBeam = make_shared<CircularSectionBeam>(*this, 0.001, Beam::BeamModel::EULER, 0.0);
                    if (configuration.addVirtualMaterial) {
                        virtualBeam->assignMaterial(getVirtualMaterial());
                    }
                    ostringstream oss;
                    oss << "created by generateBeamsToDisplayMasterSlaveConstraint() because of rbe3 constraint: " << constraint;
                    virtualGroupRBE3 = mesh.createCellGroup("VRBE3", Group::NO_ORIGINAL_ID, oss.str());
                    virtualBeam->add(*virtualGroupRBE3);
                    this->add(virtualBeam);
                }
                const auto& rbe3 = static_pointer_cast<RBE3>(constraint);
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
        if (not this->material_assignment_by_materialRef.empty()) {
            cerr << "generateMaterialAssignments with PartitionModel is not "
                    << " yet implemented. " << endl
                    << "This method should partition the elementSets"
                    << " and assign materials to elementSets. Useful if output=Nastran" << endl;
        }
    } else {
        //generate or update material assignments from elementSets
        for (const auto& element : elementSets) {
            const auto& cellElementSet = dynamic_pointer_cast<CellElementSet>(element);
            if (cellElementSet == nullptr) {
                throw logic_error("Assigning an ElementSet which is not a CellContainer to a Material not yet implemeted");
            }
            if (cellElementSet->empty()) {
                throw logic_error("Assigning an ElementSet to an empty CellContainer?");
            }
            for (const auto& material : element->getMaterials()) {
                if (element->effective()) {
                    if (configuration.logLevel >= LogLevel::TRACE)
                        cout << "Generating assignment for material:" << *material << " to elementSet:" << *cellElementSet << endl;
                    assignMaterial(material->getReference(), *cellElementSet);
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
        if (configuration.logLevel >= LogLevel::TRACE)
            cout << "Removed ineffective " << *loading << endl;
        remove(Reference<Loading>(*loading));
    }

    // remove empty loadSets from the model
    vector<Reference<LoadSet>> loadSetSetsToRemove;
    for (const auto& loadSet : this->loadSets) {
        if (loadSet->empty()) {
            loadSetSetsToRemove.push_back(loadSet->getReference());
        }
    }
    for (Reference<LoadSet> loadSetRef : loadSetSetsToRemove) {
        if (configuration.logLevel >= LogLevel::TRACE)
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
        if (configuration.logLevel >= LogLevel::TRACE)
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
        if (configuration.logLevel >= LogLevel::TRACE)
            cout << "Removed empty " << constraintSetRef << endl;
        remove(constraintSetRef);
    }

    // remove empty elementSets from the model
    vector<shared_ptr<ElementSet>> elementSetsToRemove;
    for (const auto& elementSet : elementSets) {
        if (not elementSet->effective())
            elementSetsToRemove.push_back(elementSet);
    }
    for (const auto& elementSet : elementSetsToRemove) {
        if (configuration.logLevel >= LogLevel::TRACE)
            cout << "Removed empty " << *elementSet << endl;
        this->elementSets.erase(elementSet->getReference());
    }

    for (const int& unusedNodePosition : mesh.nodes.reservedButUnusedNodePositions) {
        for (auto& nodeGroup : mesh.getNodeGroups()) {
            if (nodeGroup->containsNodePosition(unusedNodePosition)) {
                nodeGroup->removeNodeByPosition(unusedNodePosition);
            }
        }
    }

}

void Model::removeUnassignedMaterials() {
    //vector<shared_ptr<Material>> materialsToRemove;
    //for (const auto& material : materials) {
        /*
         * if the model is configured to assign materials to cells directly check
         * that for every material at lest a cell or cellgroup is assigned
         */
        //if (!configuration.partitionModel and material->getAssignment().empty())
        //        materialsToRemove.push_back(material);
    //}
    //for (const auto& material : materialsToRemove) {
    //    if (configuration.logLevel >= LogLevel::DEBUG)
    //        cout << "Removed unassigned " << *material << endl;
    //    this->materials.erase(material->getReference());
    //}
}

void Model::replaceCombinedLoadSets() {
    for (const auto& loadSet : this->loadSets) {
        for (const auto& kv : loadSet->embedded_loadsets) {
            const auto& otherloadSet = this->find(kv.first);
            if (otherloadSet == nullptr) {
                throw logic_error("CombinedLoadSet: missing loadSet " + to_str(kv.first));
            }
            double coefficient = kv.second;
            for (const auto& loading : otherloadSet->getLoadings()) {
                const shared_ptr<Loading>& newLoading = loading->clone();
                newLoading->resetId();
                newLoading->scale(coefficient);
                this->add(newLoading);
                this->addLoadingIntoLoadSet(*newLoading, *loadSet);
                if (configuration.logLevel >= LogLevel::DEBUG) {
                    cout << "Cloned " << *loading << " into " << *newLoading << " and scaled by "
                            << coefficient << " and assigned to " << *loadSet << endl;
                }
                loading->markAsWritten();
            }
            otherloadSet->markAsWritten();
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
                if (not assertionDOFS.empty()) {
                    const Node& node = mesh.findNode(nodePosition);
                    const DOFS& availableDOFS = node.dofs + analysis->findBoundaryDOFS(nodePosition);
                    if (not availableDOFS.containsAll(assertionDOFS)) {
                        objectivesToRemove.push_back(assertion);
                    }
                }
            }
        }
    }
    for (const auto& objective : objectivesToRemove) {
        if (configuration.logLevel >= LogLevel::TRACE)
            cout << "Removed ineffective " << *objective << endl;

        remove(objective->getReference());
    }
}

void Model::addDefaultAnalysis()
{
    if (this->analyses.empty() and (loadings.empty() or constraints.empty())) {
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
        const auto& matrix = static_pointer_cast<MatrixElement>(elementSetM);
        for (int nodePosition : matrix->nodePositions()) {
            requiredDofsByNode[nodePosition] = DOFS();
            const int nodeId = mesh.findNodeId(nodePosition);
            DOFS owned;
            for (const auto elementSetI : elementSets) {
                if (not elementSetI->effective()) {
                    continue;
                }
                for (const int cellPosition : elementSetI->cellPositions()) {
                    const Cell& cell = mesh.findCell(cellPosition);
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
                if (not matrix->findInPairs(pair.first).empty()) {
                    continue; // will be handled by a segment cell with another node
                }
                // single node
                int nodePosition = pair.first;
                const int nodeId = mesh.findNodeId(nodePosition);
                DOFS requiredDofs = requiredDofsByNode.find(nodePosition)->second;
                const auto& submatrix = matrix->findSubmatrix(nodePosition, nodePosition);
                const auto& discrete = make_shared<DiscretePoint>(*this, submatrix->matrixType);
                for (const auto& kv : submatrix->componentByDofs) {
                    double value = kv.second;
                    const vega::DOF& dof1 = kv.first.first;
                    const vega::DOF& dof2 = kv.first.second;
                    if (!is_equal(value, 0)) {
                        switch (matrix->type) {
                        case ElementSet::Type::STIFFNESS_MATRIX:
                            discrete->addStiffness(dof1, dof2, value);
                            break;
                        case ElementSet::Type::MASS_MATRIX:
                            discrete->addMass(dof1, dof2, value);
                            break;
                        case ElementSet::Type::DAMPING_MATRIX:
                            discrete->addDamping(dof1, dof2, value);
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
                const auto& matrixGroup = mesh.createCellGroup(
                        "MTN" + to_string(matrix_count), Group::NO_ORIGINAL_ID, oss.str());
                discrete->add(*matrixGroup);
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

                const auto& discrete = make_shared<DiscreteSegment>(*this, matrix->matrixType);
                ostringstream oss;
                oss << "created by replaceDirectMatrices() because of matrix element on node couple: " << matrix;
                const auto& matrixGroup = mesh.createCellGroup(
                        "MTL" + to_string(matrix_count), Group::NO_ORIGINAL_ID, oss.str());
                matrix_count++;
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, { rowNodeId,
                        colNodeId }, true);
                matrixGroup->addCellPosition(cellPosition);
                if (configuration.addVirtualMaterial) {
                    discrete->assignMaterial(getVirtualMaterial());
                }
                discrete->add(*matrixGroup);
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
                        shared_ptr<const DOFMatrix> submatrix = matrix->findSubmatrix(rowNodePosition2,
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
                                case ElementSet::Type::MASS_MATRIX:
                                    discrete->addMass(row_index, col_index, rowDof, colDof,
                                            value);
                                    break;
                                case ElementSet::Type::DAMPING_MATRIX:
                                    discrete->addDamping(row_index, col_index, rowDof, colDof,
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
                if (this->configuration.logLevel >= LogLevel::TRACE) {
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
            const auto& constraintNodes = constraint->nodePositions();
            if (constraintNodes.find(nodePosition) == constraintNodes.end()) {
                continue;
            }
            required += constraint->getDOFSForNode(nodePosition);
        }
        const DOFS& extra = added - owned - required;
        if (extra != DOFS::NO_DOFS) {
            const auto& spc = make_shared<SinglePointConstraint>(*this, extra);
            spc->addNodePosition(nodePosition);
            add(spc);
            addConstraintIntoConstraintSet(*spc, *commonConstraintSet);
            if (configuration.logLevel >= LogLevel::TRACE) {
                cout << "Adding virtual spc on node id: " << this->mesh.findNodeId(nodePosition) << " for " << extra
                        << endl;
            }
        }
    }
    for (const auto& elementSet : elementSetsToRemove) {
        if (configuration.logLevel >= LogLevel::TRACE) {
            cout << "Replaced " << *elementSet << endl;
        }
        this->elementSets.erase(elementSet->getReference());
    }
}

void Model::replaceRigidSegments()
{
    for (const auto& elementSet : elementSets.filter(ElementSet::Type::STRUCTURAL_SEGMENT)) {
        const auto& segment = static_pointer_cast<StructuralSegment>(elementSet);
        if (not segment->isDiagonalRigid()) {
            continue;
        }
        for (const auto& cell : segment->getCellsIncludingGroups()) {
            const int masterId = cell.nodeIds.front();
            const set<int> slaveNodeIds{cell.nodeIds.begin()+1, cell.nodeIds.end()};
            const auto& rigid = make_shared<RigidConstraint>(*this, masterId, RigidConstraint::NO_ORIGINAL_ID, slaveNodeIds);
            this->add(rigid);
            addConstraintIntoConstraintSet(*rigid, *commonConstraintSet);
            if (configuration.logLevel >= LogLevel::TRACE) {
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
            const auto& spcs = constraintSet->getConstraintsByType(Constraint::Type::SPC);
            if (spcs.empty()) {
                continue;
            }
            for (const auto& constraint : spcs) {
                const auto& spc = static_pointer_cast<SinglePointConstraint>(constraint);
                for (int nodePosition : spc->nodePositions()) {
                    DOFS dofsToRemove;
                    DOFS blockedDofs = spc->getDOFSForNode(nodePosition);
                    for (const DOF dof : blockedDofs) {
                        pair<int, DOF> key{nodePosition, dof};
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
                    if (not dofsToRemove.empty()) {
                        analysis->removeSPCNodeDofs(*spc, nodePosition, dofsToRemove);
                        if (configuration.logLevel >= LogLevel::TRACE) {
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
            const auto& loads = loadSet->getLoadingsByType(Loading::Type::IMPOSED_DISPLACEMENT);
            if (loads.empty()) {
                continue;
            }
            for (const auto& load : loads) {
                const auto& impo = static_pointer_cast<ImposedDisplacement>(load);
                for (int nodePosition : impo->nodePositions()) {
                    imposedDofsByNodeId[nodePosition] = impo->getDOFSForNode(nodePosition);
                }
            }
        }
        for (const auto& constraintSet : analysis->getConstraintSets()) {
            const auto& spcs = constraintSet->getConstraintsByType(Constraint::Type::SPC);
            if (spcs.empty()) {
                continue;
            }
            for (const auto& constraint : spcs) {
                const auto& spc = static_pointer_cast<SinglePointConstraint>(constraint);
                for (int nodePosition : spc->nodePositions()) {
                    DOFS imposedDofs = imposedDofsByNodeId[nodePosition];
                    DOFS blockedDofs = spc->getDOFSForNode(nodePosition);
                    DOFS dofsToRemove = imposedDofs.intersection(blockedDofs);
                    if (not dofsToRemove.empty()) {
                        analysis->removeSPCNodeDofs(*spc, nodePosition, dofsToRemove);
                        if (configuration.logLevel >= LogLevel::TRACE) {
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

        const auto& matrix = static_pointer_cast<MatrixElement>(elementSetM);

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
        const shared_ptr<ElementSet>& dummyElement = elementSetM->clone();
        const auto& dummyMatrix = dynamic_pointer_cast<MatrixElement>(dummyElement);
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
                ps = {sI,sJ};
            }else{
                ps = {sJ,sI};
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
                    esToAddByStackNumber[{stF,stF}]=newElementSet;
                    esToAddByStackNumber[{stF,stF+1}]=newElementSet;
                    esToAddByStackNumber[{stF+1,stF+1}]=newElementSet;
                }
            } else {
                newElementSet = it2->second;
            }

            // We copy the values
            const auto& nM = static_pointer_cast<MatrixElement>(newElementSet);
            const auto& dM = matrix->findSubmatrix(np.first, np.second);
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



void Model::makeCellsFromDirectMatrices() {

    int idM=0;
    for (const auto& elementSetM : elementSets) {
        if (!elementSetM->isMatrixElement()) {
            //TODO: Display informative message in debug mode.
            continue;
        }
        const auto& matrix = static_pointer_cast<MatrixElement>(elementSetM);

        // If cells already exists, we do nothing
        if (!matrix->empty()) {
            //TODO: Display informative message in debug mode.
            continue;
        }

        // If matrix is void, we do nothing (should not happen)
        if (matrix->nodePositions().empty()) {
            //TODO: Display informative message in debug mode.
            continue;
        }

        // Create a Cell Group
        idM++;
        const auto& matrixGroup = mesh.createCellGroup("DM" + to_string(idM), Group::NO_ORIGINAL_ID, "Direct Matrix "+ elementSetM->name);
        matrix->add(*matrixGroup);

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

void Model::makeCellsFromLMPC() {

    shared_ptr<Material> materialLMPC = nullptr;

    for (const auto& analysis : this->analyses) {
        for (const auto& constraintSet : analysis->getConstraintSets()) {

            // Group lmpcs by nodePositions
            map<set<int>, vector<shared_ptr<LinearMultiplePointConstraint>>> lmpcsByNodepositions;
            for (const auto& constraint : constraintSet->getConstraintsByType(Constraint::Type::LMPC)) {
                const auto& lmpc = static_pointer_cast<LinearMultiplePointConstraint>(constraint);
                const auto& nodePositions = lmpc->nodePositions();
                const auto& it = lmpcsByNodepositions.find(nodePositions);
                if (it == lmpcsByNodepositions.end()) {
                    lmpcsByNodepositions[nodePositions] = {lmpc};
                } else {
                    lmpcsByNodepositions[nodePositions].push_back(lmpc);
                }
            }

            // Re-group nodePositions by cell dof coefs
            map<vector<vector<DOFCoefs>>, shared_ptr<Lmpc>> elmpcByDOFCoefs;
            for(const auto& lmpcEntry : lmpcsByNodepositions) {
                const auto& sortedNodePositions = lmpcEntry.first;
                const auto& lmpcs = lmpcEntry.second;

                vector<vector<DOFCoefs>> cellDofCoefs;
                for (const auto& lmpc : lmpcs) {
                    // We sort the Coeffs in order to fuse various LMPC into the same ElementSet/CellGroup
                    vector<DOFCoefs> sortedCoefs;
                    for (int nodePosition : sortedNodePositions){
                        sortedCoefs.push_back(lmpc->getDoFCoefsForNode(nodePosition));
                    }
                    cellDofCoefs.push_back(sortedCoefs);
                    lmpc->markAsWritten();
                }
                for (size_t i = 0; i < cellDofCoefs.size(); i += Lmpc::LMPCCELL_DOFNUM) {
                    const auto& cellDofCoefsSlice = vector<vector<DOFCoefs>>(cellDofCoefs.begin() + i, cellDofCoefs.begin() + min(i + Lmpc::LMPCCELL_DOFNUM, cellDofCoefs.size()));

                    vector<int> sortedNodeIds;
                    for (int nodePosition : sortedNodePositions ) {
                        sortedNodeIds.push_back(mesh.findNodeId(nodePosition));
                    }
                    int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::polyType(static_cast<unsigned int>(sortedNodeIds.size())), sortedNodeIds, true);

                    const auto& it = elmpcByDOFCoefs.find(cellDofCoefsSlice);
                    shared_ptr<Lmpc> elementsetLMPC = nullptr;
                    if (it == elmpcByDOFCoefs.end()) {

                        if (materialLMPC == nullptr){
                            materialLMPC = make_shared<Material>(*this);
                            materialLMPC->addNature(make_shared<RigidNature>(*this, 1));
                            this->add(materialLMPC);
                        }
                        elementsetLMPC = make_shared<Lmpc>(*this, analysis->getId());
                        elementsetLMPC->assignMaterial(materialLMPC);
                        elmpcByDOFCoefs[cellDofCoefsSlice] = elementsetLMPC;
                        for (const auto& lmpcDofCoefs : cellDofCoefs) {
                            elementsetLMPC->appendDofCoefs(lmpcDofCoefs);
                        }
                        this->add(elementsetLMPC);
                        if (configuration.logLevel >= LogLevel::DEBUG){
                            cout << "Created a new elementSet " << *elementsetLMPC << " for cell " << cellPosition << " for lmpcs " << cellDofCoefs << " ." << endl;
                        }
                    } else {
                        elementsetLMPC = elmpcByDOFCoefs[cellDofCoefsSlice];
                        if (configuration.logLevel >= LogLevel::DEBUG){
                            cout << "Adding to existing elementSet " << *elementsetLMPC << " the cell " << cellPosition << " for lmpcs " << cellDofCoefs << " ." << endl;
                        }
                    }
                    elementsetLMPC->addCellPosition(cellPosition);
                }
            }
        }
    }

    // After the translation, we remove all LMPC constraints
    // We don't do this during the loop because some LMPC may be used by several analysis.
    for (const auto& analysis : this->analyses) {
        for (const auto& constraintSet : analysis->getConstraintSets()) {
            for (const auto& constraint : constraintSet->getConstraintsByType(Constraint::Type::LMPC)) {
                remove(constraint->getReference(), constraintSet->getReference());
                constraint->markAsWritten();
            }
            if (constraintSet->empty())
                constraintSet->markAsWritten();
        }
    }
}

void Model::makeCellsFromRBE(){

    for (const auto& constraintSet : this->getCommonConstraintSets()) {

        // Translation of RBAR and RBE2 (RBE2 are viewed as an assembly of RBAR)
        // See Systus Reference Analysis Manual: RIGID BODY Element (page 498)
        vector<shared_ptr<Constraint>> toBeRemoved;
        for (const auto& constraint : constraintSet->getConstraintsByType(Constraint::Type::RIGID)) {
            const auto& rbe2 = static_pointer_cast<RigidConstraint>(constraint);

            // Creating an elementset, a CellGroup and a dummy rigid material
            const auto& materialRBE2 = make_shared<Material>(*this);
            materialRBE2->addNature(make_shared<RigidNature>(*this, 1));
            this->add(materialRBE2);

            //const auto& group = mesh.createCellGroup("RBE2_"+to_string(constraint->bestId()), CellGroup::NO_ORIGINAL_ID, "RBE2");
            const auto& elementsetRbe2 = make_shared<Rbar>(*this, mesh.findNodeId(rbe2->getMaster()));
            //elementsetRbe2->add(*group);
            elementsetRbe2->assignMaterial(materialRBE2);
            this->add(elementsetRbe2);

            // Creating cells and adding them to the CellGroup
            const int masterId = mesh.findNodeId(rbe2->getMaster());
            for (int position : rbe2->getSlaves()){
                const int slaveId = mesh.findNodeId(position);
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, {masterId, slaveId}, true);
                //group->addCellPosition(cellPosition);
                elementsetRbe2->addCellPosition(cellPosition);
            }

            // Removing the constraint from the model.
            toBeRemoved.push_back(constraint);
            //if (configuration.logLevel >= LogLevel::TRACE){
            //    cout << "Building cells in cellgroup "<<group->getName()<<" from "<< *rbe2<<"."<<endl;
            //}
        }

        for (const auto& constraint : constraintSet->getConstraintsByType(Constraint::Type::QUASI_RIGID)) {
            const auto& rbar = static_pointer_cast<QuasiRigidConstraint>(constraint);

            if (not rbar->isCompletelyRigid()){
                cerr << "QUASI_RIGID constraint not implemented yet for RBARs. Constraint "+to_string(constraint->bestId())+ " translated as rigid constraint."<<endl;
            }
            //if (rbar->getSlaves().size()!=2){
            //   throw logic_error("QUASI_RIGID constraint must have exactly two slaves.");
            //}

            // Creating an elementset, a CellGroup and a dummy rigid material
            const auto& materialRBAR = make_shared<Material>(*this);
            materialRBAR->addNature(make_shared<RigidNature>(*this, 1));
            this->add(materialRBAR);

            int masterId;
            if (rbar->hasMaster()) {
                masterId = mesh.findNodeId(rbar->getMaster());
            } else {
                masterId = mesh.findNodeId(*rbar->getSlaves().begin());
            }

            const auto& elementsetRBAR = make_shared<Rbar>(*this, masterId);
            elementsetRBAR->assignMaterial(materialRBAR);
            this->add(elementsetRBAR);

            //const auto& group = mesh.createCellGroup("RBAR_"+to_string(constraint->bestId()), CellGroup::NO_ORIGINAL_ID, "RBAR");

            for (int position : rbar->getSlaves()) {
                const int slaveId = mesh.findNodeId(position);
                if (not rbar->hasMaster() and slaveId == masterId)
                    continue;
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, {masterId, slaveId}, true);
                //group->addCellPosition(cellPosition);
                elementsetRBAR->addCellPosition(cellPosition);
            }
            //elementsetRBAR->add(*group);
            //if (configuration.logLevel >= LogLevel::TRACE){
            //    cout << "Building cells in cellgroup "<<group->getName()<<" from "<< *rbar<<"."<<endl;
            //}

            // Removing the constraint from the model.
            toBeRemoved.push_back(constraint);

        }


        /* Translation of RBE3
         * See Systus Reference Analysis Manual, Section 8.8 "Special Elements",
         * Subsection "Use of Averaging Type Solid Elements", p500.
         */
        for (const auto& constraint : constraintSet->getConstraintsByType(Constraint::Type::RBE3)) {

            const auto& rbe3 = static_pointer_cast<RBE3>(constraint);
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
                const auto& it = groupByCoefByDOFS.find(sDOFS);
                if (it !=groupByCoefByDOFS.end()){
                    auto it2 = it->second.find(sCoef);
                    if (it2 != it->second.end())
                        groupRBE3 = it2->second;
                }

                if (groupRBE3==nullptr){

                    // Creating an elementset, a CellGroup and a dummy rigid material
                    nbParts++;
                    const auto& materialRBE3 = make_shared<Material>(*this);
                    materialRBE3->addNature(make_shared<RigidNature>(*this, Nature::UNAVAILABLE_DOUBLE, sCoef));
                    this->add(materialRBE3);

                    const auto& group = mesh.createCellGroup("RBE3_"+to_string(nbParts)+"_"+to_string(constraint->bestId()), CellGroup::NO_ORIGINAL_ID, "RBE3");
                    const auto& elementsetRbe3 = make_shared<Rbe3>(*this, masterId, mDOFS, sDOFS);
                    elementsetRbe3->add(*group);
                    elementsetRbe3->assignMaterial(materialRBE3);
                    this->add(elementsetRbe3);

                    if (configuration.logLevel >= LogLevel::TRACE){
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
            remove(constraint->getReference(), constraintSet->getReference());
            constraint->markAsWritten();
        }
        if (constraintSet->empty())
            constraintSet->markAsWritten();
    }
}

void Model::makeCellsFromSurfaceSlide() {
    for (const auto& constraintSet : this->getCommonConstraintSets()) {

        vector<shared_ptr<Constraint>> toBeRemoved;
        for (const auto& constraint : constraintSet->getConstraintsByType(Constraint::Type::SURFACE_SLIDE_CONTACT)) {
            const auto& surface = static_pointer_cast<SurfaceSlide>(constraint);
            surface->makeCellsFromSurfaceSlide();
            toBeRemoved.push_back(constraint);
        }

        for(const auto& constraint: toBeRemoved) {
            remove(constraint->getReference(), constraintSet->getReference());
            constraint->markAsWritten();
        }
        if (constraintSet->empty())
            constraintSet->markAsWritten();
    }
}

void Model::splitElementsByDOFS(){

    vector<shared_ptr<ScalarSpring>> elementSetsToAdd;
    vector<shared_ptr<ElementSet>> elementSetsToRemove;

    for (const auto& elementSet : elementSets.filter(ElementSet::Type::SCALAR_SPRING)) {
        const auto& ss = static_pointer_cast<ScalarSpring>(elementSet);
        if (ss->getNbDOFSSpring()>1) {
            int i =1;
            const double stiffness = ss->getStiffness();
            const double damping = ss->getDamping();
            if (configuration.logLevel >= LogLevel::DEBUG)
                cout<< *elementSet << " spring must be split."<<endl;
            for (const auto & it : ss->getCellPositionByDOFS()){
                const auto& scalarSpring = make_shared<ScalarSpring>(*this, Identifiable<ElementSet>::NO_ORIGINAL_ID, stiffness, damping);
                const auto& cellGroup = this->mesh.createCellGroup(ss->name+"_"+to_string(i), Group::NO_ORIGINAL_ID);
                scalarSpring->add(*cellGroup);
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
        for (const auto& constraint : constraintSet->getConstraintsByType(Constraint::Type::SLIDE)) {
            const auto& slide = static_pointer_cast<SlideContact>(constraint);
            const auto& masterCellGroup = mesh.createCellGroup("SLIDE_M_"+to_string(slide->bestId()), Group::NO_ORIGINAL_ID, "created by makeBoundarySegments() for the master in a SLIDE contact");
            const auto& slaveCellGroup = mesh.createCellGroup("SLIDE_S_"+to_string(slide->bestId()), Group::NO_ORIGINAL_ID, "created by makeBoundarySegments() for the slave in a SLIDE contact");
            const auto& masterNodeLine = static_pointer_cast<BoundaryNodeLine>(this->find(slide->master));
            const auto& masterNodeIds = masterNodeLine->nodeids;
            auto it = masterNodeIds.begin();
            for(unsigned int i = 0; i < masterNodeIds.size() - 1;++i) {
                int nodeId1 = *it;
                int nodeId2 = *(++it);
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, {nodeId1, nodeId2}, true);
                masterCellGroup->addCellPosition(cellPosition);
            }
            slide->masterCellGroup = masterCellGroup;
            masterNodeLine->markAsWritten();
            const auto& slaveNodeLine = static_pointer_cast<BoundaryNodeLine>(this->find(slide->slave));
            const auto& slaveNodeIds = slaveNodeLine->nodeids;
            auto it2 = slaveNodeIds.begin();
            for(unsigned int i = 0; i < slaveNodeIds.size() - 1;++i) {
                int nodeId1 = *it2;
                int nodeId2 = *(++it2);
                int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, {nodeId1, nodeId2}, true);
                slaveCellGroup->addCellPosition(cellPosition);
            }
            slide->slaveCellGroup = slaveCellGroup;
            slaveNodeLine->markAsWritten();
        }
    }
}

void Model::makeBoundarySurfaces() {
    for (const auto& constraintSet : this->getCommonConstraintSets()) {
        for (const auto& constraint : constraintSet->getConstraintsByType(Constraint::Type::SURFACE_CONTACT)) {
            const auto& surface = static_pointer_cast<SurfaceContact>(constraint);
            surface->makeBoundarySurfaces();
        }
    }
}

void Model::addAutoAnalysis() {
    // LD very basic implementation of analysis detection. Should also look for non linear materials etc ?
    if (objectives.contains(Objective::Type::NONLINEAR_PARAMETERS)) {
        const auto& analysis = make_shared<NonLinearMecaStat>(*this, this->commonObjectiveSet);
        this->add(analysis);
    } else if (objectives.contains(Objective::Type::FREQUENCY_SEARCH)) {
        const auto& analysis = make_shared<LinearModal>(*this, this->commonObjectiveSet);
        this->add(analysis);
    } else {
        const auto& analysis = make_shared<LinearMecaStat>(*this);
        this->add(analysis);
    }
}

void Model::changeParametricForceLineToAbsolute() {
    for (const auto& loading: this->loadings.filter(Loading::Type::FORCE_LINE)) {
        const auto& forceLine = static_pointer_cast<ForceLine>(loading);
        if (not forceLine->hasFunctions())
            continue;
        if (forceLine->getCellPositionsIncludingGroups().size() != 1)
            throw logic_error("Conversion of more than one cell per ForceLine not yet implemented (but should be doable)");
        const int cellPosition = *(forceLine->getCellPositionsIncludingGroups().begin());
        const Cell& cell = mesh.findCell(cellPosition);
        const Node& node1 = mesh.findNode(cell.nodePositions[0]);
        const Node& node2 = mesh.findNode(cell.nodePositions[1]);
        if (is_equal(node1.x, node2.x))
            throw logic_error("Need to implement beams oriented on the Y or Z axis too.");
        const auto& functionTable = dynamic_pointer_cast<FunctionTable>(forceLine->force);
        if (functionTable == nullptr)
            throw logic_error("Cannot convert to absolute this kind of Function");
        if (functionTable->getParaX() != Function::ParaName::ABSC)
            continue;
        const auto& absoluteFunction = make_shared<FunctionTable>(*this, functionTable->parameter, functionTable->value, functionTable->left, functionTable->right, functionTable->getOriginalId());
        absoluteFunction->setParaX(Function::ParaName::PARAX);
        absoluteFunction->setParaY(functionTable->getParaY());

        if (cell.nodePositions.size() != 2)
            throw logic_error("Conversion of cell without two nodeids per ForceLine not yet implemented (but should be doable)");
        for (auto it = functionTable->getBeginValuesXY(); it != functionTable->getEndValuesXY(); it++) {
            double absc = it->first;
            double val = it->second;
            absoluteFunction->setXY(node1.x * (1-absc) + node2.x * absc, val);
        }
        this->add(absoluteFunction);
        forceLine->force = absoluteFunction;
    }
}

void Model::convert0DDiscretsInto1D() {
    vector<shared_ptr<ElementSet>> elementSetsToAdd;
    vector<shared_ptr<ElementSet>> elementSetsToRemove;
    for (const auto& elementSet: this->elementSets.filter(ElementSet::Type::DISCRETE_0D)) {
        const auto& discretePoint = static_pointer_cast<DiscretePoint>(elementSet);
        if (discretePoint->empty())
            continue;
        const auto& discreteSegment = make_shared<StructuralSegment>(*this, discretePoint->matrixType);
        for (const DOF dof1 : DOFS::ALL_DOFS) {
            for (const DOF dof2 : DOFS::ALL_DOFS) {
                const auto& stiffnessValue = discretePoint->findStiffness(dof1, dof2);
                discreteSegment->addStiffness(dof1, dof2, stiffnessValue);
                const auto& massValue = discretePoint->findMass(dof1, dof2);
                discreteSegment->addMass(dof1, dof2, massValue);
                const auto& dampingValue = discretePoint->findDamping(dof1, dof2);
                discreteSegment->addDamping(dof1, dof2, dampingValue);
            }
        }

        const auto& discret0D1DCellGroup = mesh.createCellGroup("DIS0D1D_"+to_string(discretePoint->bestId()), Group::NO_ORIGINAL_ID, "created by convert0DDiscretsInto1D() for a 0D discret element");
        for (int nodePosition: discretePoint->nodePositions()) {
            const int nodeId = this->mesh.findNodeId(nodePosition);
            int cellPosition = mesh.addCell(Cell::AUTO_ID, CellType::SEG2, {nodeId, nodeId}, true);
            discret0D1DCellGroup->addCellPosition(cellPosition);

        }
        elementSetsToRemove.push_back(discretePoint);
        elementSetsToAdd.push_back(discreteSegment);
    }
    for (const auto& elementSet : elementSetsToRemove){
        this->elementSets.erase(elementSet->getReference());
    }
    for (const auto& elementSet : elementSetsToAdd){
        this->add(elementSet);
    }
}

void Model::createSetGroups() {
    for (const auto& loadSet : this->loadSets) {
        if (not loadSet->isOriginal())
            continue;
        const auto& loadSetNodeGroup = mesh.createNodeGroup(loadSet->getGroupName(), Group::NO_ORIGINAL_ID, "Display nodegroup for loadset : " + LoadSet::stringByType.at(loadSet->type) + "_" + to_string(loadSet->bestId()));
        for (const auto& loading : loadSet->getLoadings()) {
            loadSetNodeGroup->addNodePositions(loading->nodePositions());
        }
    }
    for (const auto& constraintSet : this->constraintSets) {
        if (not constraintSet->isOriginal())
            continue;
        const auto& constraintSetGroup = mesh.createNodeGroup(constraintSet->getGroupName(), Group::NO_ORIGINAL_ID, "Display nodegroup for constraintset : " + ConstraintSet::stringByType.at(constraintSet->type) + "_" + to_string(constraintSet->bestId()));
        for (const auto& constraint : constraintSet->getConstraints()) {
            constraintSetGroup->addNodePositions(constraint->nodePositions());
        }
    }
    for (auto output : objectives.filter(Objective::Type::NODAL_DISPLACEMENT_OUTPUT)) {
        if (not output->isOriginal())
            continue;
        const auto& displacementOutput = static_pointer_cast<const NodalDisplacementOutput>(output);
        if (not displacementOutput->hasNodeGroups())
            continue;
        displacementOutput->getNodeGroups(); // LD TODO Lazy creation :..(
    }
    /*for (auto output : objectives.filter(Objective::Type::VONMISES_STRESS_OUTPUT)) {
        if (not output->isOriginal())
            continue;
        const auto& vonMisesOutput = static_pointer_cast<const VonMisesStressOutput>(output);
        if (not vonMisesOutput->hasCellGroups())
            continue;
        vonMisesOutput->getCellGroups(); // LD TODO Lazy creation :..(
    }*/
}

void Model::splitElementsByCellOffsets() {
    vector<shared_ptr<ElementSet>> elementSetsToAdd;
    vector<shared_ptr<ElementSet>> elementSetsToRemove;
    map<long, shared_ptr<CellContainer>> cellPositionsByRoundedOffset;

    for (const auto& elementSet : elementSets.filter(ElementSet::Type::SHELL)) {
        const auto& shell = static_pointer_cast<Shell>(elementSet);
        if (not shell->containsNonzeroOffsetCell())
            continue;
        elementSetsToRemove.push_back(shell);
        for (const Cell& cell : shell->getCellsIncludingGroups()) {
            long offsetRounded = lround(pow(cell.offset,6));
            const auto& it = cellPositionsByRoundedOffset.find(offsetRounded);
            if (it == cellPositionsByRoundedOffset.end()) {
                const shared_ptr<ElementSet>& clonedElement = shell->clone();
                const auto& clonedShell = dynamic_pointer_cast<Shell>(clonedElement);
                clonedShell->clear();
                clonedShell->offset = shell->offset + cell.offset;
                clonedShell->add(cell);
                cellPositionsByRoundedOffset[offsetRounded] = clonedShell;
                elementSetsToAdd.push_back(clonedShell);
            } else {
                it->second->add(cell);
            }
        }
    }

    for (const auto& elementSet : elementSets.filter(ElementSet::Type::COMPOSITE)) {
        const auto& composite = static_pointer_cast<Composite>(elementSet);
        if (not composite->containsNonzeroOffsetCell())
            continue;
        elementSetsToRemove.push_back(composite);
        for (const Cell& cell : composite->getCellsIncludingGroups()) {
            long offsetRounded = lround(pow(cell.offset,6));
            const auto& it = cellPositionsByRoundedOffset.find(offsetRounded);
            if (it == cellPositionsByRoundedOffset.end()) {
                const shared_ptr<ElementSet>& clonedElement = composite->clone();
                const auto& clonedComposite = dynamic_pointer_cast<Composite>(clonedElement);
                clonedComposite->clear();
                clonedComposite->offset = composite->offset + cell.offset;
                clonedComposite->add(cell);
                cellPositionsByRoundedOffset[offsetRounded] = clonedComposite;
                elementSetsToAdd.push_back(clonedComposite);
            } else {
                it->second->add(cell);
            }
        }
    }

    for (const auto& elementSet : elementSetsToRemove){
        this->elementSets.erase(Reference<ElementSet>(*elementSet));
    }
    for (const auto& elementSet : elementSetsToAdd){
        this->add(elementSet);
    }
}

void Model::finish() {
    if (finished) {
        return;
    }

    /* Build the coordinate systems from their definition points */
    for (const auto& coordinateSystemEntry : mesh.coordinateSystemStorage.coordinateSystemByRef) {
        coordinateSystemEntry.second->build();
    }

    for (const auto& elementSet : elementSets) {
        for (int nodePosition : elementSet->nodePositions()) {
            mesh.allowDOFS(nodePosition,elementSet->getDOFSForNode(nodePosition));
        }
    }

    if (this->configuration.autoDetectAnalysis and analyses.empty()) {
        addAutoAnalysis();
    }

    if (this->configuration.createSkin) {
        generateSkin();
    }

    for (const auto& analysis : analyses) {
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

    for (const auto& constraint : constraints.filter(Constraint::Type::QUASI_RIGID)) {
        const auto& rigid = static_pointer_cast<QuasiRigidConstraint>(constraint);
        if (this->configuration.convertCompletelyRigidsIntoMPCs or not rigid->isCompletelyRigid())
            rigid->emulateWithMPCs();
    }

    if (this->configuration.displayMasterSlaveConstraint) {
        generateBeamsToDisplayMasterSlaveConstraint();
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

    if (this->configuration.convert0DDiscretsInto1D) {
        convert0DDiscretsInto1D();
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

    if (this->configuration.splitElementsByCellOffsets){
        //splitElementsByCellOffsets();
    }

    assignElementsToCells();
    generateMaterialAssignments();

    if (this->configuration.changeParametricForceLineToAbsolute) {
        changeParametricForceLineToAbsolute();
    }

    createSetGroups();

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
    size_t sizeMat = materials.size();     string sMat = ( (sizeMat > 1) ? "s are " : " is ");
    size_t sizeEle = elementSets.size();   string sEle = ( (sizeEle > 1) ? "s are " : " is ");
    size_t sizeLoa = loadings.size();      string sLoa = ( (sizeLoa > 1) ? "s are " : " is ");
    size_t sizeLos = loadSets.size();      string sLos = ( (sizeLos > 1) ? "s are " : " is ");
    size_t sizeCon = constraints.size();   string sCon = ( (sizeCon > 1) ? "s are " : " is ");
    size_t sizeCos = constraintSets.size();string sCos = ( (sizeCos > 1) ? "s are " : " is ");
    size_t sizeAna = analyses.size();      string sAna = ( (sizeAna > 1) ? "es are " : "is is ");
    size_t sizeTar = targets.size();       string sTar = ( (sizeTar > 1) ? "s are " : " is ");
    size_t sizeObj = objectives.size();    string sObj = ( (sizeObj > 1) ? "s are " : " is ");

    bool validMat = materials.validate();
    bool validEle = elementSets.validate();
    bool validLoa = loadings.validate();
    bool validLos = loadSets.validate();
    bool validCon = constraints.validate();
    bool validCos = constraintSets.validate();
    bool validAna = analyses.validate();
    bool validTar = targets.validate();
    bool validObj = objectives.validate();

    if (configuration.logLevel >= LogLevel::DEBUG) {
       cout << "The " << sizeMat << " material"     << sMat << (validMat ? "" : "NOT ") << "valid." << endl;
       cout << "The " << sizeEle << " elementSet"   << sEle << (validEle ? "" : "NOT ") << "valid." << endl;
       cout << "The " << sizeLoa << " loading"      << sLoa << (validLoa ? "" : "NOT ") << "valid." << endl;
       cout << "The " << sizeLos << " loadSet"      << sLos << (validLos ? "" : "NOT ") << "valid." << endl;
       cout << "The " << sizeCon << " constraint"   << sCon << (validCon ? "" : "NOT ") << "valid." << endl;
       cout << "The " << sizeCos << " constraintSet"<< sCos << (validCos ? "" : "NOT ") << "valid." << endl;
       cout << "The " << sizeAna << " analys"      << sAna << (validAna ? "" : "NOT ") << "valid." << endl;
       if (sizeTar >= 1) cout << "The " << sizeTar << " target"      << sTar << (validTar ? "" : "NOT ") << "valid." << endl;
       if (sizeObj >= 1) cout << "The " << sizeObj << " objective"      << sObj << (validObj ? "" : "NOT ") << "valid." << endl;
    }
    bool allValid = meshValid && validMat && validEle && validLoa && validLos && validCon
            && validCos && validAna && validTar && validObj;
    this->afterValidation = true;
    return allValid;
}

bool Model::checkWritten() const {

    // Sizes are stocked now, because validation remove invalid objects.
    size_t sizeMat = materials.size();
    size_t sizeEle = elementSets.size();
    size_t sizeLoa = loadings.size();
    size_t sizeLos = loadSets.size();
    size_t sizeCon = constraints.size();
    size_t sizeCos = constraintSets.size();
    size_t sizeAna = analyses.size();
    size_t sizeTar = targets.size();
    size_t sizeObj = objectives.size();

    bool validMat = materials.checkWritten();
    bool validEle = elementSets.checkWritten();
    bool validLoa = true;
    bool validLos = true;
    bool validCon = true;
    bool validCos = true;
    bool validAna = analyses.checkWritten();
    bool validTar = true;
    bool validObj = true;

    for (const auto& analysis : analyses) {
        if (not analysis->isWritten()) {
            continue;
        }
        if (configuration.logLevel >= LogLevel::TRACE) {
            cerr << "Checking if analysis:" << *analysis << " has been written" << endl;
        }
        for (const auto& loadSet : analysis->getLoadSets()) {
            validLos = validLos and loadSet->isWritten();
            for (const auto& loading : loadSet->getLoadings()) {
                validLoa = validLoa and loading->isWritten();
            }
        }

        for (const auto& constraintSet : analysis->getConstraintSets()) {
            validCos = validCos and constraintSet ->isWritten();
            for (const auto& constraint : constraintSet->getConstraints()) {
                if (configuration.logLevel >= LogLevel::DEBUG and not constraint->isWritten()) {
                    cerr << "Constraint:" << *constraint << " has not been written" << endl;
                }
                validCon = validCon and constraint->isWritten();
            }
        }

        for (const auto& objective : analysis->getObjectives()) {
            if (objective->isWritten()) {
                continue;
            }
            if (objective->isOutput()) {
                if (configuration.logLevel >= LogLevel::TRACE) {
                    cerr << "Output objective:" << *objective << " has not been written" << endl;
                }
            }
            validObj = validObj and objective->isWritten();
            if (configuration.logLevel >= LogLevel::DEBUG) {
                cerr << "Objective:" << *objective << " has not been written" << endl;
            }
        }

    }

    string sMat = ( (sizeMat > 1) ? "s have " : " has ");
    string sEle = ( (sizeEle > 1) ? "s have " : " has ");
    string sLoa = ( (sizeLoa > 1) ? "s have " : " has ");
    string sLos = ( (sizeLos > 1) ? "s have " : " has ");
    string sCon = ( (sizeCon > 1) ? "s have " : " has ");
    string sCos = ( (sizeCos > 1) ? "s have " : " has ");
    string sAna = ( (sizeAna > 1) ? "es have " : "is has ");
    string sTar = ( (sizeTar > 1) ? "s have " : " has ");
    string sObj = ( (sizeObj > 1) ? "s have " : " has ");

    if (configuration.logLevel >= LogLevel::DEBUG) {
        if (not validMat) cout << "The " << sizeMat << " material"     << sMat << "NOT been all written." << endl;
        if (not validEle) cout << "The " << sizeEle << " elementSet"   << sEle << "NOT been all written." << endl;
        if (not validLoa) cout << "The " << sizeLoa << " loading"      << sLoa << "NOT been all written." << endl;
        if (not validLos) cout << "The " << sizeLos << " loadSet"      << sLos << "NOT been all written." << endl;
        if (not validCon) cout << "The " << sizeCon << " constraint"   << sCon << "NOT been all written." << endl;
        if (not validCos) cout << "The " << sizeCos << " constraintSet"<< sCos << "NOT been all written." << endl;
        if (not validAna) cout << "The " << sizeAna << " analys"      << sAna << "NOT been all written." << endl;
        if (not validTar) cout << "The " << sizeTar << " target"      << sTar << "NOT been all written." << endl;
        if (not validObj) cout << "The " << sizeObj << " objective"      << sObj << "NOT been all written." << endl;
    }

    if (configuration.logLevel >= LogLevel::TRACE) {
        if (validMat) cout << "The " << sizeMat << " material"     << sMat << "been written." << endl;
        if (validEle) cout << "The " << sizeEle << " elementSet"   << sEle << "been written." << endl;
        if (validLoa) cout << "The " << sizeLoa << " loading"      << sLoa << "been written." << endl;
        if (validLos) cout << "The " << sizeLos << " loadSet"      << sLos << "been written." << endl;
        if (validCon) cout << "The " << sizeCon << " constraint"   << sCon << "been written." << endl;
        if (validCos) cout << "The " << sizeCos << " constraintSet"<< sCos << "been written." << endl;
        if (validAna) cout << "The " << sizeAna << " analys"      << sAna << "been written." << endl;
        if (validTar) cout << "The " << sizeTar << " target"      << sTar << "been written." << endl;
        if (validObj) cout << "The " << sizeObj << " objective"      << sObj << "been written." << endl;
    }
    bool allValid = validMat && validEle && validLoa && validLos && validCon
            && validCos && validAna && validTar && validObj;
    return allValid;
}

void Model::assignVirtualMaterial() {
    for (const auto& element : elementSets.filter(ElementSet::Type::STRUCTURAL_SEGMENT)) {
        const auto& cellElementSet = static_pointer_cast<CellElementSet>(element);
        cellElementSet->assignMaterial(getVirtualMaterial());
    }
    for (const auto& element : elementSets.filter(ElementSet::Type::NODAL_MASS)) {
        const auto& cellElementSet = static_pointer_cast<CellElementSet>(element);
        cellElementSet->assignMaterial(getVirtualMaterial());
    }
}

void Model::assignElementsToCells() {
    for (const auto& element : elementSets) {
        for (int cellPosition : element->cellPositions()) {
            mesh.assignElementId(cellPosition, element->getId());
        }
    }
}

} /* namespace vega */
