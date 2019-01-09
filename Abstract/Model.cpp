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
#include <boost/geometry/geometries/geometries.hpp>
#include <ciso646>

using namespace std;

namespace vega {

Model::Model(string name, string inputSolverVersion, SolverName inputSolver,
        const ModelConfiguration configuration, const vega::ConfigurationParameters::TranslationMode translationMode) :
        name(name), inputSolverVersion(inputSolverVersion), //
        inputSolver(inputSolver), //
        modelType(ModelType::TRIDIMENSIONAL_SI), //
        configuration(configuration), translationMode(translationMode), //
        commonLoadSet(*this, LoadSet::Type::ALL, LoadSet::COMMON_SET_ID), //
		commonConstraintSet(*this, ConstraintSet::Type::ALL, ConstraintSet::COMMON_SET_ID) {
    this->mesh = make_shared<Mesh>(configuration.logLevel, name);
    this->finished = false;
    this->onlyMesh = false;
}

Model::~Model() {
}

template<class T>
void Model::Container<T>::add(const T& t) {
    shared_ptr<T> ptr = t.clone();
    add(ptr);
}

template<class T>
void Model::Container<T>::add(shared_ptr<T> ptr) {
    if (find(ptr->getReference())) {
        ostringstream oss;
        oss << *ptr << " is already in the model";
        throw runtime_error(oss.str());
    }
    by_id[ptr->getId()] = ptr;
    if (ptr->isOriginal())
        by_original_ids_by_type[ptr->type][ptr->getOriginalId()] = ptr;
}

template<class T>
void Model::Container<T>::erase(const Reference<T> ref) {
    by_id.erase(ref.id);
    if (ref.has_original_id())
        by_original_ids_by_type[ref.type].erase(ref.original_id);
}

template<class T>
const vector<shared_ptr<T>> Model::Container<T>::filter(const typename T::Type type) const {
    vector<shared_ptr<T>> result;
    for (const auto& id_obj_pair: by_id) {
        if (id_obj_pair.second->type == type) {
            result.push_back(id_obj_pair.second);
        }
    }
    return result;
}

//template<class T>
//bool Model::Container<T>::contains(const typename T::Type type) const {
//    for (const auto& id_obj_pair: by_id) {
//        if (id_obj_pair.second->type == type) {
//            return true;
//        }
//    }
//    return false;
//}


/*
 * Redefining method add for Value to take into account placeHolder
 */
template<>
void Model::Container<NamedValue>::add(const NamedValue& t) {
    shared_ptr<NamedValue> ptr = t.clone();
    if (shared_ptr<NamedValue> ptr_old = find(t)) {
        if (ptr->isPlaceHolder()) { // TODO : make a merge function for placeHolder
            shared_ptr<Function> funPtr = dynamic_pointer_cast<Function>(ptr);
            shared_ptr<Function> funptr_old = dynamic_pointer_cast<Function>(ptr_old);
            if (funPtr->hasParaX())
                funptr_old->setParaX(funPtr->getParaX());
            if (funPtr->hasParaY())
                funptr_old->setParaY(funPtr->getParaY());
            ptr = ptr_old;
        } else if (ptr_old->isPlaceHolder()) {
            shared_ptr<Function> funPtr = dynamic_pointer_cast<Function>(ptr);
            shared_ptr<Function> funptr_old = dynamic_pointer_cast<Function>(ptr_old);
            if (funptr_old->hasParaX())
                funPtr->setParaX(funptr_old->getParaX());
            if (funptr_old->hasParaY())
                funPtr->setParaY(funptr_old->getParaY());
        } else {
            ostringstream oss;
            oss << t << " is already in the model";
            throw runtime_error(oss.str());
        }
    }
    if (!t.isPlaceHolder()) {
        by_id[t.getId()] = ptr;
    }
    if (t.isOriginal()) {
        by_original_ids_by_type[t.type][t.getOriginalId()] = ptr;
    }
}

template<class T>
shared_ptr<T> Model::Container<T>::find(const Reference<T>& reference) const {
    shared_ptr<T> t;
    if (reference.has_original_id()) {
        auto it = by_original_ids_by_type.find(reference.type);
        if (it != by_original_ids_by_type.end()) {
            auto it2 = it->second.find(reference.original_id);
            if (it2 != it->second.end()) {
                t = it2->second;
            }
        }
    } else if (reference.has_id()) {
        auto it = by_id.find(reference.id);
        if (it != by_id.end()) {
            t = it->second;
        }
    } else {
        throw logic_error("Reference is not valid:" + to_str(reference));
    }

    return t;
}

template<class T>
shared_ptr<T> Model::Container<T>::find(int original_id) const {
    shared_ptr<T> t;
    for (auto it = by_original_ids_by_type.begin(); it != by_original_ids_by_type.end(); it++) {
        auto it2 = it->second.find(original_id);
        if (it2 != it->second.end()) {
            t = it2->second;
        }
    }
    return t;
}

template<class T>
shared_ptr<T> Model::Container<T>::get(int id) const {
    shared_ptr<T> t = nullptr;
    auto it = by_id.find(id);
    if (it != by_id.end()) {
    	t = it->second;
    }
    return t;
}



void Model::add(const Analysis& analysis) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << analysis << endl;
    }
    analyses.add(analysis);
}

void Model::add(const Loading& loading) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << loading << endl;
    }
    loadings.add(loading);
}

void Model::add(const LoadSet& loadSet) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << loadSet << endl;
    }
    loadSets.add(loadSet);
}

// This "add" function used shared_ptr because adding an object
// clone it, which does not keep the original id.
//TODO: same type of "add" than the other ?
void Model::add(const shared_ptr<Material> material) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << *material << endl;
    }
    materials.add(material);
}

void Model::add(const Constraint& constraint) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << constraint << endl;
    }
    constraints.add(constraint);
}

void Model::add(const ConstraintSet& constraintSet) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << constraintSet << endl;
    }
    constraintSets.add(constraintSet);
}

void Model::add(const Objective& objective) {
    if (configuration.logLevel >= LogLevel::TRACE) {
        cout << "Adding " << objective << endl;
    }
    objectives.add(objective);
}

void Model::add(const NamedValue& value) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << value << endl;
    }
    values.add(value);
}

void Model::add(const ElementSet& elementSet) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << elementSet << endl;
    }
    elementSets.add(elementSet);
}

