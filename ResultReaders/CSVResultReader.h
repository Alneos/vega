/*
 * CSVAssertionParser.h
 *
 *  Created on: Jan 14, 2015
 *      Author: devel
 */

#ifndef COMMANDLINE_CSVASSERTIONPARSER_H_
#define COMMANDLINE_CSVASSERTIONPARSER_H_

#include "../Abstract/SolverInterfaces.h"
#include <vector>
#include <memory>

namespace vega {

class ConfigurationParameters;
class Assertion;

namespace result {

class CSVResultReader: public vega::ResultReader {
public:
	CSVResultReader();
	virtual void add_assertions(const ConfigurationParameters& configuration,
			std::shared_ptr<Model> model) override;
	virtual ~CSVResultReader();
};

}
} /* namespace vega */

#endif /* COMMANDLINE_CSVASSERTIONPARSER_H_ */
