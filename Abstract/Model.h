/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Model.h
 *
 *  Created on: Jan 11, 2013
 *      Author: dallolio
 */

#ifndef MODEL_H_
#define MODEL_H_

#include "Dof.h"
#include "Analysis.h"
#include "ConfigurationParameters.h"
#include "Loading.h"
#include "Constraint.h"
#include "Material.h"
#include "Element.h"
#include "Mesh.h"
#include "Value.h"
#include "Objective.h"
#include "Reference.h"
#include "Target.h"
#include <string>

namespace vega {

class Mesh;

/**
 * Responsible of collecting any information for a finite element problem definition.
 */

enum class ModelParameter {
    MASS_OVER_FORCE_MULTIPLIER,
    PRINT_MAXIM,
    LARGE_DISPLACEMENTS,
    LOWER_CUTOFF_FREQUENCY,
    UPPER_CUTOFF_FREQUENCY,
    ELEMENT_QUALITY_CHECK,
    STRUCTURAL_DAMPING,
    FREQUENCY_OF_INTEREST_RADIANS,
    SHELL_NORMAL_STIFFNESS_FACTOR /**< see Nastran K6ROT, Aster COEF_RIGI_DRZ http://eric.cabrol.free.fr/CalculEF/params_Nastran.html */
};

class Model final {
private:
    std::shared_ptr<Material> virtualMaterial = nullptr;
    void generateDiscrets();
    void generateSkin();
    void emulateLocalDisplacementConstraint();
    void emulateAdditionalMass();
    void generateBeamsToDisplayMasterSlaveConstraint();
    /**
     * Generate material assignment if the input model and the output model
     * differs: some solver (Nastran) wants the materials assigned to the
     * elements, some other (Aster) assign the materials to cells.
     * @see ConfigurationParameters.partitionModel
     */
    void generateMaterialAssignments();
    void removeIneffectives();
    void removeUnassignedMaterials();
    void replaceCombinedLoadSets();
    /**
     * This method assign a virtual material to some specific element types, important
     * for Code_Aster
     */
    void assignVirtualMaterial();
    /**
     * This method assign elements to cells. It is important for Code Aster to know which
     * Element is associated to a cell (for instance to write an FORCE_ARETE or a FORCE_POUTRE)
     */
    void assignElementsToCells();
    void removeAssertionsMissingDOFS();
    void addDefaultAnalysis();
    void replaceDirectMatrices();
    void removeRedundantSpcs();
    /**
     * Split all direct matrices whose sizes is greater than sizeMax into smaller matrices.
     * All data is kept, just divided into various elements.
     * Used, for example, by Systus, where cells can't have more than 20 nodes.
     * Must be called before makeCellsFromDirectMatrices().
     */
    void splitDirectMatrices(const unsigned int sizeMax);
    /**
     * Build the corresponding cells of ElementSets of Direct Matrices (DISCRETE_0D, DISCRETE_1D,
     * STIFFNESS_MATRIX, MASS_MATRIX, DAMPING_MATRIX). They generally don't have associated cells/cellgroup.
     * This may causes problems in a generic cell writer, such as in SYSTUS.
     */
    void makeCellsFromDirectMatrices();
    /**
     * Build the corresponding ElmentSets, Cellgroup and cells corresponding to LMPC constraints.
     * In some solver (ex SYSTUS), these constraints are enforced via rigid elements.
     */
    void makeCellsFromLMPC();
    /**
     * Build the corresponding cells of RigidSets (RBAR, RBE3) with a group, and a Rigid material.
     */
    void makeCellsFromRBE();
    /**
     * Build the corresponding cells of Surface slides (Optistruct SURF) with a group.
     */
    void makeCellsFromSurfaceSlide();
    /**
     * Some elementSet can hold very general elements, acting on various DOFs of the corresponding cells.
     * For example, Nastran PELAS1 can regroup CELAS1 cells which "spring" in various DOFS (DXtoDX, DYtoDZ, etc)
     * whereas Systus can only have one spring direction by 1902 Part.
     * This method splits the ElementSets to have only one direction in each.
     */
    void splitElementsByDOFS();
    /**
     * Explicitely creating cell elements defined by nodes (see Nastran BLSEG)
     */
    void makeBoundarySegments();
    /**
     * Explicitely creating cell elements defined by nodes (see Nastran BSSEG)
     */
    void makeBoundarySurfaces();
    /**
     * Automatically add the analysis when missing
     */
    void addAutoAnalysis();

