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
	ResultReadersFacade() = default;
public:
	/**
	 * Returns an empty shared pointer if the result file isn't specified
	 */
	static std::unique_ptr<ResultReader> getResultReader(
			const ConfigurationParameters& configuration);
    ResultReadersFacade(const ResultReadersFacade& that) = delete;

};

}
} /* namespace vega */

#endif /* RESULTREADERS_RESULTREADERSFACADE_H_ */
