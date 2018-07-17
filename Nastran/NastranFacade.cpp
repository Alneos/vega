/*
 * NastranFacade.cpp
 *
 *  Created on: Jan 8, 2015
 *      Author: devel
 */

#include "NastranFacade.h"

using namespace std;

namespace vega {
namespace nastran {

NastranParser::NastranParser() :
		pimpl(new NastranParserImpl()) {
}

shared_ptr<Model> NastranParser::parse(const ConfigurationParameters& configuration) {
	return pimpl->parse(configuration);
}
NastranParser::~NastranParser() {

}

NastranWriter::NastranWriter() :
		pimpl(new NastranWriterImpl()) {

}

string NastranWriter::toString() {
	return string("NastranWriter");
}

NastranWriter::~NastranWriter() {
}

string NastranWriter::writeModel(const shared_ptr<Model> model_ptr,
		const ConfigurationParameters& configuration)  {
	return pimpl->writeModel(model_ptr, configuration);
}

NastranRunner::NastranRunner() :
		pimpl(new NastranRunnerImpl()) {
}

Runner::ExitCode NastranRunner::execSolver(const ConfigurationParameters& configuration,
		string modelFile) {
	return pimpl->execSolver(configuration, modelFile);
}

NastranRunner::~NastranRunner() {
}

} /* namespace nastran */
} /* namespace vega */