    /**
     * Remove constraints if an imposed displacement is also present
     * Nastran (and Systus): SPC1+SPCD, Aster ASSEMBLA_26
     * "le noeud:  composante:  est bloqué plusieurs fois."
     */
    void removeConstrainedImposed();

    /**
     * Replace rigid elements (for example PBUSH with RIGID keywords)
     * see http://blog.altair.co.kr/wp-content/uploads/2013/08/PBUSH.pdf
     * using cinematic constraints to avoid conditionment
     * problems in stiffness matrix
     */
    void replaceRigidSegments();

    /**
     * Replace parametric functions (ex: ABSC coordinates, like PLOAD1 with LR option in Nastran)
     * with absolute (X,Y,Z coordinates)
     */
    void changeParametricForceLineToAbsolute();

    /**
     * Converts 0D Discrets into 1Ds
     */
    void convert0DDiscretsInto1D();

    /**
     * Create groups out of loadsets and constraintsets and SETs (mainly for display)
     */
    void createSetGroups();

    /**
     * Get a non rigid material (virtual)
     */
    std::shared_ptr<Material> getVirtualMaterial();

public:
    bool finished = false;
    bool afterValidation = false;
    std::string name;
    std::string inputSolverVersion;
    const SolverName inputSolver;
    ModelType modelType;
    std::string title = "";
    std::string description = "";

    Mesh mesh; /**< Handles geometrical information */
    const ModelConfiguration configuration;
    vega::ConfigurationParameters::TranslationMode translationMode;
    const std::shared_ptr<LoadSet> commonLoadSet;
    const std::shared_ptr<ConstraintSet> commonConstraintSet;
    const std::shared_ptr<ObjectiveSet> commonObjectiveSet;

private:
    std::unordered_map<LoadSet::Type, std::map<int, std::set<Reference<Loading>> > ,EnumClassHash>
    loadingReferences_by_loadSet_original_ids_by_loadSet_type;
    std::unordered_map<int, std::set<Reference<Loading>> >
    loadingReferences_by_loadSet_ids;

    std::unordered_map< ConstraintSet::Type,
    std::map<int, std::set<Reference<Constraint>>>,EnumClassHash>
    constraintReferences_by_constraintSet_original_ids_by_constraintSet_type;

    std::unordered_map< ObjectiveSet::Type,
    std::map<int, std::set<Reference<Objective>>>,EnumClassHash>
    objectiveReferences_by_objectiveSet_original_ids_by_objectiveSet_type;

    std::map< int, std::set<Reference<Constraint>>>
    constraintReferences_by_constraintSet_ids;
    std::map< int, std::set<Reference<Objective>>>
    objectiveReferences_by_objectiveSet_ids;

