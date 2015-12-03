/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * SystusRunner.h
 *
 *  Created on: Nov 8, 2013
 *      Author: devel
 */

#ifndef SYSTUSRUNNER_H_
#define SYSTUSRUNNER_H_

#include "../Abstract/SolverInterfaces.h"

namespace vega {

class SystusRunner: public vega::Runner {
public:
	SystusRunner();
	virtual ExitCode execSolver(const ConfigurationParameters &configuration,
			std::string modelFile);

	virtual ~SystusRunner();
};

} /* namespace vega */
#endif /* SYSTUSRUNNER_H_ */
