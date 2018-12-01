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
//class Constraint;

/**
 * Responsible of collecting any information for a finite element problem definition.
 */

class Model final {
private:
    const std::string type;
    std::shared_ptr<Material> virtualMaterial;
    void generateDiscrets();
    void generateSkin();
    void emulateLocalDisplacementConstraint();
    void emulateAdditionalMass();
    void generateBeamsToDisplayHomogeneousConstraint();
    /**
     * Generate material assignment if the input model and the output model
     * differs: some solver (Nastran) wants the materials assigned to the
     * elements, some other (Aster) assign the materials to cells.
     * @see ConfigurationParameters.partitionModel
     */
    void generateMaterialAssignments();
    Cell generateSkinCell(const std::vector<int>& faceIds, const SpaceDimension& dimension);
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
     * Get a non rigid material (virtual)
     */
    std::shared_ptr<Material> getVirtualMaterial();

public:
    bool finished;
    bool afterValidation = false;
    std::string name;
    std::string inputSolverVersion;
    const SolverName inputSolver;
    ModelType modelType;
    std::string title;
    std::string description;

    const ModelConfiguration configuration;
    vega::ConfigurationParameters::TranslationMode translationMode;
    std::shared_ptr<Mesh> mesh; /**< Handles geometrical information */

    enum class Parameter {
        MASS_OVER_FORCE_MULTIPLIER,
        PRINT_MAXIM,
        LARGE_DISPLACEMENTS,
        LOWER_CUTOFF_FREQUENCY,
        UPPER_CUTOFF_FREQUENCY,
        ELEMENT_QUALITY_CHECK,
        STRUCTURAL_DAMPING,
        FREQUENCY_OF_INTEREST_RADIANS
    };
    const LoadSet commonLoadSet;
    const ConstraintSet commonConstraintSet;

private:
    std::unordered_map<LoadSet::Type, std::map<int, std::set<std::shared_ptr<Reference<Loading>>> > ,EnumClassHash>
    loadingReferences_by_loadSet_original_ids_by_loadSet_type;
    std::unordered_map<int, std::set<std::shared_ptr<Reference<Loading>>> >
    loadingReferences_by_loadSet_ids;

    std::unordered_map< ConstraintSet::Type,
    std::map<int, std::set<std::shared_ptr<Reference<Constraint>>>>,EnumClassHash>
    constraintReferences_by_constraintSet_original_ids_by_constraintSet_type;
    std::map< int, std::set<std::shared_ptr<Reference<Constraint>>>>
    constraintReferences_by_constraintSet_ids;

    template<class T> class Container final {
        std::map<int, std::shared_ptr<T>> by_id;
        std::unordered_map< typename T::Type, std::map<int, std::shared_ptr<T>>,
        EnumClassHash> by_original_ids_by_type;
    private:
        Model& model;
    public:
        Container(Model& model): model(model) {}
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
        std::shared_ptr<T> first() const {return *begin();};
        iterator end() const {return iterator(by_id, by_id.end());}
        int size() const {return static_cast<int>(by_id.size());}
        bool empty() const {return by_id.size() == 0;}
        void add(const T&);
        void add(std::shared_ptr<T> T_ptr);
        void erase(const Reference<T>);
        std::shared_ptr<T> find(const Reference<T>&) const;
        std::shared_ptr<T> find(int) const; /**< Find an object by its Original Id **/
        std::shared_ptr<T> get(int) const; /**< Return an object by its Vega Id **/
        const std::vector<std::shared_ptr<T>> filter(const typename T::Type) const; /**< Choose objects based on their type */
        bool contains(const typename T::Type) const; /**< Ask if objects of a given type are contained */
        bool validate(){
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
            for(auto& t: toBeRemoved) {
                this->erase(*t);
            }
            return isValid;
        };
        Container(const Container& that) = delete; /**< Containers should never be copied */
        }; /* Container class */
        std::unordered_map<int,CellContainer> material_assignment_by_material_id;
    public:
        Container<Analysis> analyses{*this};
        Container<Objective> objectives{*this};
        Container<NamedValue> values{*this};
        Container<Loading> loadings{*this};
        Container<LoadSet> loadSets{*this};
        Container<Constraint> constraints{*this};
        Container<ConstraintSet> constraintSets{*this};
        Container<ElementSet> elementSets{*this};
        Container<Material> materials{*this};
        Container<Target> targets{*this};
        std::map<Parameter, double> parameters;
        bool onlyMesh;