    template<class T> class Container final {
    private:
        std::map<int, std::shared_ptr<T>> by_id;
        std::unordered_map< typename T::Type, std::map<int, std::shared_ptr<T>>,
        EnumClassHash> by_original_ids_by_type;
        Model& model;
    public:
        Container(Model& model): model(model) {}
        Container(const Container& that) = delete; /**< Containers should never be copied */
        class iterator;
        friend class iterator;
        class iterator : public std::iterator< std::input_iterator_tag,T,ptrdiff_t> {
            typename std::map<int,std::shared_ptr<T>>::const_iterator it;
            const std::map<int,std::shared_ptr<T>>* mp;
        public:
            iterator(const std::map<int,std::shared_ptr<T>>& mp, const typename std::map<int,std::shared_ptr<T>>::const_iterator& it) : it(it), mp(&mp) {}
                bool operator==(const iterator& x) const {
                    return it == x.it;
                }
                bool operator!=(const iterator& x) const {
                    return !(*this == x);
                }
                const std::shared_ptr<T> operator*() const {
                    return it->second;
                }
                iterator& operator++() {
                    ++it;
                    return *this;
                }
                iterator operator++(int) {
                    iterator tmp = *this;
                    ++*this;
                    return tmp;
                }
        }; /* iterator class */
        iterator begin() const {return iterator(by_id, by_id.begin());}
        iterator end() const {return iterator(by_id, by_id.end());}
        std::shared_ptr<T> first() const {return *begin();};
        std::shared_ptr<T> last() const {return by_id.rbegin()->second;};
        size_t size() const {return by_id.size();}
        bool empty() const {return by_id.empty();}
        void add(std::shared_ptr<T> T_ptr);
        void erase(const Reference<T> ref);
        std::shared_ptr<T> find(const Reference<T>&) const;
        std::shared_ptr<T> find(int) const; /**< Find an object by its Original Id **/
        std::shared_ptr<T> get(int) const; /**< Return an object by its Vega Id **/
        bool contains(const typename T::Type type) const; /**< Ask if objects of a given type exist inside */
        std::vector<std::shared_ptr<T>> filter(const typename T::Type type) const; /**< Choose objects based on their type */
        //const std::vector<std::shared_ptr<T>> filter(const std::unordered_set<const typename T::Type> types) const; /**< Choose objects based on their types */
        bool validate(); /**< Says if model parts are coherent (no unresolved references, etc.) AND SOMETIMES IT TRIES TO FIX THEM :( */
        bool checkWritten() const; /**< Says if all container objects have been written in output (or not) */
    }; /* Container class */
    std::unordered_map<int,CellContainer> material_assignment_by_material_id;
    std::map<ModelParameter, std::string> parameters;
public:
    Container<Analysis> analyses{*this};
    Container<Objective> objectives{*this};
    Container<NamedValue> values{*this};
    Container<Loading> loadings{*this};
    Container<LoadSet> loadSets{*this};
    Container<Constraint> constraints{*this};
    Container<ConstraintSet> constraintSets{*this};
    Container<ElementSet> elementSets{*this};
    Container<ObjectiveSet> objectiveSets{*this};
    Container<Material> materials{*this};
    Container<Target> targets{*this};
    bool onlyMesh = false;

    Model(std::string name, std::string inputSolverVersion = "UNKNOWN",
            SolverName inputSolver = SolverName::NASTRAN,
            const ModelConfiguration configuration = ModelConfiguration(),
            const vega::ConfigurationParameters::TranslationMode translationMode = vega::ConfigurationParameters::TranslationMode::BEST_EFFORT);
    Model(const Model& that) = delete; /** bad bad things happens if you ever try to copy a Model (back references to model are not up to date). */

    /**
     * Add any kind of object to the model.
     */
    //TODO : make and use a template, does not work because of inherited objects, or, better, avoid need to add altogether (object creation should implicitily add)
    //template<typename T> void add(const T&);
    void add(const std::shared_ptr<Analysis>&);
    void add(const std::shared_ptr<Loading>&);
    void add(const std::shared_ptr<LoadSet>&);
    void add(const std::shared_ptr<Constraint>&);
    void add(const std::shared_ptr<ConstraintSet>&);
    void add(const std::shared_ptr<Objective>&);
    void add(const std::shared_ptr<ObjectiveSet>&);
    void add(const std::shared_ptr<NamedValue>&);
    void add(const std::shared_ptr<ElementSet>&);
    void add(const std::shared_ptr<Target>&);
    void add(const std::shared_ptr<Material>&);

