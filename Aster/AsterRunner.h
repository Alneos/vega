/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * AsterRunner.h
 *
 *  Created on: Nov 8, 2013
 *      Author: devel
 */

#ifndef ASTERRUNNER_H_
#define ASTERRUNNER_H_

#include "../Abstract/SolverInterfaces.h"

namespace vega {
namespace aster {

class AsterRunner: public Runner {
public:
	AsterRunner();
	ExitCode execSolver(const ConfigurationParameters &configuration,
			std::string modelFile) override;
};

}
} /* namespace vega */
#endif /* ASTERRUNNER_H_ */