        Model(std::string name, std::string inputSolverVersion = std::string("UNKNOWN"),
                SolverName inputSolver = SolverName::NASTRAN,
                const ModelConfiguration configuration = ModelConfiguration(),
                const vega::ConfigurationParameters::TranslationMode translationMode = vega::ConfigurationParameters::TranslationMode::BEST_EFFORT);
        Model(const Model& that) = delete; /** bad bad things happens if you ever try to copy a Model (back references to model are not up to date). */
        virtual ~Model();

        /**
         * Add any kind of object to the model.
         */
        //TODO : make and use a template, does not work because of inherited objects, or, better, avoid need to add altogether (object creation should implicitily add)
        //template<typename T> void add(const T&);
        void add(const Analysis&);
        void add(const Loading&);
        void add(const LoadSet&);
        void add(const Constraint&);
        void add(const ConstraintSet&);
        void add(const Objective&);
        void add(const NamedValue&);
        void add(const ElementSet&);
        void add(const Target&);
        void add(const std::shared_ptr<Material>);

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

        std::shared_ptr<LoadSet> getOrCreateLoadSet(int loadset_id, vega::LoadSet::Type loadset_type); /**< Return or create a LoadSet by its real Id **/


        /* Get the Id of all elements belonging to set */
        //TODO: make a template, general function?
        const std::vector<int> getMaterialsId() const;
        const std::vector<int> getElementSetsId() const;
        /**
         * Remove any kind of object from the model, by giving a reference.
         * Very time consuming when the list is big : restrict use to the minimum
         */
        template<typename T>
        void remove(const Reference<T>);

        /**
         * Remove a constraint from a known reference set when we already know some informations
         */
        void remove(const Reference<Constraint> , const int, const int, const ConstraintSet::Type);
        /**
         * Retrieve any kind of object from the model, by giving a reference.
         * Return 0 if the object is not found in the model.
         */
        template<typename T>
        const std::shared_ptr<T> find(const Reference<T>) const;

        /**
         * Add a Loading reference into a LoadSet reference.
         * If needed, create and add the LoadSet reference to the model.
         */
        void addLoadingIntoLoadSet(const Reference<Loading>&, const Reference<LoadSet>&);

        /**
         * Retrieve all the Loadings corresponding to a given LoadSet.
         */
        const std::set<std::shared_ptr<Loading>> getLoadingsByLoadSet(const Reference<LoadSet>&) const;

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
        const CellContainer getMaterialAssignment(int materialId) const;
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
        const std::set<std::shared_ptr<Constraint>> getConstraintsByConstraintSet(const Reference<ConstraintSet>&) const;

        /**
         * Retrieve all the ConstraintSet containing a corresponding Constraint.
         */
        const std::set<std::shared_ptr<ConstraintSet>> getConstraintSetsByConstraint(const Reference<Constraint>& constraintReference) const;

        /**
         * Retrieve all the ConstraintSet of the model that are at least referenced by one analysis
         */
        const std::vector<std::shared_ptr<ConstraintSet>> getActiveConstraintSets() const;

        /**
         * Retrieve all the LoadSet of the model that are at least referenced by one analysis
         */
        const std::vector<std::shared_ptr<LoadSet>> getActiveLoadSets() const;
        /**
         * Retrieve all the ConstraintSet of the model that are common to all analysis
         */
        const std::vector<std::shared_ptr<ConstraintSet>> getCommonConstraintSets() const;

        /**
         * Retrieve all the LoadSet of the model that are are common to all analysis
         */
        const std::vector<std::shared_ptr<LoadSet>> getCommonLoadSets() const;
        /**
         * Retrieve all the ConstraintSet of the model that are active but not common to all analysis
         */
        const std::set<std::shared_ptr<ConstraintSet>> getUncommonConstraintSets() const;
        /**
         * Retrieve all the ConstraintSet of the model that are active but not common to all analysis
         */
        const std::set<std::shared_ptr<LoadSet>> getUncommonLoadSets() const;

        const std::vector<std::shared_ptr<Beam>> getBeams() const;

        /**
         * Method that is called when parsing is complete.
         */
        void finish();
        /**
         * This method should be called after finish to check if the model
         * is correct. Validation results are printed to stdout/stderr.
         */
        bool validate();

    };

}
                    /* namespace abstract */
#endif /* MODEL_H_ */
