/*
 * AsterFacade.cpp
 *
 *  Created on: Jan 8, 2015
 *      Author: devel
 */

#include "AsterFacade.h"

#include "AsterWriter.h"
#include "AsterRunner.h"

namespace vega {
namespace aster {

using namespace std;

AsterWriter::AsterWriter() :
		pimpl(new AsterWriterImpl()) {

}

string AsterWriter::toString() {
	return string("AsterWriter");
}

AsterWriter::~AsterWriter() {
}

string AsterWriter::writeModel(const shared_ptr<Model> model_ptr,
		const ConfigurationParameters& configuration)  {
	return pimpl->writeModel(model_ptr, configuration);
}

AsterRunner::AsterRunner() :
		pimpl(new AsterRunnerImpl()) {
}

Runner::ExitCode AsterRunner::execSolver(const ConfigurationParameters& configuration,
		string modelFile) {
	return pimpl->execSolver(configuration, modelFile);
}

AsterRunner::~AsterRunner() {
}

} /* namespace aster */
} /* namespace vega */

