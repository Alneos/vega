/*
 * AsterFacade.h
 *
 *  Created on: Jan 8, 2015
 *      Author: devel
 */

#ifndef ASTER_ASTERFACADE_H_
#define ASTER_ASTERFACADE_H_

#include "../Abstract/SolverInterfaces.h"
#include <memory>

/**
 * This class is a Facade to all the files contained into the directory "Aster".
 * This means that class outside the directory should only reference this include
 *
 * This is to improve compilation times and reduce internal dependencies
 * of the project.
 */
namespace vega {
namespace aster {
class AsterWriterImpl;

/**
 * Facade class to the real writer: AsterWriterImpl to break dependency chains
 * and improve recompilation time.
 *
 * It utilises the pimpl pattern (Poiter to Implementation: hide implementation details)
 * http://herbsutter.com/gotw/_100/
 * http://en.wikipedia.org/wiki/Opaque_pointer#C.2B.2B
 */
class AsterWriter: public Writer {
private:
	std::unique_ptr<AsterWriterImpl> pimpl;
	public:
	AsterWriter();
	std::string writeModel(const std::shared_ptr<Model> model_ptr, const ConfigurationParameters&)
			override;
	virtual std::string toString();
	virtual ~AsterWriter();
};

class AsterRunnerImpl;

class AsterRunner: public vega::Runner {
private:
	std::unique_ptr<AsterRunnerImpl> pimpl;
	public:
	AsterRunner();
	virtual Runner::ExitCode execSolver(const ConfigurationParameters &configuration,
			std::string modelFile);

	virtual ~AsterRunner();
};

} /* namespace aster */
} /* namespace vega */

#endif /* ASTER_ASTERFACADE_H_ */
