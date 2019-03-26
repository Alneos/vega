/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * F06Parser.h
 *
 *  Created on: Oct 16, 2013
 *      Author: devel
 */

#ifndef F06PARSER_H_
#define F06PARSER_H_
#include "../Abstract/SolverInterfaces.h"
#include <iostream>

namespace vega {
class Model;

namespace result {
class F06Parser: public vega::ResultReader {
private:
	int lineNumber;
	bool readLine(std::istream &istream, std::string& line);
	int addAssertionsToModel(int currentSubcase, double loadStep, Model &model,
			const ConfigurationParameters&, std::ifstream& istream);
	int addFrequencyAssertionsToModel(int currentSubCase, Model&, const ConfigurationParameters&,
			std::ifstream&);
	int addComplexAssertionsToModel(int currentSubCase, double frequency, Model&,
			const ConfigurationParameters&, std::ifstream&);
	int readDisplacementSection(Model& model, const ConfigurationParameters&,
			std::ifstream& istream, std::vector<std::shared_ptr<Assertion>>& assertions, double loadStep);
	int readEigenvalueSection(Model&, const ConfigurationParameters&, std::ifstream&,
			std::vector<std::shared_ptr<Assertion>>&);
	int readComplexDisplacementSection(Model&, const ConfigurationParameters&, std::ifstream&,
			std::vector<std::shared_ptr<Assertion>>&, double frequency);

	int parseSubcase(int currentSubCase, const std::string& currentLine);
	static const int NO_SUBCASE = -1;

public:
	F06Parser();
	F06Parser(const F06Parser& that) = delete;
	virtual void add_assertions(const ConfigurationParameters& configuration,
			Model& model) override;
	virtual ~F06Parser();
};

}
} /* namespace vega */
#endif /* F06PARSER_H_ */
