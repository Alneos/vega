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
 * Analysis.h
 *
 *  Created on: Sep 4, 2013
 *      Author: devel
 */

#ifndef ANALYSIS_H_
#define ANALYSIS_H_

#include <climits>

#include <map>
#include <set>
#include <utility>
#include <list>
#include <set>
#include "MeshComponents.h"
#include "Loading.h"
#include "Constraint.h"
#include "Objective.h"
#include "Value.h"
#include "Object.h"

namespace vega {

class Model;
class ModelConfiguration;

/**
 * Responsible of keeping options to use in a finite element calculation
 */
class Analysis: public Identifiable<Analysis> {
private:
    friend std::ostream &operator<<(std::ostream &out, const Analysis& analysis);    //output
    std::map<int, char> boundaryDOFSByNodePosition;
    const std::string label;         /**< User defined label for this instance of Analysis. **/
public:
    enum class Type {
        COMBINATION,
        LINEAR_MECA_STAT,
        LINEAR_BUCKLING,
        LINEAR_MODAL,
        LINEAR_DYNA_DIRECT_FREQ,
        LINEAR_DYNA_MODAL_FREQ,
        NONLINEAR_MECA_STAT,
        UNKNOWN,
    };
protected:
    std::set<Reference<LoadSet>>loadSet_references;
    std::set<Reference<ConstraintSet>> constraintSet_references;
    std::set<Reference<Objective>> objectiveReferences;
    Analysis(Model& model, const Type Type, const std::string original_label = "", const int original_id = NO_ORIGINAL_ID);
    Analysis(const Analysis& that) = delete;

public:
    virtual ~Analysis() = default;
    Model& model;
    const Type type;
    static const std::string name;     /**< String conversion of the object, i.e "Analysis" **/
    static const std::map<Type,std::string> stringByType;
    std::shared_ptr<Analysis> previousAnalysis = nullptr;

    void add(const Reference<LoadSet>&);
    void add(const Reference<ConstraintSet>&);
    void add(const Reference<Objective>&);

    bool contains(const Reference<LoadSet>) const noexcept;
    bool contains(const Reference<ConstraintSet>) const noexcept;
    bool contains(const Reference<Objective>) const noexcept;

    bool contains(const LoadSet::Type) const noexcept;
    bool contains(const ConstraintSet::Type) const noexcept;
    bool contains(const Objective::Type) const noexcept;

    void remove(const Reference<LoadSet>);
    void remove(const Reference<ConstraintSet>);
    void remove(const Reference<Objective>);

    virtual bool canReuse(const std::shared_ptr<Analysis>&) const noexcept {
        return false;
    }

    std::string getLabel() const noexcept {return label;}  /**< Getter for the label variable **/

    /**
     * retrieve the ConstraintSets specific to this analysis
     * plus eventually the common ConstraintSet of all analyzes of the model
     */
    std::vector<std::shared_ptr<ConstraintSet>> getConstraintSets() const noexcept;

    /**
     * retrieve the LoadSets specific to this analysis
     * plus eventually the common LoadSet of all analyzes of the model
     */
    std::vector<std::shared_ptr<LoadSet>> getLoadSets() const noexcept;
    std::vector<std::shared_ptr<BoundaryCondition>> getBoundaryConditions() const noexcept;
    std::vector<std::shared_ptr<Assertion>> getAssertions() const noexcept;
    std::vector<std::shared_ptr<Objective>> getObjectives() const noexcept;

    /**
     * Return true if the analysis has at least one SPC (or equivalent SPCD, MPCD, etc)
     * ConstraintSet or Constraint. Check both common and specific ConstraintSet.
     */
    bool hasSPC() const noexcept;

    void removeSPCNodeDofs(SinglePointConstraint& spc, int nodePosition, const DOFS dofs);
    void addBoundaryDOFS(int nodePosition, const DOFS dofs);
    DOFS findBoundaryDOFS(int nodePosition) const;
    std::set<int> boundaryNodePositions() const;
    void copyInto(Analysis& other) const;

    virtual bool isStatic() const noexcept {
        return false;
    }
    virtual bool isLinear() const noexcept {
        return false;
    }
    virtual bool isModal() const noexcept {
        return false;
    }
    virtual bool isBuckling() const noexcept {
        return false;
    }
    bool validate() const override;

