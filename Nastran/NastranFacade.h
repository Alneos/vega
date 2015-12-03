/*
 * AsterFacade.h
 *
 *  Created on: Jan 8, 2015
 *      Author: devel
 */

#ifndef NASTRANFACADE_H_
#define NASTRANFACADE_H_

#include "../Abstract/SolverInterfaces.h"
#include "NastranRunner.h"
#include "NastranWriter.h"
#include "NastranParser.h"
#include <memory>

/**
 * This class is a Facade to all the files contained into the directory "Nastran".
 * This means that class outside the directory should only reference this include
 *
 * This is to improve compilation times and reduce internal dependencies
 * of the project.
 */
namespace vega {
namespace nastran {
class NastranParserImpl;

/**
 * Facade class to the real writer: NastranParserImpl to break dependency chains
 * and improve recompilation time.
 *
 * It utilises the pimpl pattern (Poiter to Implementation: hide implementation details)
 * http://herbsutter.com/gotw/_100/
 * http://en.wikipedia.org/wiki/Opaque_pointer#C.2B.2B
 */

class NastranParserImpl;

class NastranParser: public Parser {
private:
	std::unique_ptr<NastranParserImpl> pimpl;
	public:
	NastranParser();
	std::shared_ptr<Model> parse(const ConfigurationParameters& configuration) override;
	virtual ~NastranParser();
};

class NastranWriterImpl;

class NastranWriter: public Writer {
private:
	std::unique_ptr<NastranWriterImpl> pimpl;
	public:
	NastranWriter();
	std::string writeModel(const std::shared_ptr<Model> model_ptr, const ConfigurationParameters&)
			override;
	virtual std::string toString();
	virtual ~NastranWriter();
};

class NastranRunnerImpl;

class NastranRunner: public vega::Runner {
private:
	std::unique_ptr<NastranRunnerImpl> pimpl;
	public:
	NastranRunner();
	virtual Runner::ExitCode execSolver(const ConfigurationParameters &configuration,
			std::string modelFile);

	virtual ~NastranRunner();
};

} /* namespace aster */
} /* namespace vega */

#endif /* NASTRANFACADE_H_ */