void Model::add(const Target& target) {
    if (configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Adding " << target << endl;
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
	for (auto material : materials){
		v.push_back(material->getId());
	}
	return v;
}

const vector<int> Model::getElementSetsId() const{
	vector<int> v;
	for (auto elementSet : elementSets){
		v.push_back(elementSet->getId());
	}
	return v;
}

template<>
void Model::remove(const Reference<Constraint> constraintReference) {
    for (auto it : constraintReferences_by_constraintSet_ids) {
        for (auto it2 : it.second) {
            if (*it2 == constraintReference) {
                constraintReferences_by_constraintSet_ids[it.first].erase(it2);
                break; // iterator is invalid now
            }
        }
    }
    for (auto it : constraintReferences_by_constraintSet_original_ids_by_constraintSet_type) {
        for (auto it2 : it.second) {
            for (auto it3 : it2.second) {
                if (*it3 == constraintReference) {
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
    for (auto it2 : cR) {
        if (*it2 == refC){
            constraintReferences_by_constraintSet_ids[idCS].erase(it2);
            break; // iterator is invalid now
        }
    }
    if (originalIdCS!= Identifiable<ConstraintSet>::NO_ORIGINAL_ID){
        const auto & cR2 = constraintReferences_by_constraintSet_original_ids_by_constraintSet_type[csT][originalIdCS];
        for (auto it3 : cR2) {
            if (*it3 == refC){
                constraintReferences_by_constraintSet_original_ids_by_constraintSet_type[csT][originalIdCS].erase(it3);
                break; // iterator is invalid now
            }
        }
    }
    constraints.erase(refC);
}

template<>
void Model::remove(const Reference<Loading> loadingReference) {
    for (auto it : loadingReferences_by_loadSet_ids) {
        for (auto it2 : it.second) {
            if (*it2 == loadingReference) {
                loadingReferences_by_loadSet_ids[it.first].erase(it2);
                break; // iterator is invalid now
            }
        }
    }
    for (auto it : loadingReferences_by_loadSet_original_ids_by_loadSet_type) {
        for (auto it2 : it.second) {
            for (auto it3 : it2.second) {
                if (*it3 == loadingReference) {
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
    for (auto analysis : analyses) {
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
    for (auto analysis : analyses) {
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
        for (auto analysis : analyses) {
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
    if (reference.type == Analysis::Type::UNKNOWN && reference.id == Reference<Analysis>::NO_ID)
        // retrieve by original_id
        analysis = analyses.find(reference.original_id);
    else
        analysis = analyses.find(reference);
    return analysis;
}

template<>
const shared_ptr<ElementSet> Model::find(const Reference<ElementSet> reference) const {
    shared_ptr<ElementSet> elementSet;
    if (reference.type == ElementSet::Type::UNKNOWN && reference.id == Reference<ElementSet>::NO_ID)
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
    shared_ptr<Reference<Loading>> loadingReference_ptr = loadingReference.clone();
    if (loadSetReference.has_id())
        loadingReferences_by_loadSet_ids[loadSetReference.id].insert(loadingReference_ptr);
    if (loadSetReference.has_original_id())
        loadingReferences_by_loadSet_original_ids_by_loadSet_type[loadSetReference.type][loadSetReference.original_id].insert(
                loadingReference_ptr);
    if (loadSetReference == commonLoadSet.getReference() && !find(commonLoadSet.getReference()))
        add(commonLoadSet); // commonLoadSet is added to the model if needed
    if (!this->find(loadSetReference)) {
        LoadSet loadSet(*this, loadSetReference.type, loadSetReference.original_id);
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
        for (auto itm2 : itm->second) {
            shared_ptr<Loading> loading = find(*itm2);
            assert(loading != nullptr);
            result.insert(loading);
        }
    }
    auto itm2 = loadingReferences_by_loadSet_original_ids_by_loadSet_type.find(
            loadSetReference.type);
    if (itm2 != loadingReferences_by_loadSet_original_ids_by_loadSet_type.end()) {
        auto itm3 = itm2->second.find(loadSetReference.original_id);
        if (itm3 != itm2->second.end()) {
            for (auto itm4 : itm3->second) {
                shared_ptr<Loading> loading = find(*itm4);
                assert(loading != nullptr);
                result.insert(loading);
            }
        }
    }
    return result;
}

void Model::addConstraintIntoConstraintSet(const Reference<Constraint>& constraintReference,
        const Reference<ConstraintSet>& constraintSetReference) {
    shared_ptr<Reference<Constraint>> constraintReference_ptr = constraintReference.clone();
    if (constraintSetReference.has_id())
        constraintReferences_by_constraintSet_ids[constraintSetReference.id].insert(
                constraintReference_ptr);
    if (constraintSetReference.has_original_id())
        constraintReferences_by_constraintSet_original_ids_by_constraintSet_type[constraintSetReference.type][constraintSetReference.original_id].insert(
                constraintReference_ptr);
    if (constraintSetReference == commonConstraintSet.getReference()
            && !find(commonConstraintSet.getReference()))
        add(commonConstraintSet); // commonConstraintSet is added to the model if needed
}

const set<shared_ptr<Constraint>> Model::getConstraintsByConstraintSet(
        const Reference<ConstraintSet>& constraintSetReference) const {
    set<shared_ptr<Constraint>> result;
    auto itm = constraintReferences_by_constraintSet_ids.find(constraintSetReference.id);
    if (itm != constraintReferences_by_constraintSet_ids.end()) {
        for (auto itm2 : itm->second) {
            result.insert(find(*itm2));
        }
    }
    auto itm2 = constraintReferences_by_constraintSet_original_ids_by_constraintSet_type.find(
            constraintSetReference.type);
    if (itm2 != constraintReferences_by_constraintSet_original_ids_by_constraintSet_type.end()) {
        auto itm3 = itm2->second.find(constraintSetReference.original_id);
        if (itm3 != itm2->second.end()) {
            for (auto itm4 : itm3->second) {
                result.insert(find(*itm4));
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
        for (auto& constraint : constraints) {
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
    for (auto analyse : analyses) {
        for (auto constraintSet : analyse->getConstraintSets()) {
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
    for (auto analyse : analyses) {
        for (auto loadSet : analyse->getLoadSets()) {
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
    for (auto analyse : analyses) {
        for (auto constraintSet : analyse->getConstraintSets()) {
            map[constraintSet] += 1;
        }
    }
    for (auto constraintSet : constraintSets) {
        auto it = map.find(constraintSet);
        if (it != map.end() && it->second == analyses.size())
            result.push_back(constraintSet);
    }
    return result;
}

const vector<shared_ptr<LoadSet>> Model::getCommonLoadSets() const {
    vector<shared_ptr<LoadSet>> result;
    map<shared_ptr<LoadSet>, int> map;
    for (auto analyse : analyses) {
        for (auto loadSet : analyse->getLoadSets()) {
            map[loadSet] += 1;
        }
    }
    for (auto loadSet : loadSets) {
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
    for (auto analyse : analyses) {
        for (auto constraintSet : analyse->getConstraintSets()) {
            map[constraintSet] += 1;
        }
    }
    for (auto constraintSet : constraintSets) {
        auto it = map.find(constraintSet);
        if (it != map.end() && it->second < analyses.size())
            result.insert(constraintSet);
    }
    return result;
}

const set<shared_ptr<LoadSet>> Model::getUncommonLoadSets() const {
    set<shared_ptr<LoadSet>> result;
    map<shared_ptr<LoadSet>, int> map;
    for (auto analyse : analyses) {
        for (auto loadSet : analyse->getLoadSets()) {
            map[loadSet] += 1;
        }
    }
    for (auto loadSet : loadSets) {
        auto it = map.find(loadSet);
        if ((it != map.end() && it->second < analyses.size()) && loadSet->type != LoadSet::Type::DLOAD)
            result.insert(loadSet);
    }
    return result;
}

void Model::generateDiscrets() {

    //rigid constraints
    shared_ptr<CellGroup> virtualDiscretTRGroup = nullptr;

    shared_ptr<CellGroup> virtualDiscretTGroup = nullptr;

    for (const Node& node : this->mesh->nodes) {
        DOFS missingDOFS;

        for (auto& analysis : analyses) {

            const DOFS& requiredDOFS = analysis->findBoundaryDOFS(node.position);
            if (!node.dofs.containsAll(requiredDOFS)) {
                missingDOFS = missingDOFS + requiredDOFS - node.dofs;
            }
        }

        DOFS addedDOFS;
        if (missingDOFS.size() != 0) {
            if (missingDOFS.containsAnyOf(DOFS::ROTATIONS)) {
                //extra dofs added by the DISCRET. They need to be blocked.
                addedDOFS = DOFS::ALL_DOFS - node.dofs - missingDOFS;
                if (virtualDiscretTRGroup == nullptr) {
                    DiscretePoint virtualDiscretTR(*this, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
                    ostringstream oss;
                    oss << "created by generateDiscrets() because missing DOFs: " << missingDOFS << " and node: " << node;
                    virtualDiscretTRGroup = mesh->createCellGroup("VDiscrTR", Group::NO_ORIGINAL_ID, oss.str());
                    virtualDiscretTR.assignCellGroup(virtualDiscretTRGroup);
                    if (this->configuration.addVirtualMaterial) {
                        virtualDiscretTR.assignMaterial(getVirtualMaterial());
                    }
                    this->add(virtualDiscretTR);
                }
                vector<int> cellNodes;
                cellNodes.push_back(node.id);
                mesh->allowDOFS(node.position, DOFS::ALL_DOFS);
                int cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::POINT1, cellNodes,
                        true);
                virtualDiscretTRGroup->addCellPosition(cellPosition);
            } else {
                addedDOFS = DOFS::TRANSLATIONS - node.dofs - missingDOFS;
                if (virtualDiscretTGroup == nullptr) {
                    DiscretePoint virtualDiscretT(*this, 0.0, 0.0, 0.0);
                    ostringstream oss;
                    oss << "created by generateDiscrets() because missing DOFs: " << missingDOFS << " and node: " << node;
                    virtualDiscretTGroup = mesh->createCellGroup("VDiscrT", Group::NO_ORIGINAL_ID, oss.str());
                    virtualDiscretT.assignCellGroup(virtualDiscretTGroup);
                    if (this->configuration.addVirtualMaterial) {
                        virtualDiscretT.assignMaterial(getVirtualMaterial());
                    }
                    this->add(virtualDiscretT);
                }
                int cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::POINT1, { node.id },
                        true);
                virtualDiscretTGroup->addCellPosition(cellPosition);
                mesh->allowDOFS(node.position, DOFS::TRANSLATIONS);
            }
        }

        for (auto& analysis : analyses) {
            ConstraintSet* spcSet = nullptr;

            const DOFS& requiredDOFS = analysis->findBoundaryDOFS(node.position);
            if (!node.dofs.containsAll(requiredDOFS)) {
                const DOFS& extraDOFS = addedDOFS - requiredDOFS - node.dofs;

                if (extraDOFS != DOFS::NO_DOFS) {
                    if (spcSet == nullptr) {
                        spcSet = new ConstraintSet(*this, ConstraintSet::Type::SPC);
                        add(*spcSet);
                    }
                    SinglePointConstraint spc = SinglePointConstraint(*this, extraDOFS);
                    spc.addNodeId(node.id);
                    add(spc);
                    addConstraintIntoConstraintSet(spc, *spcSet);
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
    if (!result && createIfNotExists) {
        result = make_shared<Material>(this, material_id);
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
        return CellContainer(*(this->mesh));
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
    if (!virtualMaterial) {
        virtualMaterial = this->getOrCreateMaterial(Material::NO_ORIGINAL_ID);
        virtualMaterial->addNature(ElasticNature(*this, 1e-12, 0.0));
    }
    return virtualMaterial;
}

const vector<shared_ptr<Beam>> Model::getBeams() const {
    vector<shared_ptr<Beam>> result;
    for (auto elementSet : elementSets) {
        if (elementSet->isBeam()) {
            shared_ptr<Beam> beam = dynamic_pointer_cast<Beam>(elementSet);
            result.push_back(beam);
        }
    }
    return result;
}


void Model::generateSkin() {
    ostringstream oss;
    oss << "created by generateSkin() because of FORCE_SURFACE or BOUNDARY_ELEMENTFACE or ...";
    shared_ptr<CellGroup> mappl = mesh->createCellGroup("VEGASKIN", Group::NO_ORIGINAL_ID, oss.str());

    bool addedSkin = false;
    for (auto it = loadings.begin(); it != loadings.end(); it++) {
        shared_ptr<Loading> loadingPtr = *it;
        if (loadingPtr->applicationType == Loading::ApplicationType::ELEMENT) {
            shared_ptr<ElementLoading> elementLoading = dynamic_pointer_cast<ElementLoading>(
                    loadingPtr);
            if (elementLoading->cellDimensionGreatherThan(elementLoading->getLoadingDimension())) {
                switch (loadingPtr->type) {
                case Loading::Type::FORCE_SURFACE: {
                    shared_ptr<ForceSurface> forceSurface = dynamic_pointer_cast<ForceSurface>(
                            loadingPtr);
                    vector<int> faceIds = forceSurface->getApplicationFace();
                    if (faceIds.size() > 0) {
                        addedSkin = true;
                        vega::Cell cell = generateSkinCell(faceIds, SpaceDimension::DIMENSION_2D);
                        mappl->addCellPosition(cell.position);
                        forceSurface->clear(); //< To remove the volumic cell and then add the skin at its place
                        //forceSurface->addCellId(cell.id);
                        // LD Workaround for problem "cannot write cell names"
                        shared_ptr<CellGroup> cellGrp = mesh->createCellGroup(Cell::MedName(cell.position), Group::NO_ORIGINAL_ID, "Single cell group over skin element");
                        cellGrp->addCellPosition(cell.position);
                        forceSurface->add(*cellGrp);
                        //forceSurface->add(*mappl);
                    }
                    break;
                }
                case Loading::Type::NORMAL_PRESSION_FACE: {
                    shared_ptr<NormalPressionFace> normalPressionFace = dynamic_pointer_cast<NormalPressionFace>(
                            loadingPtr);
                    vector<int> faceIds = normalPressionFace->getApplicationFace();
                    if (faceIds.size() > 0) {
                        addedSkin = true;
                        vega::Cell cell = generateSkinCell(faceIds, SpaceDimension::DIMENSION_2D);
                        mappl->addCellPosition(cell.position);
                        normalPressionFace->clear(); //< To remove the volumic cell and then add the skin at its place
                        //forceSurface->addCellId(cell.id);
                        // LD Workaround for problem "cannot write cell names"
                        shared_ptr<CellGroup> cellGrp = mesh->createCellGroup(Cell::MedName(cell.position), Group::NO_ORIGINAL_ID, "Single cell group over skin element");
                        cellGrp->addCellPosition(cell.position);
                        normalPressionFace->add(*cellGrp);
                        //forceSurface->add(*mappl);
                    }
                    break;
                }
                default:
                    throw logic_error("generate skin implemented only for pression face");
                }
            }
        }
    }

    for (auto it = targets.begin(); it != targets.end(); it++) {
        shared_ptr<Target> target = *it;
        if (target->type == Target::Type::BOUNDARY_ELEMENTFACE) {
            shared_ptr<BoundaryElementFace> elementFace = dynamic_pointer_cast<BoundaryElementFace>(
                    target);
            ostringstream oss2;
            oss2 << "created by generateSkin() because of BOUNDARY_ELEMENTFACE";
            shared_ptr<CellGroup> surfGrp = this->mesh->createCellGroup("SURF" + to_string(elementFace->bestId()), Group::NO_ORIGINAL_ID, oss2.str());
            shared_ptr<CellGroup> elemGrp = this->mesh->createCellGroup("SURFO" + to_string(elementFace->bestId()), Group::NO_ORIGINAL_ID, "BOUNDARY ELEMENTFACE");
            for(auto& faceInfo: elementFace->faceInfos) {
                elemGrp->addCellId(faceInfo.cellId);
                elementFace->elementCellGroup = elemGrp;
                const Cell& cell0 = this->mesh->findCell(this->mesh->findCellPosition(faceInfo.cellId));
                const vector<int>& faceIds = cell0.faceids_from_two_nodes(faceInfo.nodeid1, faceInfo.nodeid2);
                if (faceIds.size() > 0) {
                    addedSkin = true;
                    vega::Cell cell = generateSkinCell(faceIds, SpaceDimension::DIMENSION_2D);
                    mappl->addCellPosition(cell.position);
                    surfGrp->addCellPosition(cell.position);
                    elementFace->surfaceCellGroup = surfGrp;
                }
            }
        }
    }

    if (addedSkin) {
        // LD : Workaround for Aster problem : MODELISA6_96
        //  les 1 mailles imprimées ci-dessus n'appartiennent pas au modèle et pourtant elles ont été affectées dans le mot-clé facteur : !
        //   ! FORCE_FACE
        Continuum skin(*this, this->modelType);
        skin.assignCellGroup(mappl);
        // LD: Workaround for Aster problem : COMPOR1_60
        // Toutes les mailles déclarées dans l'occurrence numéro 6 du mot-clé COMPORTEMENT
        // sont des éléments de bord qui ne portent pas de rigidité.
        // Il faut supprimer cette occurrence pour que le calcul fonctionne.
        //skin.assignMaterial(getVirtualMaterial());
        this->add(skin);
    }

}

Cell Model::generateSkinCell(const vector<int>& faceIds, const SpaceDimension& dimension) {
    CellType* cellTypeFound = nullptr;
    for (auto typeAndCodePair : CellType::typeByCode) {
        CellType * typeToTest = typeAndCodePair.second;
        if (typeToTest->dimension == dimension && faceIds.size() == typeToTest->numNodes) {
            cellTypeFound = typeToTest;
            break;
        }
    }
    vector<int> externalFaceIds;
    if (cellTypeFound == nullptr) {
        throw logic_error(
                "CellType not found connections:"
                        + to_string(faceIds.size()));
    }
    int cellPosition = mesh->addCell(Cell::AUTO_ID, *cellTypeFound, faceIds, true);
    return mesh->findCell(cellPosition);
}

void Model::emulateLocalDisplacementConstraint() {
    unordered_map<shared_ptr<Constraint>, set<LinearMultiplePointConstraint*>> linearMultiplePointConstraintsByConstraint;
    // first pass : create LinearMultiplePoint constraints for each constraint that need it
    for (auto it = constraints.begin(); it != constraints.end(); it++) {
        const shared_ptr<Constraint>& constraint = *it;
        if (constraint->type == Constraint::Type::SPC) {
            shared_ptr<SinglePointConstraint> spc = dynamic_pointer_cast<SinglePointConstraint>(
                    constraint);
            for (int nodePosition : spc->nodePositions()) {
                const Node& node = mesh->findNode(nodePosition);
                if (node.displacementCS != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
                    shared_ptr<CoordinateSystem> coordSystem = mesh->getCoordinateSystemByPosition(node.displacementCS);
                    // TODO LD: this should be done differently: is used to compute different nodes but every this it changes the coordinate system instance!
                    coordSystem->updateLocalBase(VectorialValue(node.x, node.y, node.z));
                    DOFS dofs = constraint->getDOFSForNode(nodePosition);
                    if (configuration.logLevel >= LogLevel::DEBUG)
                        cout << "Replacing local spc" << *spc << "node:" << node << ",dofs " << constraint->getDOFSForNode(nodePosition) << endl;
                    for (int i = 0; i < 6; i++) {
                        vega::DOF currentDOF = *DOF::dofByPosition[i];
                        if (dofs.contains(currentDOF)) {
                            VectorialValue participation = coordSystem->vectorToGlobal(
                                    VectorialValue::XYZ[i % 3]);
                            LinearMultiplePointConstraint* lmpc =
                                    new LinearMultiplePointConstraint(*this,
                                            spc->getDoubleForDOF(currentDOF));
                            if (i < 3) {
                                lmpc->addParticipation(node.id, participation.x(),
                                        participation.y(), participation.z());
                            } else {
                                lmpc->addParticipation(node.id, 0, 0, 0, participation.x(),
                                        participation.y(), participation.z());
                            }
                            if (configuration.logLevel >= LogLevel::DEBUG)
                                cout << "Node:" << node << ", current dof:" << currentDOF << ", participation:" << participation << ", coef:" << lmpc->coef_impo << endl;
                            linearMultiplePointConstraintsByConstraint[constraint].insert(lmpc);
                        }
                    }
                    constraint->removeNode(nodePosition);
                }
            }
        }
    }

    // second pass : insert the new LinearMultiplePointConstraints into the model and the constraintSets
    for (auto it : linearMultiplePointConstraintsByConstraint) {
        shared_ptr<Constraint> constraint = it.first;
        const set<shared_ptr<ConstraintSet>> constraintSets = getConstraintSetsByConstraint(
                constraint->getReference());
        for (auto linearMultiplePointConstraint : it.second) {
            add(*linearMultiplePointConstraint);
            for (auto constraintSet : constraintSets) {
                addConstraintIntoConstraintSet(linearMultiplePointConstraint->getReference(),
                        constraintSet->getReference());
            }
            delete linearMultiplePointConstraint;
        }
        if (constraint->nodePositions().size() == 0)
            remove(constraint->getReference());
    }
}

void Model::emulateAdditionalMass() {
    vector<shared_ptr<ElementSet>> newElementSets;
    for (auto elementSet : elementSets) {
        double rho = elementSet->getAdditionalRho();
        if (!is_zero(rho)) {
            // create new elementSet
            shared_ptr<ElementSet> newElementSet = elementSet->clone();
            newElementSet->resetId();
            newElementSets.push_back(newElementSet);
            // assign new material
            shared_ptr<Material> newMaterial = make_shared<Material>(this);
            newMaterial->addNature(ElasticNature(*this, 0, 0, 0, rho));
            materials.add(newMaterial);
            newElementSet->assignMaterial(newMaterial);
            // copy and assign new cellGroup
            ostringstream oss;
            oss << "created by emulateAdditionalMass() because of elementSet: " << elementSet << " additional rho:" << rho;
            shared_ptr<CellGroup> newCellGroup = mesh->createCellGroup(
                    "VAM_" + to_string(newElementSets.size()), Group::NO_ORIGINAL_ID, oss.str());
            newElementSet->assignCellGroup(newCellGroup);
            vector<Cell> cells = elementSet->cellGroup->getCells();
            for (auto cell : cells) {
                int cellPosition = mesh->addCell(Cell::AUTO_ID, cell.type, cell.nodeIds, cell.isvirtual,
                        cell.cspos, cell.elementId);
                newCellGroup->addCellPosition(cellPosition);
            }
        }
    }
    for (auto elementSet : newElementSets)
        elementSets.add(elementSet);
}

void Model::generateBeamsToDisplayHomogeneousConstraint() {

    shared_ptr<CellGroup> virtualGroupRigid = nullptr;
    shared_ptr<CellGroup> virtualGroupRBE3 = nullptr;

    vector<shared_ptr<ConstraintSet>> activeConstraintSets = getActiveConstraintSets();
    for (auto constraintSet : activeConstraintSets) {
        set<shared_ptr<Constraint>> constraints = constraintSet->getConstraints();
        for (auto constraint : constraints) {
            switch (constraint->type) {
            case Constraint::Type::RIGID: {
                if (!virtualGroupRigid) {
                    CircularSectionBeam virtualBeam(*this, 0.001, Beam::BeamModel::EULER, 0.0);
                    if (this->configuration.addVirtualMaterial) {
                        virtualBeam.assignMaterial(getVirtualMaterial());
                    }
                    ostringstream oss;
                    oss << "created by generateBeamsToDisplayHomogeneousConstraint() because of rigid constraint: " << constraint;
                    virtualGroupRigid = mesh->createCellGroup("VRigid", Group::NO_ORIGINAL_ID, oss.str());
                    virtualBeam.assignCellGroup(virtualGroupRigid);
                    this->add(virtualBeam);
                }
                shared_ptr<RigidConstraint> rigid = dynamic_pointer_cast<RigidConstraint>(
                        constraint);
                vector<int> nodes = { 0, 0 };
                nodes[0] = mesh->findNodeId(rigid->getMaster());
                mesh->allowDOFS(rigid->getMaster(), DOFS::ALL_DOFS);
                for (int slaveNode : rigid->getSlaves()) {
                    nodes[1] = mesh->findNodeId(slaveNode);
                    int cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG2, nodes, true);
                    virtualGroupRigid->addCellPosition(cellPosition);
                    mesh->allowDOFS(slaveNode, DOFS::ALL_DOFS);
                }
                break;
            }
            case Constraint::Type::RBE3: {
                if (!virtualGroupRBE3) {
                    CircularSectionBeam virtualBeam(*this, 0.001, Beam::BeamModel::EULER, 0.0);
                    if (configuration.addVirtualMaterial) {
                        virtualBeam.assignMaterial(getVirtualMaterial());
                    }
                    ostringstream oss;
                    oss << "created by generateBeamsToDisplayHomogeneousConstraint() because of rbe3 constraint: " << constraint;
                    virtualGroupRBE3 = mesh->createCellGroup("VRBE3", Group::NO_ORIGINAL_ID, oss.str());
                    virtualBeam.assignCellGroup(virtualGroupRBE3);
                    this->add(virtualBeam);
                }
                shared_ptr<RBE3> rbe3 = dynamic_pointer_cast<RBE3>(constraint);
                vector<int> nodes = { 0, 0 };
                nodes[0] = mesh->findNodeId(rbe3->getMaster());
                mesh->allowDOFS(rbe3->getMaster(), DOFS::ALL_DOFS);
                for (int slaveNode : rbe3->getSlaves()) {
                    nodes[1] = mesh->findNodeId(slaveNode);
                    int cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG2, nodes, true);
                    mesh->allowDOFS(slaveNode, DOFS::ALL_DOFS);
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
                        CellContainer assignment(*(this->mesh));
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
    for (auto loading : loadings) {
        if (loading->ineffective())
            loadingsToRemove.push_back(loading);
    }
    for (auto loading : loadingsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG)
            cout << "Removed ineffective " << *loading << endl;
        remove(Reference<Loading>(*loading));
    }

    // remove empty loadSets from the model
    vector<Reference<LoadSet>> loadSetSetsToRemove;
    for (auto loadSet : this->loadSets) {
        if (loadSet->size() == 0) {
            loadSetSetsToRemove.push_back(loadSet->getReference());
        }
    }
    for (Reference<LoadSet> loadSetRef : loadSetSetsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG)
            cout << "Removed empty loadset " << loadSetRef.id << endl;
        remove(loadSetRef);
    }

    // remove ineffective constraints from the model
    vector<shared_ptr<Constraint>> constraintsToRemove;
    for (auto constraint : constraints) {
        if (constraint->ineffective())
            constraintsToRemove.push_back(constraint);
    }
    for (auto constraint : constraintsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG)
            cout << "Removed ineffective " << *constraint << endl;
        remove(Reference<Constraint>(*constraint));
    }

    // remove empty constraintSets from the model
    vector<Reference<ConstraintSet>> constraintSetSetsToRemove;
    for (auto constraintSet : this->constraintSets) {
        if (constraintSet->size() == 0) {
            constraintSetSetsToRemove.push_back(constraintSet->getReference());
        }
    }
    for (Reference<ConstraintSet> constraintSetRef : constraintSetSetsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG)
            cout << "Removed empty " << constraintSetRef.id << endl;
        remove(constraintSetRef);
    }

    // remove empty elementSets from the model
    vector<shared_ptr<ElementSet>> elementSetsToRemove;
    for (auto elementSet : elementSets) {
        if (elementSet->cellGroup and elementSet->cellGroup->empty())
            elementSetsToRemove.push_back(elementSet);
    }
    for (auto elementSet : elementSetsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG)
            cout << "Removed empty " << *elementSet << endl;
        this->elementSets.erase(Reference<ElementSet>(*elementSet));
    }
}

void Model::removeUnassignedMaterials() {
    vector<shared_ptr<Material>> materialsToRemove;
    //for (auto& material : materials) {
        /*
         * if the model is configured to assign materials to cells directly check
         * that for every material at lest a cell or cellgroup is assigned
         */
        //if (!configuration.partitionModel and material->getAssignment().empty())
        //        materialsToRemove.push_back(material);
    //}
    for (auto& material : materialsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG)
            cout << "Removed unassigned " << *material << endl;
        this->materials.erase(Reference<Material>(*material));
    }
}

void Model::replaceCombinedLoadSets() {
    for (auto loadSet : this->loadSets) {
        for (auto& kv : loadSet->embedded_loadsets) {
            shared_ptr<LoadSet> otherloadSet = this->find(kv.first);
            if (!otherloadSet) {
                cerr << "CombinedLoadSet: missing loadSet " << to_string(kv.first.id) << endl;
            }
            double coefficient = kv.second;
            for (shared_ptr<Loading> loading : otherloadSet->getLoadings()) {
                shared_ptr<Loading> newLoading = loading->clone();
                newLoading->resetId();
                newLoading->scale(coefficient);
                this->add(*newLoading);
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
    for (auto& analysis : analyses) {
        for(auto& assertion : analysis->getAssertions()) {
            for(int nodePosition: assertion->nodePositions()) {
                const DOFS& assertionDOFS = assertion->getDOFSForNode(nodePosition);
                if (assertionDOFS.size() >= 1) {
                    const Node& node = mesh->findNode(nodePosition);
                    const DOFS& availableDOFS = node.dofs + analysis->findBoundaryDOFS(nodePosition);
                    if (!availableDOFS.containsAll(assertionDOFS)) {
                        objectivesToRemove.push_back(assertion);
                    }
                }
            }
        }
    }
    for (auto objective : objectivesToRemove) {
        if (configuration.logLevel >= LogLevel::TRACE)
            cout << "Removed ineffective " << *objective << endl;

        remove(Reference<Objective>(*objective));
    }
}

void Model::addDefaultAnalysis()
{
    if (this->analyses.size() == 0 && (loadings.size() > 0 || constraints.size() > 0)) {
        //add an automatic linear analysis
        LinearMecaStat analysis(*this, "", 1);
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
    for (auto elementSetM : elementSets) {
        if (!elementSetM->isMatrixElement()) {
            continue;
        }
        shared_ptr<MatrixElement> matrix = dynamic_pointer_cast<MatrixElement>(elementSetM);
        for (int nodePosition : matrix->nodePositions()) {
            requiredDofsByNode[nodePosition] = DOFS();
            const int nodeId = mesh->findNodeId(nodePosition);
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
        for (auto pair : matrix->nodePairs()) {
            if (pair.first == pair.second) {
                if (matrix->findInPairs(pair.first).size() != 0) {
                    continue; // will be handled by a segment cell with another node
                }
                // single node
                int nodePosition = pair.first;
                const int nodeId = mesh->findNodeId(nodePosition);
                DOFS requiredDofs = requiredDofsByNode.find(nodePosition)->second;
                shared_ptr<DOFMatrix> submatrix = matrix->findSubmatrix(nodePosition, nodePosition);
                DiscretePoint discrete(*this, {});
                for (auto& kv : submatrix->componentByDofs) {
                    double value = kv.second;
                    const vega::DOF& dof1 = kv.first.first;
                    const vega::DOF& dof2 = kv.first.second;
                    if (!is_equal(value, 0)) {
                        switch (matrix->type) {
                        case ElementSet::Type::STIFFNESS_MATRIX:
                            discrete.addStiffness(dof1, dof2, value);
                            break;
                        default:
                            throw logic_error("Not yet implemented");
                        }
                        requiredDofs += dof1;
                        requiredDofs += dof2;
                    }
                }
                if (configuration.addVirtualMaterial) {
                    discrete.assignMaterial(getVirtualMaterial());
                }
                matrix_count++;
                ostringstream oss;
                oss << "created by replaceDirectMatrices() because of matrix element on same node: " << matrix;
                shared_ptr<CellGroup> matrixGroup = mesh->createCellGroup(
                        "MTN" + to_string(matrix_count), Group::NO_ORIGINAL_ID, oss.str());
                discrete.assignCellGroup(matrixGroup);
                int cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG2, { nodeId }, true);
                matrixGroup->addCellPosition(cellPosition);
                if (discrete.hasRotations()) {
                    addedDofsByNode[nodePosition] = DOFS::ALL_DOFS;
                    mesh->allowDOFS(nodePosition, DOFS::ALL_DOFS);
                } else {
                    addedDofsByNode[nodePosition] = DOFS::TRANSLATIONS;
                    mesh->allowDOFS(nodePosition, DOFS::TRANSLATIONS);
                }
                if (this->configuration.logLevel >= LogLevel::DEBUG) {
                    cout << "Creating discrete : " << discrete << " over node id : "
                            << to_string(nodeId) << endl;
                }
                this->add(discrete);
            } else {
                // node couple
                int rowNodePosition = pair.first;
                int colNodePosition = pair.second;
                const int rowNodeId = mesh->findNodeId(rowNodePosition);
                const int colNodeId = mesh->findNodeId(colNodePosition);
                DOFS requiredRowDofs = requiredDofsByNode.find(rowNodePosition)->second;
                DOFS requiredColDofs = requiredDofsByNode.find(colNodePosition)->second;

                DiscreteSegment discrete(*this);
                ostringstream oss;
                oss << "created by replaceDirectMatrices() because of matrix element on node couple: " << matrix;
                shared_ptr<CellGroup> matrixGroup = mesh->createCellGroup(
                        "MTL" + to_string(matrix_count), Group::NO_ORIGINAL_ID, oss.str());
                matrix_count++;
                int cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG2, { rowNodeId,
                        colNodeId }, true);
                matrixGroup->addCellPosition(cellPosition);
                if (configuration.addVirtualMaterial) {
                    discrete.assignMaterial(getVirtualMaterial());
                }
                discrete.assignCellGroup(matrixGroup);
                if (discrete.hasRotations()) {
                    addedDofsByNode[rowNodePosition] = DOFS::ALL_DOFS;
                    mesh->allowDOFS(rowNodePosition, DOFS::ALL_DOFS);
                    addedDofsByNode[colNodePosition] = DOFS::ALL_DOFS;
                    mesh->allowDOFS(colNodePosition, DOFS::ALL_DOFS);
                } else {
                    addedDofsByNode[rowNodePosition] = DOFS::TRANSLATIONS;
                    mesh->allowDOFS(rowNodePosition, DOFS::TRANSLATIONS);
                    addedDofsByNode[colNodePosition] = DOFS::TRANSLATIONS;
                    mesh->allowDOFS(colNodePosition, DOFS::TRANSLATIONS);
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
                        for (auto& kv : submatrix->componentByDofs) {
                            // We are disassembling the matrix, so we must divide the value by the segments
                            double value = kv.second / static_cast<int>(matrix->findInPairs(pair.first).size());
                            const DOF& rowDof = kv.first.first;
                            const DOF& colDof = kv.first.second;
                            if (!is_equal(value, 0)) {
                                switch (matrix->type) {
                                case ElementSet::Type::STIFFNESS_MATRIX:
                                    discrete.addStiffness(row_index, col_index, rowDof, colDof,
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
                    cout << "Creating discrete : " << discrete << " over node ids : "
                            << to_string(rowNodeId) << " and : " << to_string(colNodeId) << endl;
                }
                this->add(discrete);
            }
        }
        elementSetsToRemove.push_back(elementSetM);
    }
    for (auto& kv : addedDofsByNode) {
        int nodePosition = kv.first;
        const int nodeId = this->mesh->findNodeId(nodePosition);
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
            SinglePointConstraint spc = SinglePointConstraint(*this, extra);
            spc.addNodeId(nodeId);
            add(spc);
            addConstraintIntoConstraintSet(spc, commonConstraintSet);
            if (configuration.logLevel >= LogLevel::DEBUG) {
                cout << "Adding virtual spc on node id: " << nodeId << "for " << extra
                        << endl;
            }
        }
    }
    for (auto elementSet : elementSetsToRemove) {
        if (configuration.logLevel >= LogLevel::DEBUG) {
            cout << "Replaced " << *elementSet << endl;
        }
        this->elementSets.erase(elementSet->getReference());
    }
}

void Model::replaceRigidSegments()
{
    for (auto elementSet : elementSets.filter(ElementSet::Type::STRUCTURAL_SEGMENT)) {
        shared_ptr<StructuralSegment> segment = dynamic_pointer_cast<StructuralSegment>(elementSet);
        if (not segment->isDiagonalRigid()) {
            continue;
        }
        const set<int>& segmentNodePositions = segment->nodePositions();
        const vector<int>& nodePositions{segmentNodePositions.begin(), segmentNodePositions.end()};
        int masterId = mesh->findCellId(nodePositions[0]);
        //set<int> slaveIds {mesh->findCellId(nodePositions[1])};
        RigidConstraint rigid(*this, masterId, RigidConstraint::NO_ORIGINAL_ID, {mesh->findCellId(nodePositions[1])});
        this->add(rigid);
        segment->setAllZero(); // make the rigid segment... no more rigid.
    }
}

void Model::removeRedundantSpcs() {
    for (auto analysis : this->analyses) {
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
                            const int nodeId = this->mesh->findNodeId(nodePosition);
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
                            cout << "Removed redundant dofs : " << dofsToRemove << " from node id : " << this->mesh->findNodeId(nodePosition)
                                    << " from spc : " << *spc << " for analysis : " << *analysis << endl;
                        }
                    }
                }
            }
        }
    }
}

void Model::removeConstrainedImposed() {
    for (auto analysis : this->analyses) {
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
                            cout << "Removed imposed dofs : " << dofsToRemove << " from node id : " << this->mesh->findNodeId(nodePosition)
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

    for (auto elementSetM : elementSets) {
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
        for (auto & v : matrix->nodePositions()){
            nodeIdOfElement[v] = mesh->findNodeId(v);
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
            for (auto es : esToAddByStackNumber){
                cout << es.second->bestId()<<" ";
            }
            cout <<endl;
        }

    }

    // We remove the elementSets corresponding to big matrices
    for (auto es : esToErase){
        this->elementSets.erase(*es);
    }

    // We add the elementSets corresponding to small matrices
    for (auto es : esToAdd){
        this->add(*es);
    }
}



void Model::makeCellsFromDirectMatrices(){

    int idM=0;
    for (auto elementSetM : elementSets) {
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
        shared_ptr<CellGroup> matrixGroup = mesh->createCellGroup("DM" + to_string(idM), Group::NO_ORIGINAL_ID, "Direct Matrix "+ elementSetM->name);
        matrix->assignCellGroup(matrixGroup);

        // Create a Cell and add it to the Cell Group
        CellType cellType = CellType::polyType(static_cast<unsigned int>(matrix->nodePositions().size()));
        vector<int> vNodeIds;
        for (int nodePosition : matrix->nodePositions()){
            const int nodeId = mesh->findNodeId(nodePosition);
            vNodeIds.push_back(nodeId);
        }

        int cellPosition = mesh->addCell(Cell::AUTO_ID, cellType, vNodeIds, true);
        matrixGroup->addCellPosition(cellPosition);

        if (configuration.logLevel >= LogLevel::DEBUG){
           cout << "Built cells, in cellgroup "<<matrixGroup->getName()<<", for Matrix Elements in "<< elementSetM->name<<"."<<endl;
        }

    }
}


void Model::makeCellsFromLMPC(){

    shared_ptr<Mesh> mesh = this->mesh;
    shared_ptr<Material> materialLMPC= nullptr;

    for (auto analysis : this->analyses) {
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
                        materialLMPC= make_shared<Material>(this);
                        materialLMPC->addNature(RigidNature(*this, 1));
                        this->add(materialLMPC);
                    }
                    group = mesh->createCellGroup("MPC_"+to_string(analysis->bestId())+"_"+to_string(constraint->bestId()), CellGroup::NO_ORIGINAL_ID, "MPC");
                    Lmpc elementsetLMPC(*this, analysis->getId());
                    elementsetLMPC.assignCellGroup(group);
                    elementsetLMPC.assignMaterial(materialLMPC);
                    elementsetLMPC.assignDofCoefs(sortedCoefs);
                    this->add(elementsetLMPC);
                    groupBySetOfCoefs[sortedCoefs]= group;
                }

                // Creating a cell and adding it to the CellGroup
                vector<int> nodes;
                for (int position : sortedNodesPosition){
                    const int nodeId = mesh->findNodeId(position);
                    nodes.push_back(nodeId);
                }
                int cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::polyType(static_cast<unsigned int>(nodes.size())), nodes, true);
                group->addCellPosition(cellPosition);
                if (configuration.logLevel >= LogLevel::DEBUG){
                    cout << "Building cells in group "<<group->getName()<<" from "<< *lmpc<<"."<<endl;
                }
            }

        }
    }

    // After the translation, we remove all LMPC constraints
    // We don't do this during the loop because some LMPC may be used by several analysis.
    for (auto analysis : this->analyses) {
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

    shared_ptr<Mesh> mesh = this->mesh;
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
            shared_ptr<Material> materialRBE2 = make_shared<Material>(this);
            materialRBE2->addNature(RigidNature(*this, 1));
            this->add(materialRBE2);

            shared_ptr<CellGroup> group = mesh->createCellGroup("RBE2_"+to_string(constraint->getOriginalId()), CellGroup::NO_ORIGINAL_ID, "RBE2");
            Rbar elementsetRbe2(*this, mesh->findNodeId(rbe2->getMaster()));
            elementsetRbe2.assignCellGroup(group);
            elementsetRbe2.assignMaterial(materialRBE2);
            this->add(elementsetRbe2);

            // Creating cells and adding them to the CellGroup
            const int masterId = mesh->findNodeId(rbe2->getMaster());
            for (int position : rbe2->getSlaves()){
                const int slaveId = mesh->findNodeId(position);
                vector<int> nodes = {masterId, slaveId};
                int cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG2, nodes, true);
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
            shared_ptr<Material> materialRBAR = make_shared<Material>(this);
            materialRBAR->addNature(RigidNature(*this, 1));
            this->add(materialRBAR);

            // Master Node : first one. Slave Node : second and last one
            const int masterNodeId = mesh->findNodeId(*rbar->getSlaves().begin());
            const int slaveNodeId = mesh->findNodeId(*rbar->getSlaves().rbegin());
            shared_ptr<CellGroup> group = mesh->createCellGroup("RBAR_"+to_string(constraint->getOriginalId()), CellGroup::NO_ORIGINAL_ID, "RBAR");

            Rbar elementsetRBAR(*this, masterNodeId);
            elementsetRBAR.assignCellGroup(group);
            elementsetRBAR.assignMaterial(materialRBAR);
            this->add(elementsetRBAR);

            // Creating a cell and adding it to the CellGroup
            vector<int> nodes = {masterNodeId, slaveNodeId};
            int cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG2, nodes, true);
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
            const int masterId = mesh->findNodeId(rbe3->getMaster());
            const DOFS& mDOFS = rbe3->getDOFS();

            int nbParts=0;
            map<DOFS, map<double, shared_ptr<CellGroup>>> groupByCoefByDOFS;

            for (int position : rbe3->getSlaves()){

                const int slaveId = mesh->findNodeId(position);
                int cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG2, {masterId, slaveId}, true);

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
                    shared_ptr<Material> materialRBE3 = make_shared<Material>(this);
                    materialRBE3->addNature(RigidNature(*this, Nature::UNAVAILABLE_DOUBLE, sCoef));
                    this->add(materialRBE3);

                    shared_ptr<CellGroup> group = mesh->createCellGroup("RBE3_"+to_string(nbParts)+"_"+to_string(constraint->getOriginalId()), CellGroup::NO_ORIGINAL_ID, "RBE3");
                    Rbe3 elementsetRbe3(*this, masterId, mDOFS, sDOFS);
                    elementsetRbe3.assignCellGroup(group);
                    elementsetRbe3.assignMaterial(materialRBE3);
                    this->add(elementsetRbe3);

                    if (configuration.logLevel >= LogLevel::DEBUG){
                        cout << "Building cells in CellGroup "<<group->getName()<<" from "<< *rbe3<<"."<<endl;
                    }
                    groupByCoefByDOFS[sDOFS][sCoef]= group;
                }
                groupRBE3=groupByCoefByDOFS[sDOFS][sCoef];
                groupRBE3->addCellPosition(cellPosition);
            }

            // Removing the constraint from the model.
            toBeRemoved.push_back(constraint);
        }

        for(auto& constraint: toBeRemoved) {
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
    using value = std::pair<node_box, unsigned>;
    for (const auto& constraintSet : this->getCommonConstraintSets()) {

        const int idConstraintSet = constraintSet->getId();
        const int originalIdConstraintSet = constraintSet->getOriginalId();
        const ConstraintSet::Type natConstraintSet = constraintSet->type;
        auto constraints = constraintSet->getConstraintsByType(Constraint::Type::SURFACE_SLIDE_CONTACT);

        vector<shared_ptr<Constraint>> toBeRemoved;
        for (const auto& constraint : constraints) {
            shared_ptr<CellGroup> group = mesh->createCellGroup("SURF_"+to_string(constraint->getOriginalId()), CellGroup::NO_ORIGINAL_ID, "SURF");
            SurfaceSlideSet elementsetSlide(*this);
            elementsetSlide.assignCellGroup(group);
            this->add(elementsetSlide);

            shared_ptr<const SurfaceSlide> surface = dynamic_pointer_cast<const SurfaceSlide>(constraint);
		    shared_ptr<const BoundaryElementFace> masterSurface = dynamic_pointer_cast<const BoundaryElementFace>(this->find(surface->master));
		    vector<node_polygon> masterFaces;
		    bgi::rtree< value, bgi::rstar<16> > rtree;
		    for (const auto& faceInfo : masterSurface->faceInfos) {
                const Cell& masterCell = this->mesh->findCell(this->mesh->findCellPosition(faceInfo.cellId));
                const vector<int>& faceIds = masterCell.faceids_from_two_nodes(faceInfo.nodeid1, faceInfo.nodeid2);
                if (faceIds.size() > 0) {
                    node_polygon masterFace;
                    for (const int faceId : faceIds) {
                        int nodePosition = this->mesh->findNodePosition(faceId);
                        const Node& node = this->mesh->findNode(nodePosition);
                        const node_point np{node.x, node.y, node.z};
                        masterFace.outer().push_back(np);
                        masterFaces.push_back(masterFace);
                    }
                    for (size_t i = 0 ; i < masterFaces.size() ; ++i) {
                        node_box b = bg::return_envelope<node_box>(masterFaces[i]);
                        rtree.insert(std::make_pair(b, i));
                    }
                }
		    }
		    shared_ptr<const BoundaryElementFace> slaveSurface = dynamic_pointer_cast<const BoundaryElementFace>(this->find(surface->slave));
            for (const auto& faceInfo : slaveSurface->faceInfos) {
                const Cell& slaveCell = this->mesh->findCell(this->mesh->findCellPosition(faceInfo.cellId));
                const vector<int>& faceIds = slaveCell.faceids_from_two_nodes(faceInfo.nodeid1, faceInfo.nodeid2);
                for (const int faceId : faceIds) {
                    int nodePosition = this->mesh->findNodePosition(faceId);
                    const Node& node = this->mesh->findNode(nodePosition);
                    vector<value> result_n;
                    rtree.query(bgi::nearest(node_point{node.x, node.y, node.z}, 1), std::back_inserter(result_n));
                    int masterFaceIndex = result_n[0].second;
                    const node_polygon& masterFace = masterFaces[masterFaceIndex];
                    size_t num_master_points = bg::num_points(masterFace);
                    switch(num_master_points) {
                    case 3:
                        break;
                    case 4:
                        break;
                    default:
                        throw logic_error("Slide element not yet implemented:" + to_string(num_master_points));
                    }

                }
		    }

            toBeRemoved.push_back(constraint);
        }

        for(auto& constraint: toBeRemoved) {
            remove(constraint->getReference(), idConstraintSet, originalIdConstraintSet, natConstraintSet);
        }
    }
}

void Model::splitElementsByDOFS(){

    vector<ScalarSpring> elementSetsToAdd;
    vector<shared_ptr<ElementSet>> elementSetsToRemove;

    for (auto & elementSet : elementSets) {

        switch (elementSet->type){
        case ElementSet::Type::DISCRETE_0D:
        case ElementSet::Type::DISCRETE_1D:
        case ElementSet::Type::NODAL_MASS:
        case ElementSet::Type::CIRCULAR_SECTION_BEAM:
        case ElementSet::Type::RECTANGULAR_SECTION_BEAM:
        case ElementSet::Type::I_SECTION_BEAM:
        case ElementSet::Type::GENERIC_SECTION_BEAM:
        case ElementSet::Type::STRUCTURAL_SEGMENT:
        case ElementSet::Type::SHELL:
        case ElementSet::Type::CONTINUUM:
        case ElementSet::Type::STIFFNESS_MATRIX:
        case ElementSet::Type::MASS_MATRIX:
        case ElementSet::Type::DAMPING_MATRIX:
        case ElementSet::Type::RIGIDSET:
        case ElementSet::Type::RBAR:
        case ElementSet::Type::RBE3:
        case ElementSet::Type::LMPC:{
            continue;
        }

        case ElementSet::Type::SCALAR_SPRING:{
            shared_ptr<ScalarSpring> ss = dynamic_pointer_cast<ScalarSpring>(elementSet);
            if (ss->getNbDOFSSpring()>1){
                int i =1;
                const double stiffness = ss->getStiffness();
                const double damping = ss->getDamping();
                const string name = ss->cellGroup->getName();
                const string comment = ss->cellGroup->getComment();
                if (configuration.logLevel >= LogLevel::DEBUG)
                    cout<< *elementSet << " spring must be split."<<endl;
                for (const auto & it : ss->getCellPositionByDOFS()){
                    ScalarSpring scalarSpring(*this, Identifiable<ElementSet>::NO_ORIGINAL_ID, stiffness, damping);
                    shared_ptr<CellGroup> cellGroup = this->mesh->createCellGroup(name+"_"+to_string(i), Group::NO_ORIGINAL_ID, comment);
                    scalarSpring.assignCellGroup(cellGroup);
                    for (const int cellPosition : it.second){
                        scalarSpring.addSpring(cellPosition, it.first.first, it.first.second);
                        cellGroup->addCellPosition(cellPosition);
                    }
                    elementSetsToAdd.push_back(scalarSpring);
                    i++;
                }
                elementSetsToRemove.push_back(elementSet);
                this->mesh->removeGroup(name);
            }
            break;
        }
        default: {
            //TODO : throw ModelException("ElementSet type not supported");
            cerr << "Warning in splitElementsByDOFS: " << *elementSet << " not supported" << endl;
        }
        }
    }

    for (auto elementSet : elementSetsToRemove){
        this->elementSets.erase(Reference<ElementSet>(*elementSet));
    }
    for (auto & elementSet : elementSetsToAdd){
        this->add(elementSet);
    }
}

void Model::makeBoundarySegments() {
    for (const auto& constraintSet : this->getCommonConstraintSets()) {
        auto constraints = constraintSet->getConstraintsByType(Constraint::Type::SLIDE);
        for (const auto& constraint : constraints) {
            shared_ptr<SlideContact> slide = dynamic_pointer_cast<SlideContact>(constraint);
            shared_ptr<CellGroup> masterCellGroup = mesh->createCellGroup("SLIDE_M_"+to_string(slide->getOriginalId()), Group::NO_ORIGINAL_ID, "created by makeBoundarySegments() for the master in a SLIDE contact");
            shared_ptr<CellGroup> slaveCellGroup = mesh->createCellGroup("SLIDE_S_"+to_string(slide->getOriginalId()), Group::NO_ORIGINAL_ID, "created by makeBoundarySegments() for the slave in a SLIDE contact");
            shared_ptr<BoundaryNodeLine> masterNodeLine = dynamic_pointer_cast<BoundaryNodeLine>(this->find(slide->master));
            const list<int>& masterNodeIds = masterNodeLine->nodeids;
            auto it = masterNodeIds.begin();
            for(unsigned int i = 0; i < masterNodeIds.size() - 1;++i) {
                int nodeId1 = *it;
                int nodeId2 = *(++it);
                vector<int> connectivity {nodeId1, nodeId2};
                int cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG2, connectivity, true);
                masterCellGroup->addCellPosition(cellPosition);
            }
            slide->masterCellGroup = masterCellGroup;
            shared_ptr<BoundaryNodeLine> slaveNodeLine = dynamic_pointer_cast<BoundaryNodeLine>(this->find(slide->slave));
            const list<int>& slaveNodeIds = slaveNodeLine->nodeids;
            auto it2 = slaveNodeIds.begin();
            for(unsigned int i = 0; i < slaveNodeIds.size() - 1;++i) {
                int nodeId1 = *it2;
                int nodeId2 = *(++it2);
                vector<int> connectivity = {nodeId1, nodeId2};
                int cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG2, connectivity, true);
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
            shared_ptr<CellGroup> masterCellGroup = mesh->createCellGroup("SURFCONT_M_"+to_string(surface->getOriginalId()), Group::NO_ORIGINAL_ID, "created by makeBoundarySurfaces() for the master in a SURFACE_CONTACT");
            shared_ptr<CellGroup> slaveCellGroup = mesh->createCellGroup("SURFCONT_S_"+to_string(surface->getOriginalId()), Group::NO_ORIGINAL_ID, "created by makeBoundarySurfaces() for the master in a SURFACE_CONTACT");
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
                int cellPosition = mesh->addCell(Cell::AUTO_ID, cellType, connectivity, true);
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
                int cellPosition = mesh->addCell(Cell::AUTO_ID, cellType, connectivity, true);
                masterCellGroup->addCellPosition(cellPosition);
            }
            surface->masterCellGroup = masterCellGroup;
        }
    }
}

void Model::addAutoAnalysis() {
    auto& nonLinearStrategies = objectives.filter(Objective::Type::NONLINEAR_STRATEGY);
    bool linearStatic = true;
    // LD very basic implementation of analysis detection. Should also look for non linear materials etc ?
    if (nonLinearStrategies.size() >= 1) {
        for(auto& nonLinearStrategy : nonLinearStrategies) {
            NonLinearMecaStat analysis(*this, nonLinearStrategy->getOriginalId());
            this->add(analysis);
            linearStatic = false;
        }
    }
    auto& modalStrategies = objectives.filter(Objective::Type::FREQUENCY_TARGET);
    if (modalStrategies.size() >= 1) {
        for(auto& modalStrategy : modalStrategies) {
            LinearModal analysis(*this, modalStrategy->getOriginalId());
            this->add(analysis);
            linearStatic = false;
        }
    }
    if (linearStatic) {
        LinearMecaStat analysis(*this);
        this->add(analysis);
    }
}

void Model::finish() {
    if (finished) {
        return;
    }

    /* Build the coordinate systems from their definition points */
    for (auto& coordinateSystemEntry : mesh->coordinateSystemStorage.coordinateSystemByRef) {
        coordinateSystemEntry.second->build();
    }

    for (shared_ptr<ElementSet> elementSet : elementSets) {
        for (int nodePosition : elementSet->nodePositions()) {
            mesh->allowDOFS(nodePosition,elementSet->getDOFSForNode(nodePosition));
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

    this->mesh->finish();
    finished = true;
}

bool Model::validate() {
    bool meshValid = mesh->validate();

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
            CellContainer container(*mesh);
            container.add(*element->cellGroup);
            mesh->assignElementId(container, element->getId());
        }
    }
}

} /* namespace vega */