    std::map<std::string, std::string> to_map() const noexcept override;
    /**
     * Create a connection graph to find and block free mechanisms
     * See https://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19950020950.pdf
     * A Verification Procedure for MSC/NASTRAN Finite Element Models
     * Alan E. Stockweil
     *
     * 3.2.4 Mechanisms
     * Mechanisms lead to stiffness matrix singularities involving two or more grid points. An
     * example of a mechanism is a section of a structure that is capable of rigid-body motion in
     * one or more directions. MSC/NASTRAN automatically checks for mechanisms every time
     * it performs a decomposition using the DCMP module
     */
    void createGraph(std::ostream& dot_ofs);
};

class Combination: public Analysis {
public:
    Combination(Model& model, const std::string original_label = "", const int original_id = NO_ORIGINAL_ID);
    std::map<Reference<Analysis>,double> coefByAnalysis;
    bool isStatic() const noexcept override {
        return true;
    }
    bool isLinear() const noexcept override {
        return true;
    }
};

class LinearMecaStat: public Analysis {
public:
    LinearMecaStat(Model& model, const std::string original_label = "", const int original_id = NO_ORIGINAL_ID);
    bool isStatic() const noexcept override {
        return true;
    }
    bool isLinear() const noexcept override {
        return true;
    }
};

class NonLinearMecaStat: public Analysis {
public:
    Reference<Objective> strategy_reference;
    NonLinearMecaStat(Model& model, const NonLinearStrategy& strategy, const std::string original_label = "", const int original_id = NO_ORIGINAL_ID);
    NonLinearMecaStat(Model& model, const int strategy_original_id, const std::string original_label = "", const int original_id = NO_ORIGINAL_ID);
    bool isStatic() const noexcept override {
        return true;
    }
    bool validate() const override;
};

class LinearModal: public Analysis {
protected:
    Reference<Objective> frequencySearchRef;
public:
    LinearModal(Model&, const Reference<Objective>&, const std::string original_label = "", const int original_id = NO_ORIGINAL_ID, const Type type = Type::LINEAR_MODAL);
    LinearModal(Model&, const int frequency_band_original_id, const std::string original_label = "", const int original_id = NO_ORIGINAL_ID, const Type type = Type::LINEAR_MODAL);
    std::shared_ptr<FrequencySearch> getFrequencySearch() const noexcept;
    bool use_power_iteration = false;
    bool validate() const override;
    bool isLinear() const noexcept override {
        return true;
    }
    bool isModal() const noexcept override {
        return true;
    }
};

class LinearBuckling: public LinearModal {
public:
    LinearBuckling(Model&, const Reference<Objective>&, const std::string original_label = "", const int original_id = NO_ORIGINAL_ID);
    bool isBuckling() const noexcept override {
        return true;
    }
};

/**
 * Modal frequency response analysis is an alternate method to compute frequency response.
 * This method uses the mode shapes of the structure to uncouple the equations of motion
 * (when no damping or only modal damping is used) and, depending on the number of modes computed and retained,
 * reduce the problem size.
 * Because modal frequency response analysis uses the mode shapes of a structure, modal frequency response analysis is a natural extension of normal modes analysis.
 */
class LinearDynaModalFreq: public LinearModal {
protected:
    Reference<Objective> modal_damping_reference;
    Reference<Objective> frequencyExcitationRef;
public:
    LinearDynaModalFreq(Model& model, const int frequency_band_original_id,
            const int modal_damping_original_id, const int frequency_value_original_id,
            const bool residual_vector = false,
            const std::string original_label = "", const int original_id = NO_ORIGINAL_ID);
    const bool residual_vector;
    std::shared_ptr<ModalDamping> getModalDamping() const;
    std::shared_ptr<FrequencyExcit> getExcitationFrequencies() const;
    bool canReuse(const std::shared_ptr<Analysis>&) const noexcept override;
    bool validate() const override;
};

/**
 * The direct frequency response method solves the coupled equations of motion in terms of forcing frequency.
 * In direct frequency response analysis, structural response is computed at discrete excitation frequencies by solving a set of coupled matrix equations using complex algebra.
 */
class LinearDynaDirectFreq: public Analysis {
protected:
    Reference<Objective> frequencyExcitationRef;
public:
    LinearDynaDirectFreq(Model& model,
            const int frequency_value_original_id,
            const std::string original_label = "", const int original_id = NO_ORIGINAL_ID);
    std::shared_ptr<FrequencyExcit> getExcitationFrequencies() const;
    bool validate() const override;
};

} /* namespace vega */
#endif /* ANALYSIS_H_ */

