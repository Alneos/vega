/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * F06Parser.cpp
 *
 *  Created on: Oct 16, 2013
 *      Author: devel
 */

#include "F06Parser.h"

//#include <bits/basic_string.h>
#if defined VDEBUG && defined __GNUC__
#include <valgrind/memcheck.h>
#endif
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <exception>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <ciso646>
#include "../Abstract/Model.h"
#include "../Abstract/ConfigurationParameters.h"

using namespace std;

namespace vega {
namespace result {

using boost::algorithm::trim;
using boost::algorithm::trim_copy;
using boost::lexical_cast;

F06Parser::F06Parser() {
	lineNumber = 0;
}

int F06Parser::readDisplacementSection(const Model& model,
		const ConfigurationParameters& configuration, ifstream& istream,
		vector<Assertion*>& assertions, double loadStep) {
	string header;
	string currentLine;
	int subcase_id = NO_SUBCASE;
	//skip header line
	this->readLine(istream, header);
	try {
		while (this->readLine(istream, currentLine)) {
			size_t subCasePosition = currentLine.find("SUBCASE");
			if (currentLine.find("DIAGNOSTIC TOOLS") != string::npos) {
				//skip
			} else if (subCasePosition != string::npos) {
				/*
				 * Only used to detect if this section has ended.
				 * Since the line has been consumed, we will return the (next) subcase
				 * LD : All this should simply be replaced by a buffer parser and a "peek" getline
				 */
				subcase_id = parseSubcase(subcase_id, currentLine);
				break;
			} else if (!isspace(currentLine.at(0))) {
				//stop parsing the section at the first line that don't start with a space
				break;
			} else {
				istringstream istringLine(currentLine);
				vector<string> tokens;
				copy(istream_iterator<string>(istringLine), istream_iterator<string>(),
						back_inserter(tokens));
				if (tokens.size() != 8)
					break;

				int nodeId = stoi(tokens[0]);
				if (tokens[1] != "G"){
					//cerr << "unsupported assertion type " << assertionType
					//		<< " : line n." << lineNumber << " " << currentLine
					//		<< endl;
					continue;
				}
				VectorialValue translation(stod(tokens[2]), stod(tokens[3]), stod(tokens[4]));
				VectorialValue rotation(stod(tokens[5]), stod(tokens[6]), stod(tokens[7]));

				int nodePosition = model.mesh->findNodePosition(nodeId);
				Node node = model.mesh->findNode(nodePosition);
				if (node.displacementCS != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
					Node node = model.mesh->findNode(nodePosition);
					shared_ptr<CoordinateSystem> coordSystem = model.find(
							Reference<CoordinateSystem>(CoordinateSystem::UNKNOWN,
									node.displacementCS));
					coordSystem->updateLocalBase(VectorialValue(node.x, node.y, node.z));
					translation = coordSystem->vectorToGlobal(translation);
					rotation = coordSystem->vectorToGlobal(rotation);
				}
				double values[6] = {
						translation.x(), translation.y(), translation.z(),
						rotation.x(), rotation.y(), rotation.z(),
				};
				for (int i = 0; i < 6; i++) {
					double value = values[i];
					if (abs(value) < 1e-12)
						value = 0.;
					assertions.push_back(new NodalDisplacementAssertion(model, configuration.testTolerance,
									nodeId, DOF::findByPosition(i), value, loadStep));
				}

			}
		}
	} catch (const exception &e) {
		string message("Error ");
		message += string(e.what()) + " parsing:";
		message += configuration.resultFile.string();
		message += " Line number " + lexical_cast<string>(currentLine);
		message += " Line: " + currentLine;
		cerr << message << endl;
		if (ConfigurationParameters::MODE_STRICT == configuration.translationMode) {
			throw e;
		} else {
			//cerr << "Conditions not added, parsing next section" << endl;
		}
	}
	return subcase_id /* It could have consumed the next subcase id so we give it back for the next section parsing */;
}

void F06Parser::readEigenvalueSection(const Model& model,
		const ConfigurationParameters& configuration, ifstream& istream,
		vector<Assertion*>& assertions) {
	string currentLine;
	try {
		while (this->readLine(istream, currentLine)) {
			size_t orderPosition = currentLine.find("ORDER");
			if (orderPosition != string::npos) {
				break;
			}
			orderPosition = currentLine.find("AFTER AUGMENTATION OF RESIDUAL VECTORS");
			if (orderPosition != string::npos) {
				//not taking into account RESVEC result
				return;
			}
			orderPosition = currentLine.find("ACTUAL MODES USED IN THE DYNAMIC ANALYSIS");
			if (orderPosition != string::npos) {
				//not taking into account modes used in dynamic analysis
				return;
			}
		}
		while (this->readLine(istream, currentLine)) {
			istringstream istringLine(currentLine);
			vector<string> tokens;
			copy(istream_iterator<string>(istringLine), istream_iterator<string>(),
					back_inserter(tokens));
			if (tokens.size() != 7)
				break;
			int number = stoi(tokens.at(0));
			double value = stod(tokens.at(4));
			if (abs(value) < 1e-12)
				value = 0.;
			assertions.push_back(
					new FrequencyAssertion(model, number, value, configuration.testTolerance));
		}
	} catch (const exception &e) {
		string message("Error ");
		message += string(e.what()) + " parsing:";
		message += configuration.resultFile.string();
		message += " Line number " + lexical_cast<string>(currentLine);
		message += " Line: " + currentLine;
		cerr << message << endl;
		if (ConfigurationParameters::MODE_STRICT == configuration.translationMode) {
			throw e;
		} else {
			//cerr << "Conditions not added, parsing next section" << endl;
		}
	}
}

int F06Parser::readComplexDisplacementSection(const Model& model,
		const ConfigurationParameters& configuration, ifstream& istream,
		vector<Assertion*>& assertions, double frequency) {
	string currentLine;
	int subcase_id = NO_SUBCASE;
	try {

		while (this->readLine(istream, currentLine)) {
			size_t orderPosition =
					currentLine.find(
							"POINT ID.   TYPE          T1             T2             T3             R1             R2             R3");
			if (orderPosition != string::npos)
				break;
		}
		while (this->readLine(istream, currentLine)) {
			size_t subCasePosition = currentLine.find("SUBCASE");
			if (subCasePosition != string::npos) {
				subcase_id = parseSubcase(subcase_id, currentLine);
				break;
			}

			istringstream istringLine(currentLine);
			vector<string> tokens;
			copy(istream_iterator<string>(istringLine), istream_iterator<string>(),
					back_inserter(tokens));
			if (tokens.size() != 9)
				break;

			this->readLine(istream, currentLine);
			istringLine.clear();
			istringLine.str(currentLine);
			copy(istream_iterator<string>(istringLine), istream_iterator<string>(),
					back_inserter(tokens));
			if (tokens.size() != 15)
				throw exception();

			int nodeId = stoi(tokens[1]);

			for (int i = 0; i < 6; i++) {
				double real = stod(tokens[3 + i]);
				if (abs(real) < 1e-12)
					real = 0;
				double imag = stod(tokens[9 + i]);
				if (abs(imag) < 1e-12)
					imag = 0;
				complex<double> value(real, imag);

				assertions.push_back(
						new NodalComplexDisplacementAssertion(model, configuration.testTolerance,
								nodeId, DOF::findByPosition(i), value, frequency));
			}
		}
	} catch (const exception &e) {
		string message("Error ");
		message += string(e.what()) + " parsing:";
		message += configuration.resultFile.string();
		message += " Line number " + lexical_cast<string>(currentLine);
		message += " Line: " + currentLine;
		cerr << message << endl;
		if (ConfigurationParameters::MODE_STRICT == configuration.translationMode) {
			throw e;
		} else {
			//cerr << "Conditions not added, parsing next section" << endl;
		}
	}
	return subcase_id /* It could have consumed the next subcase id so we give it back for the next section parsing */;
}

int F06Parser::addAssertionsToModel(int currentSubcase, double loadStep, Model &model,
		const ConfigurationParameters& configuration, ifstream& istream) {

	vector<Assertion*> assertions;
	int nextSubcase = readDisplacementSection(model, configuration, istream, assertions, loadStep);
	shared_ptr<Analysis> analysis;
	if (currentSubcase != NO_SUBCASE) {
		analysis = model.analyses.find(currentSubcase);
		if (analysis == nullptr and model.configuration.logLevel >= LogLevel::INFO) {
			cout << "Could not find subcase : " << currentSubcase << " in model." << endl;
		}
	} else {
		// LD If no subcase indicated, the first one is used.
		// FIXME: what if the model don't have an analysis and the default one is
		// created inside the finish()? GC
		analysis = *model.analyses.begin();
	}
	for (Assertion* assertion : assertions) {
		if (analysis != nullptr) {
			model.add(*assertion);
			analysis->add(assertion->getReference());
			if (model.configuration.logLevel >= LogLevel::TRACE) {
				cout << "Adding NodalDisplacementAssertion : " << *assertion << " to subcase: "
						<< currentSubcase << endl;
			}
		} else if (model.configuration.logLevel >= LogLevel::DEBUG) {
			cout << "Discarding NodalDisplacementAssertion : " << *assertion
					<< " because subcase id: " << currentSubcase << " was not found." << endl;
		}
		delete (assertion);
	}
	return nextSubcase;
}

void F06Parser::addFrequencyAssertionsToModel(int currentSubCase, Model& model,
		const ConfigurationParameters& configuration, ifstream& istream) {
	vector<Assertion*> assertions;
	readEigenvalueSection(model, configuration, istream, assertions);
	shared_ptr<Analysis> analysis;
	if (currentSubCase != NO_SUBCASE) {
		analysis = model.analyses.find(currentSubCase);
		if (analysis == nullptr and model.configuration.logLevel >= LogLevel::INFO) {
			cout << "Could not find subcase : " << currentSubCase << " in model." << endl;
		}
	} else {
		// LD If no subcase indicated, the first one is used.
		// FIXME: what if the model don't have an analysis and the default one is
		// created inside the finish()? GC
		analysis = *model.analyses.begin();
	}
	for (Assertion* assertion : assertions) {
		if (analysis != nullptr) {
			model.add(*assertion);
			analysis->add(Reference<Objective>(*assertion));
			if (model.configuration.logLevel >= LogLevel::TRACE) {
				cout << "Adding FrequencyAssertion : " << *assertion << " to subcase: "
						<< currentSubCase << endl;
			}
		} else if (model.configuration.logLevel >= LogLevel::INFO) {
			cout << "Discarding FrequencyAssertion : " << *assertion << " because subcase id: "
					<< currentSubCase << " was not found." << endl;
		}
		delete (assertion);
	}
}

int F06Parser::addComplexAssertionsToModel(int currentSubCase, double frequency, Model& model,
		const ConfigurationParameters& configuration, ifstream& istream) {
	vector<Assertion*> assertions;
	int nextSubcase = readComplexDisplacementSection(model, configuration, istream, assertions,
			frequency);
	shared_ptr<Analysis> analysis;
	if (currentSubCase != NO_SUBCASE) {
		analysis = model.analyses.find(currentSubCase);
		if (analysis == nullptr and model.configuration.logLevel >= LogLevel::INFO) {
			cout << "Could not find subcase : " << currentSubCase << " in model." << endl;
		}
	} else {
		// LD If no subcase indicated, the first one is used.
		// FIXME: what if the model don't have an analysis and the default one is
		// created inside the finish()? GC
		analysis = *model.analyses.begin();
	}
	for (Assertion* assertion : assertions) {
		if (analysis != nullptr) {
			model.add(*assertion);
			analysis->add(Reference<Objective>(*assertion));
			if (model.configuration.logLevel >= LogLevel::TRACE) {
				cout << "Adding Complex Displacement Assertion : " << *assertion << " to subcase: "
						<< currentSubCase << endl;
			}
		} else if (model.configuration.logLevel >= LogLevel::INFO) {
			cout << "Discarding Complex Displacement Assertion : " << *assertion
					<< " because subcase id: " << currentSubCase << " was not found." << endl;
		}
		delete (assertion);
	}
	return nextSubcase;
}

int F06Parser::parseSubcase(int currentSubCase, const string& currentLine) {
	size_t subCasePosition = currentLine.find("SUBCASE");
	string subcaseN = trim_copy(currentLine.substr(subCasePosition + 7));
	bool has_digits_or_space = (subcaseN.find_first_not_of("0123456789 ") == string::npos);
	int parsedSubCase = currentSubCase;
	if (has_digits_or_space) {
		try {
			parsedSubCase = atoi(subcaseN.c_str());
		} catch (invalid_argument&) {
			//many different subcase keywords, exception may happen, ignore
		}
	}
	return parsedSubCase;
}

void F06Parser::add_assertions(const ConfigurationParameters& configuration,
		shared_ptr<Model> model) {
	if (!configuration.resultFile.empty()) {
		ifstream istream(configuration.resultFile.string());
		string currentLine;
		int currentSubCase = NO_SUBCASE;
		double loadStep = -1;
		double frequency = -1;
		while (this->readLine(istream, currentLine)) {
			trim(currentLine);
			size_t subCasePosition = currentLine.find("SUBCASE");
			if (subCasePosition != string::npos) {
				string subcaseN = trim_copy(currentLine.substr(subCasePosition + 7));
				//cout << "SUBCASE "<<subcaseN<< "  " << currentLine <<endl;
				currentSubCase = parseSubcase(currentSubCase, currentLine);
				loadStep = -1;
			}
			size_t loadStepPosition = currentLine.find("LOAD STEP = ");
			if (loadStepPosition != string::npos) {
				string loadStepString = trim_copy(currentLine.substr(loadStepPosition + 12));
				istringstream istringLine(loadStepString);
				istringLine >> loadStep;
			}
			size_t frequencyPosition = currentLine.find("FREQUENCY = ");
			if (frequencyPosition != string::npos) {
				string frequencyString = trim_copy(currentLine.substr(frequencyPosition + 12));
				istringstream istringLine(frequencyString);
				istringLine >> frequency;
			}
			if (currentLine == "D I S P L A C E M E N T   V E C T O R") {
				/*
				 * The next subcase can be consumed to understand if the section has ended.
				 * In this case, it will be given back as return value, to be used for the next section.
				 */
				currentSubCase = addAssertionsToModel(currentSubCase, loadStep, *model,
						configuration, istream);
			} else if (currentLine == "R E A L   E I G E N V A L U E S") {
				addFrequencyAssertionsToModel(currentSubCase, *model, configuration, istream);
			} else if (currentLine == "C O M P L E X   D I S P L A C E M E N T   V E C T O R") {
				currentSubCase = addComplexAssertionsToModel(currentSubCase, frequency, *model,
						configuration, istream);
			}
		}
		istream.close();
	}
}

F06Parser::~F06Parser() {

}

bool F06Parser::readLine(istream &istream, string& line) {
	bool lineAvailable = false;
	while (getline(istream, line)) {
		lineNumber += 1;
		if (!line.empty()) {
			if (trim_copy(line).size() > 0) {
				lineAvailable = true;
				break;
			}
		}
	}
	return lineAvailable;
}

}
} /* namespace vega */
