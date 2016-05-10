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
#include <string>

namespace vega {

using namespace std;
class Mesh;
//class Constraint;

/**
 * Responsible of collecting any information for a finite element problem definition.
 */

class Model final {
private:
    const string type;
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
    Cell generateSkinCell(const vector<int>& faceIds, const SpaceDimension& dimension);
    void removeIneffectives();
    void replaceCombinedLoadSets();
    /**
     * This method assign elements to cells. It is important for Code Aster to know which
     * Element is associated to a cell (for instance to write an FORCE_ARETE or a FORCE_POUTRE)
     */
    void assignElementsToCells();
    void removeAssertionsMissingDOFS();
    void addDefaultAnalysis();
    void replaceDirectMatrices();
    void removeRedundantSpcs();

public:
    bool finished;
    bool afterValidation = false;
    string name;
    string inputSolverVersion;
    const SolverName inputSolver;
    ModelType modelType;
    string title;
    string description;

    ModelConfiguration configuration;
    std::shared_ptr<Mesh> mesh; /**< Handles geometrical information */

    enum Parameter {
        MASS_OVER_FORCE_MULTIPLIER,
        PRINT_MAXIM,
        LARGE_DISPLACEMENTS
    };
    const LoadSet commonLoadSet;
    const ConstraintSet commonConstraintSet;

private:
    std::unordered_map<LoadSet::Type, map<int, set<std::shared_ptr<Reference<Loading>>> > ,hash<int>>
    loadingReferences_by_loadSet_original_ids_by_loadSet_type;
    std::unordered_map<int, set<std::shared_ptr<Reference<Loading>>> >
    loadingReferences_by_loadSet_ids;

    std::unordered_map< ConstraintSet::Type,
    map<int, set<std::shared_ptr<Reference<Constraint>>>>,hash<int>>
    constraintReferences_by_constraintSet_original_ids_by_constraintSet_type;
    std::map< int, set<std::shared_ptr<Reference<Constraint>>>>
    constraintReferences_by_constraintSet_ids;

    template<class T> class Container final {
        std::map<int, std::shared_ptr<T>> by_id;
        std::unordered_map< typename T::Type, std::map<int, std::shared_ptr<T>>,
        std::hash<int>> by_original_ids_by_type;
    public:
        class iterator;
        friend class iterator;
        class iterator : public std::iterator< std::input_iterator_tag,T,ptrdiff_t> {
            typename map<int,std::shared_ptr<T>>::const_iterator it;
            const map<int,std::shared_ptr<T>>* mp;
        public:
            iterator(const map<int,std::shared_ptr<T>>& mp, const typename map<int,std::shared_ptr<T>>::const_iterator& it) : it(it), mp(&mp) {}
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
                    };
                    iterator begin() const {return iterator(by_id, by_id.begin());}
                    iterator end() const {return iterator(by_id, by_id.end());}
                    int size() const {return (int)by_id.size();}
                    void add(const T&);
                    void add(std::shared_ptr<T> T_ptr);
                    void erase(const Reference<T>);
                    std::shared_ptr<T> find(const Reference<T>&) const;
                    std::shared_ptr<T> find(int) const;
                    bool validate() const;
                };
        std::unordered_map<int,CellContainer> material_assignment_by_material_id;
    public:
        Container<Analysis> analyses;
        Container<Objective> objectives;
        Container<Value> values;
        Container<Loading> loadings;
        Container<LoadSet> loadSets;
        Container<Constraint> constraints;
        Container<ConstraintSet> constraintSets;
        Container<CoordinateSystem> coordinateSystems;
        Container<ElementSet> elementSets;
        Container<Material> materials;
        std::map<Parameter, double> parameters;

        bool onlyMesh;

        Model(string name, string inputSolverVersion = string("UNKNOWN"),
                SolverName inputSolver = NASTRAN,
                const ModelConfiguration configuration = ModelConfiguration());
        //GC: bad bad things happens if you ever try to use this (back references to model are not up to date).
        Model(const Model& that) = delete;
        virtual ~Model();

        /**
         * Add any kind of object to the model.
         */
        //TODO : make and use a template, does not work because of inherited objects
        //template<typename T> void add(const T&);
        void add(const Analysis&);
        void add(const Loading&);
        void add(const LoadSet&);
        void add(const Constraint&);
        void add(const ConstraintSet&);
        void add(const Objective&);
        void add(const Value&);
        void add(const CoordinateSystem&);
        void add(const ElementSet&);
        void add(const Material&);

        /* Get the Id of all elements belonging to set */
        //TODO: make a template, general function?
        const vector<int> getMaterialsId() const;
        const vector<int> getElementSetsId() const;
        /**
         * Remove any kind of object from the model, by giving a reference.
         */
        template<typename T>
        void remove(const Reference<T>);

        /**
         * Retrieve any kind of object from the model, by giving a reference.
         * Return 0 if the object is not found in the model.
         */
        template<typename T>
        const std::shared_ptr<T> find(const Reference<T>) const;

        /**
         * Add a Loading reference into a LoadSet reference.
         */
        void addLoadingIntoLoadSet(const Reference<Loading>&, const Reference<LoadSet>&);

        /**
         * Retrieve all the Loadings corresponding to a given LoadSet.
         */
        const set<std::shared_ptr<Loading>> getLoadingsByLoadSet(const Reference<LoadSet>&) const;

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
        const set<std::shared_ptr<Constraint>> getConstraintsByConstraintSet(const Reference<ConstraintSet>&) const;

        /**
         * Retrieve all the ConstraintSet containing a corresponding Constraint.
         */
        const set<std::shared_ptr<ConstraintSet>> getConstraintSetsByConstraint(const Reference<Constraint>& constraintReference) const;

        /**
         * Retrieve all the ConstraintSet of the model that are at least referenced by one analysis
         */
        const vector<std::shared_ptr<ConstraintSet>> getActiveConstraintSets() const;

        /**
         * Retrieve all the LoadSet of the model that are at least referenced by one analysis
         */
        const vector<std::shared_ptr<LoadSet>> getActiveLoadSets() const;
        /**
         * Retrieve all the ConstraintSet of the model that are common to all analysis
         */
        const vector<std::shared_ptr<ConstraintSet>> getCommonConstraintSets() const;

        /**
         * Retrieve all the LoadSet of the model that are are common to all analysis
         */
        const vector<std::shared_ptr<LoadSet>> getCommonLoadSets() const;
        /**
         * Retrieve all the ConstraintSet of the model that are active but not common to all analysis
         */
        const set<std::shared_ptr<ConstraintSet>> getUncommonConstraintSets() const;
        /**
         * Retrieve all the ConstraintSet of the model that are active but not common to all analysis
         */
        const set<std::shared_ptr<LoadSet>> getUncommonLoadSets() const;

        /**
         * Get a non rigid material (virtual)
         */
        std::shared_ptr<Material> getVirtualMaterial();

        const vector<std::shared_ptr<ElementSet>> filterElements(ElementSet::Type type) const;

        const vector<std::shared_ptr<Beam>> getBeams() const;

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
