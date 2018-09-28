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
    enum Type {
        LINEAR_MECA_STAT,
        LINEAR_MODAL,
        LINEAR_DYNA_MODAL_FREQ,
        NONLINEAR_MECA_STAT,
        UNKNOWN,
    };
protected:
    std::list<std::shared_ptr<Reference<LoadSet>>>loadSet_references;
    std::list<std::shared_ptr<Reference<ConstraintSet>>> constraintSet_references;
    std::list<std::shared_ptr<Reference<Objective>>> objectiveReferences;

public:
    Model& model;
    const Type type;
    static const std::string name;     /**< String conversion of the object, i.e "Analysis" **/
    static const std::map<Type,std::string> stringByType;
    std::shared_ptr<Analysis> previousAnalysis;

    Analysis(Model& model, const Type Type, const std::string original_label = "", const int original_id = NO_ORIGINAL_ID);

    void add(const Reference<LoadSet>&);
    void add(const Reference<ConstraintSet>&);
    void add(const Reference<Objective>&);

    bool contains(const Reference<LoadSet>) const;
    bool contains(const Reference<ConstraintSet>) const;
    bool contains(const Reference<Objective>) const;

    void remove(const Reference<LoadSet>);
    void remove(const Reference<ConstraintSet>);
    void remove(const Reference<Objective>);

    const std::string getLabel() const {return label;}  /**< Getter for the label variable **/

    /**
     * retrieve the ConstraintSets specific to this analysis
     * plus eventually the common ConstraintSet of all analyzes of the model
     */
    const std::vector<std::shared_ptr<ConstraintSet>> getConstraintSets() const;

    /**
     * retrieve the LoadSets specific to this analysis
     * plus eventually the common LoadSet of all analyzes of the model
     */
    const std::vector<std::shared_ptr<LoadSet>> getLoadSets() const;
    const std::vector<std::shared_ptr<BoundaryCondition>> getBoundaryConditions() const;
    const std::vector<std::shared_ptr<Assertion>> getAssertions() const;


    /**
     * Return true if the analysis has at least one SPC (or equivalent SPCD, MPCD, etc)
     * ConstraintSet or Constraint. Check both common and specific ConstraintSet.
     */
    bool hasSPC() const;

    void removeSPCNodeDofs(SinglePointConstraint& spc, int nodePosition, const DOFS dofs);
    void addBoundaryDOFS(int nodePosition, const DOFS dofs);
    const DOFS findBoundaryDOFS(int nodePosition) const;
    const std::set<int> boundaryNodePositions() const;

    virtual std::shared_ptr<Analysis> clone() const =0;
    bool validate() const override;
    std::map<std::string, std::string> to_map() const;

    virtual ~Analysis();

};

class LinearMecaStat: public Analysis {
public:
    LinearMecaStat(Model& model, const std::string original_label = "", const int original_id = NO_ORIGINAL_ID);
    std::shared_ptr<Analysis> clone() const;
};

class NonLinearMecaStat: public Analysis {
public:
    Reference<Objective> strategy_reference;
    NonLinearMecaStat(Model& model, const NonLinearStrategy& strategy, const std::string original_label = "",
            const int original_id = NO_ORIGINAL_ID);
    NonLinearMecaStat(Model& model, const int strategy_original_id, const std::string original_label = "",
            const int original_id = NO_ORIGINAL_ID);
    std::shared_ptr<Analysis> clone() const;
    bool validate() const override;
};

class LinearModal: public Analysis {
protected:
    Reference<Objective> frequencySearchRef;
public:
    LinearModal(Model&, const FrequencyTarget&, const std::string original_label = "",
            const int original_id = NO_ORIGINAL_ID, const Type type = Type::LINEAR_MODAL);
    LinearModal(Model&, const int frequency_band_original_id, const std::string original_label = "",
            const int original_id = NO_ORIGINAL_ID, const Type type = Type::LINEAR_MODAL);
    std::shared_ptr<FrequencyTarget> getFrequencySearch() const;
    std::shared_ptr<Analysis> clone() const;
    bool use_direct_solver = false;
    bool validate() const override;
};

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
    std::shared_ptr<FrequencyTarget> getExcitationFrequencies() const;
    std::shared_ptr<Analysis> clone() const;
    bool validate() const override;
};

} /* namespace vega */
#endif /* ANALYSIS_H_ */