    // Get functions : get object by their VEGA Id.
    // Mainly here in order to instanciate all template type for the Container template functions
    std::shared_ptr<Analysis> getAnalysis(int id) const; /**< Return an Analysis by its Vega Id **/
    std::shared_ptr<Loading> getLoading(int id) const;   /**< Return a Loading by its Vega Id **/
    std::shared_ptr<LoadSet> getLoadSet(int id) const;   /**< Return a LoadSet by its Vega Id **/
    std::shared_ptr<Constraint> getConstraint(int id) const; /**< Return a Constraint by its Vega Id **/
    std::shared_ptr<ConstraintSet> getConstraintSet(int id) const; /**< Return a ConstraintSet by its Vega Id **/
    std::shared_ptr<Objective> getObjective(int id) const; /**< Return an Objective by its Vega Id **/
    std::shared_ptr<NamedValue> getValue(int id) const; /**< Return a Value by its Vega Id **/
    std::shared_ptr<ElementSet> getElementSet(int id) const; /**< Return an ElementSet by its Vega Id **/
    std::shared_ptr<Material> getMaterial(int id) const; /**< Return a Material by its Vega Id **/
    std::shared_ptr<Target> getTarget(int id) const; /**< Return a Material by its Vega Id **/
    std::shared_ptr<ObjectiveSet> getObjectiveSet(int id) const;   /**< Return an ObjectiveSet by its Vega Id **/

    std::shared_ptr<LoadSet> getOrCreateLoadSet(int loadset_id, vega::LoadSet::Type); /**< Return or create a LoadSet by its real Id **/
    std::shared_ptr<ObjectiveSet> getOrCreateObjectiveSet(int objectiveSet_id, vega::ObjectiveSet::Type); /**< Return or create a LoadSet by its real Id **/

    void setParameter(const ModelParameter& parameter, const std::string& value) noexcept;

    bool contains(const ModelParameter& parameter) const noexcept;

    std::string getParameter(const ModelParameter& parameter) const noexcept;

    /* Get the Id of all elements belonging to set */
    //TODO: make a template, general function?
    std::vector<int> getMaterialsId() const;
    std::vector<int> getElementSetsId() const;
    /**
     * Remove any kind of object from the model, by giving a reference.
     * Very time consuming when the list is big : restrict use to the minimum
     */
    template<typename T>
    void remove(const Reference<T>);

    /**
     * Remove a constraint from a known reference set when we already know some informations
     */
    void remove(const Reference<Constraint>, const Reference<ConstraintSet>);
    /**
     * Retrieve any kind of object from the model, by giving a reference.
     * Return nullptr if the object is not found in the model.
     */
    template<typename T>
    std::shared_ptr<T> find(const Reference<T>) const;

    /**
     * Add a Loading reference into a LoadSet reference.
     * If needed, create and add the LoadSet reference to the model.
     */
    void addLoadingIntoLoadSet(const Reference<Loading>&, const Reference<LoadSet>&);

    /**
     * Add an Objective reference into an ObjectiveSet reference.
     */
    void addObjectiveIntoObjectiveSet(const Reference<Objective>&, const Reference<ObjectiveSet>&);

    /**
     * Retrieve all the Loadings corresponding to a given LoadSet.
     */
    std::set<std::shared_ptr<Loading>, ptrLess<Loading>> getLoadingsByLoadSet(const Reference<LoadSet>&) const;

    /**
     * Create a material
     */
    std::shared_ptr<Material> getOrCreateMaterial(
            int materialId, bool createIfNotExists = true);

    /**
     * Get all the cells assigned to a specific material. This inspects
     * both the elementSets with a material assigned and the materials assigned
     * directly.
     *
     * If no assigment is found it returns an empty cell container.
     */
    CellContainer getMaterialAssignment(int materialId) const;
    /**
     * Assign a material to a group of cells. There are two ways of assigning
     * a material: either trough this method or with an ElementSet.
     * Choose the one appropriate to your input model.
     */
    void assignMaterial(int materialId, const CellContainer& cellsToAssign);

    /**
     * Add a Constraint reference into a ConstraintSet reference.
     */
    void addConstraintIntoConstraintSet(const Reference<Constraint>&, const Reference<ConstraintSet>&);

    /**
     * Retrieve all the Constraints corresponding to a given ConstraintSet.
     */
    std::set<std::shared_ptr<Constraint>, ptrLess<Constraint>> getConstraintsByConstraintSet(const Reference<ConstraintSet>&) const;

