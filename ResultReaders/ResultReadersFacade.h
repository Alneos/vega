/*
 * ResultReadersFacade.h
 *
 *  Created on: Jan 20, 2015
 *      Author: devel
 */

#ifndef RESULTREADERS_RESULTREADERSFACADE_H_
#define RESULTREADERS_RESULTREADERSFACADE_H_

#include "../Abstract/SolverInterfaces.h"
#include <vector>
#include <memory>

namespace vega {
namespace result {

class ResultReadersFacade {
private:
	ResultReadersFacade();
public:
	/**
	 * Returns an empty shared pointer if the result file isn't specified
	 */
	static std::shared_ptr<ResultReader> getResultReader(
			const ConfigurationParameters& configuration);

};

}
} /* namespace vega */

#endif /* RESULTREADERS_RESULTREADERSFACADE_H_ */
