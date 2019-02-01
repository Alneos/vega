/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * NastranRunner.h
 *
 *  Created on: Nov 8, 2013
 *      Author: devel
 */

#ifndef NASTRANRUNNER_H
#define NASTRANRUNNER_H

#include "../Abstract/SolverInterfaces.h"
#include "../Abstract/Utility.h"

namespace vega {
namespace nastran {

class NastranRunner: public vega::Runner {
public:
    NastranRunner() = default;
	NastranRunner(const NastranRunner& that) = delete;
	ExitCode execSolver(const ConfigurationParameters &configuration,
			std::string modelFile) override;

	virtual ~NastranRunner();
};

} /* namespace nastran */
} /* namespace vega */
#endif /* NASTRANRUNNER_H */