    /**
     * Retrieve all the ConstraintSet containing a corresponding Constraint.
     */
    std::set<std::shared_ptr<ConstraintSet>, ptrLess<ConstraintSet>> getConstraintSetsByConstraint(const Reference<Constraint>& constraintReference) const;

    /**
     * Retrieve all the ConstraintSet of the model that are at least referenced by one analysis
     */
    std::vector<std::shared_ptr<ConstraintSet>> getActiveConstraintSets() const;

    /**
     * Retrieve all the LoadSet of the model that are at least referenced by one analysis
     */
    std::vector<std::shared_ptr<LoadSet>> getActiveLoadSets() const;

    /**
     * Retrieve all the Objectives corresponding to a given ObjectiveSet.
     */
    std::set<std::shared_ptr<Objective>, ptrLess<Objective>> getObjectivesByObjectiveSet(const Reference<ObjectiveSet>&) const;

    /**
     * Retrieve all the ConstraintSet of the model that are common to all analysis
     */
    std::vector<std::shared_ptr<ConstraintSet>> getCommonConstraintSets() const;

    /**
     * Retrieve all the LoadSet of the model that are are common to all analysis
     */
    std::vector<std::shared_ptr<LoadSet>> getCommonLoadSets() const;
    /**
     * Retrieve all the ConstraintSet of the model that are active but not common to all analysis
     */
    std::set<std::shared_ptr<ConstraintSet>, ptrLess<ConstraintSet>> getUncommonConstraintSets() const;
    /**
     * Retrieve all the ConstraintSet of the model that are active but not common to all analysis
     */
    std::set<std::shared_ptr<LoadSet>, ptrLess<LoadSet>> getUncommonLoadSets() const;

    /**
	 * Retrieves only 1D elements having rotational stiffness in linear static analysis, false for Truss elements (also see getTrusses() method )
	 */
    std::vector<std::shared_ptr<Beam>> getBeams() const;

    /**
	 * Retrieves only 1D elements having no rotational stiffness in linear static analysis (also see getBeams() method )
	 */
    std::vector<std::shared_ptr<Beam>> getTrusses() const;
    bool needsLargeDisplacements() const;

    std::shared_ptr<Analysis> reusableAnalysisFor(const std::shared_ptr<Analysis>&) const noexcept;
    bool canBeReused(const std::shared_ptr<Analysis>&) const noexcept;

