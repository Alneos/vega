/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * BoundaryCondition.h
 *
 *  Created on: 20 févr. 2013
 *      Author: dallolio
 */

#ifndef BOUNDARYCONDITION_H_
#define BOUNDARYCONDITION_H_

#include <cfloat>
#include "Dof.h"
#include "Utility.h"
#include <unordered_map>
#include <set>

namespace vega {

class BoundaryCondition {
public:
	BoundaryCondition();
	virtual bool ineffective() const {
		throw logic_error("boundary condition ineffective() used but not implemented");
	}
    virtual bool hasFunctions() const {
	    return false;
	}
	virtual const DOFS getDOFSForNode(int nodePosition) const = 0;
	virtual std::set<int> nodePositions() const = 0;
	virtual ~BoundaryCondition();
};

} /* namespace vega */

#endif /* BOUNDARYCONDITION_H_ */