    /**
     * Method that is called when parsing is complete.
     */
    void finish();
    /**
     * This method should be called after finish to check if the model
     * is correct. Validation results are printed to stdout/stderr.
     */
    bool validate();
    bool checkWritten() const; /**< Says if model parts have been completely translated */

};

/*
 * Template implementations need to stay in header
 */

template<class T>
void Model::Container<T>::erase(const Reference<T> ref) {
    by_id.erase(ref.id);
    if (ref.has_original_id())
        by_original_ids_by_type[ref.type].erase(ref.original_id);
}

template<class T>
std::vector<std::shared_ptr<T>> Model::Container<T>::filter(const typename T::Type type) const {
    std::vector<std::shared_ptr<T>> result;
    for (const auto& id_obj_pair: by_id) {
        if (id_obj_pair.second->type == type) {
            result.push_back(id_obj_pair.second);
        }
    }
    return result;
}

/*template<class T>
std::vector<std::shared_ptr<T>> Model::Container<T>::filter(const std::unordered_set<const typename T::Type> types) const {
    std::vector<std::shared_ptr<T>> result;
    for (const auto& id_obj_pair: by_id) {
        if (types.find(id_obj_pair.second->type) != types.end()) {
            result.push_back(id_obj_pair.second);
        }
    }
    return result;
}*/

template<class T>
bool Model::Container<T>::contains(const typename T::Type type) const {
    for (const auto& id_obj_pair: by_id) {
        if (id_obj_pair.second->type == type) {
            return true;
        }
    }
    return false;
}

template<class T>
bool Model::Container<T>::validate() {
    bool isValid = true;
    std::vector<std::shared_ptr<T>> toBeRemoved;
    for (iterator it = this->begin(); it != this->end(); ++it) {
        std::shared_ptr<T> t = *it;
        if (!t->validate()) {
            isValid = false;
            std::cerr << *t << " is not valid" << std::endl;

            switch (model.translationMode) {
            case vega::ConfigurationParameters::TranslationMode::MODE_STRICT:
                // Shouldn't do any cleanup in STRICT mode
                break;
            case vega::ConfigurationParameters::TranslationMode::MESH_AT_LEAST:
            case vega::ConfigurationParameters::TranslationMode::BEST_EFFORT:
                toBeRemoved.push_back(t);
                break;
            default:
                throw std::logic_error("Unknown enum class in Translation mode");
            }
        }
    }
    for(const auto& t: toBeRemoved) {
        this->erase(*t);
    }
    return isValid;
}

template<class T>
bool Model::Container<T>::checkWritten() const {
    bool isWritten = true;
    for (iterator it = this->begin(); it != this->end(); ++it) {
        std::shared_ptr<T> t = *it;
        if (!t->isWritten()) {
            isWritten = false;
            std::cerr << *t << " hasn't been written." << std::endl;
        }
    }
    return isWritten;
}


/*
 * Redefining method add for Value to take into account placeHolder
 */
template<>
inline void Model::Container<NamedValue>::add(std::shared_ptr<NamedValue> ptr) {
    const auto& ptr_old = this->find(ptr->getReference());
    if (ptr_old != nullptr) {
        if (ptr->isPlaceHolder()) { // TODO : make a merge function for placeHolder
            const auto& funPtr = std::static_pointer_cast<Function>(ptr);
            const auto& funptr_old = std::static_pointer_cast<Function>(ptr_old);
            if (funPtr->hasParaX())
                funptr_old->setParaX(funPtr->getParaX());
            if (funPtr->hasParaY())
                funptr_old->setParaY(funPtr->getParaY());
            ptr = ptr_old;
        } else if (ptr_old->isPlaceHolder()) {
            const auto& funPtr = std::static_pointer_cast<Function>(ptr);
            const auto& funptr_old = std::static_pointer_cast<Function>(ptr_old);
            if (funptr_old->hasParaX())
                funPtr->setParaX(funptr_old->getParaX());
            if (funptr_old->hasParaY())
                funPtr->setParaY(funptr_old->getParaY());
        } else {
            std::ostringstream oss;
            oss << *ptr << " is already in the model";
            throw std::runtime_error(oss.str());
        }
    }
    if (!ptr->isPlaceHolder()) {
        by_id[ptr->getId()] = ptr;
    }
    if (ptr->isOriginal()) {
        by_original_ids_by_type[ptr->type][ptr->getOriginalId()] = ptr;
    }
}

template<class T>
void Model::Container<T>::add(std::shared_ptr<T> ptr) {
    if (this->find(ptr->getReference()) != nullptr) {
        std::ostringstream oss;
        oss << *ptr << " is already in the model";
        throw std::runtime_error(oss.str());
    }
    by_id[ptr->getId()] = ptr;
    if (ptr->isOriginal())
        by_original_ids_by_type[ptr->type][ptr->getOriginalId()] = ptr;
}

template<class T>
std::shared_ptr<T> Model::Container<T>::find(const Reference<T>& reference) const {
    std::shared_ptr<T> t;
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
        throw std::logic_error("Reference is not valid:" + to_str(reference));
    }

    return t;
}

template<class T>
std::shared_ptr<T> Model::Container<T>::find(int original_id) const {
    std::shared_ptr<T> t;
    for (const auto& it : by_original_ids_by_type) {
        const auto& it2 = it.second.find(original_id);
        if (it2 != it.second.end()) {
            t = it2->second;
        }
    }
    return t;
}

template<class T>
std::shared_ptr<T> Model::Container<T>::get(int id) const {
    std::shared_ptr<T> t = nullptr;
    auto it = by_id.find(id);
    if (it != by_id.end()) {
    	t = it->second;
    }
    return t;
}

} /* namespace vega */

#endif /* MODEL_H_ */
