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
#if VALGRIND_FOUND && defined VDEBUG && defined __GNUC__  && !defined(_WIN32)
#include <valgrind/memcheck.h>
#endif
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <exception>
#include <boost/algorithm/string.hpp>
#include <ciso646>
#include "../Abstract/Model.h"
#include "../Abstract/ConfigurationParameters.h"

using namespace std;

namespace vega {
namespace result {

using boost::algorithm::trim;
using boost::algorithm::trim_copy;

int F06Parser::readDisplacementSection(int currentSubCase, Model& model,
		const ConfigurationParameters& configuration, ifstream& istream,
		vector<shared_ptr<Assertion>>& assertions, double loadStep) {
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
				shared_ptr<ObjectiveSet> objectiveSet = nullptr;
				if (currentSubCase == NO_SUBCASE) {
                    objectiveSet = model.commonObjectiveSet;
				} else {
                    objectiveSet = model.getOrCreateObjectiveSet(currentSubCase, ObjectiveSet::Type::ASSERTION);
				}
				VectorialValue translation(stod(tokens[2]), stod(tokens[3]), stod(tokens[4]));
				VectorialValue rotation(stod(tokens[5]), stod(tokens[6]), stod(tokens[7]));

				const Node& node = model.mesh.findNode(model.mesh.findNodePosition(nodeId));
				if (node.displacementCS != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
					const auto& coordSystem = model.mesh.getCoordinateSystemByPosition(node.displacementCS);
					coordSystem->updateLocalBase(VectorialValue(node.x, node.y, node.z));
					translation = coordSystem->vectorToGlobal(translation);
					rotation = coordSystem->vectorToGlobal(rotation);
				}
				double values[6] = {
						translation.x(), translation.y(), translation.z(),
						rotation.x(), rotation.y(), rotation.z(),
				};
				for (dof_int i = 0; i < 6; i++) {
					double value = values[i];
					if (abs(value) < 1e-12)
						value = 0.;
					assertions.push_back(make_shared<NodalDisplacementAssertion>(model, objectiveSet, configuration.testTolerance,
									nodeId, DOF::findByPosition(i), value, loadStep));
				}

			}
		}
	} catch (const exception &e) {
		string message("Error ");
		message += string(e.what()) + " parsing:";
		message += configuration.resultFile.string();
		message += " Line number " + to_string(lineNumber);
		message += " Line: " + currentLine;
		cerr << message << endl;
		if (ConfigurationParameters::TranslationMode::MODE_STRICT == configuration.translationMode) {
			throw e;
		} else {
			//cerr << "Conditions not added, parsing next section" << endl;
		}
	}
	return subcase_id /* It could have consumed the next subcase id so we give it back for the next section parsing */;
}

int F06Parser::readEigenvalueSection(int currentSubCase, Model& model,
		const ConfigurationParameters& configuration, ifstream& istream,
		vector<shared_ptr<Assertion>>& assertions) {
	string currentLine;
	int subcase_id = NO_SUBCASE;
	try {
		while (this->readLine(istream, currentLine)) {
			size_t orderPosition = currentLine.find("ORDER");
			if (orderPosition != string::npos) {
				break;
			}
			orderPosition = currentLine.find("AFTER AUGMENTATION OF RESIDUAL VECTORS");
			if (orderPosition != string::npos) {
				//not taking into account RESVEC result
				return subcase_id;
			}
			orderPosition = currentLine.find("ACTUAL MODES USED IN THE DYNAMIC ANALYSIS");
			if (orderPosition != string::npos) {
				//not taking into account modes used in dynamic analysis
				return subcase_id;
			}
		}
		while (this->readLine(istream, currentLine)) {
            size_t subCasePosition = currentLine.find("SUBCASE");
            if (subCasePosition != string::npos) {
				/*
				 * Only used to detect if this section has ended.
				 * Since the line has been consumed, we will return the (next) subcase
				 * LD : All this should simply be replaced by a buffer parser and a "peek" getline
				 */
				subcase_id = parseSubcase(subcase_id, currentLine);
				break;
            }
			istringstream istringLine(currentLine);
			vector<string> tokens;
			copy(istream_iterator<string>(istringLine), istream_iterator<string>(),
					back_inserter(tokens));
			if (tokens.size() != 7)
				break;
			int number = stoi(tokens.at(0));
			double eigenValue = stod(tokens.at(2));
			double cycles = stod(tokens.at(4));
			double generalizedMass = stod(tokens.at(5));
			double generalizedStiffness = stod(tokens.at(6));
			if (abs(cycles) < 1e-12)
				cycles = 0.;
            shared_ptr<ObjectiveSet> objectiveSet = nullptr;
            if (currentSubCase == NO_SUBCASE) {
                    objectiveSet = model.commonObjectiveSet;
            } else {
                    objectiveSet = model.getOrCreateObjectiveSet(currentSubCase, ObjectiveSet::Type::ASSERTION);
            }
			assertions.push_back(
					make_shared<FrequencyAssertion>(model, objectiveSet, number, cycles, eigenValue, generalizedMass, generalizedStiffness, configuration.testTolerance));
		}
	} catch (const exception &e) {
		string message("Error ");
		message += string(e.what()) + " parsing:";
		message += configuration.resultFile.string();
		message += " Line number " + to_string(lineNumber);
		message += " Line: " + currentLine;
		cerr << message << endl;
		if (ConfigurationParameters::TranslationMode::MODE_STRICT == configuration.translationMode) {
			throw e;
		} else {
			//cerr << "Conditions not added, parsing next section" << endl;
		}
	}
	return subcase_id /* It could have consumed the next subcase id so we give it back for the next section parsing */;
}

int F06Parser::readComplexDisplacementSection(int currentSubCase, Model& model,
		const ConfigurationParameters& configuration, ifstream& istream,
		vector<shared_ptr<Assertion>>& assertions, double frequency) {
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
			shared_ptr<ObjectiveSet> objectiveSet = nullptr;
            if (currentSubCase == NO_SUBCASE) {
                objectiveSet = model.commonObjectiveSet;
            } else {
                objectiveSet = model.getOrCreateObjectiveSet(currentSubCase, ObjectiveSet::Type::ASSERTION);
            }
			for (dof_int i = 0; i < 6; i++) {
				double real = stod(tokens[3 + i]);
				if (abs(real) < 1e-12)
					real = 0;
				double imag = stod(tokens[9 + i]);
				if (abs(imag) < 1e-12)
					imag = 0;
				complex<double> value(real, imag);

				assertions.push_back(
						make_shared<NodalComplexDisplacementAssertion>(model, objectiveSet, configuration.testTolerance,
								nodeId, DOF::findByPosition(i), value, frequency));
			}
		}
	} catch (const exception &e) {
		string message("Error ");
		message += string(e.what()) + " parsing:";
		message += configuration.resultFile.string();
		message += " Line number " + to_string(lineNumber);
		message += " Line: " + currentLine;
		cerr << message << endl;
		if (ConfigurationParameters::TranslationMode::MODE_STRICT == configuration.translationMode) {
			throw e;
		} else {
			//cerr << "Conditions not added, parsing next section" << endl;
		}
	}
	return subcase_id /* It could have consumed the next subcase id so we give it back for the next section parsing */;
}

int F06Parser::readStressesForSolidsSection(int currentSubCase, Model& model,
		const ConfigurationParameters& configuration, ifstream& istream,
		vector<shared_ptr<Assertion>>& assertions) {
	string currentLine;
	int subcase_id = NO_SUBCASE;
	try {

        bool foundHeader = false;
		while (this->readLine(istream, currentLine) and currentLine[0] != '1') {
			size_t orderPosition =
					currentLine.find(
							"ELEMENT-ID    GRID-ID        NORMAL              SHEAR             PRINCIPAL       -A-  -B-  -C-     PRESSURE       VON MISES");
			if (orderPosition != string::npos) {
                foundHeader = true;
				break;
			}
		}
		while (foundHeader and this->readLine(istream, currentLine)) {
			size_t subCasePosition = currentLine.find("SUBCASE");
			if (subCasePosition != string::npos) {
                subcase_id = parseSubcase(subcase_id, currentLine);
                if (subcase_id == currentSubCase) {
                    continue;
                } else {
                    break;
                }
			}

			if (currentLine[0] == '0' and currentLine.find("0GRID") == string::npos) {
                throw exception();
			}

			istringstream istringLine(currentLine);
			vector<string> tokens;

			// Element header
			copy(istream_iterator<string>(istringLine), istream_iterator<string>(),
					back_inserter(tokens));
			if (tokens.size() != 6)
				break;
            int cellId = stoi(tokens[1]);
            int nodeNum = stoi(tokens[4]);

            for (int nodePos = 1; nodePos <= nodeNum + 1 /* for CENTER stress */; nodePos++) {
                for (int dir = 1; dir <= 3; dir++) {
                    // PAGE should only happen between nodes
                    this->readLine(istream, currentLine);
                    if (currentLine[0] == '1' and currentLine.find("PAGE") != string::npos) {
                        for (int i = 1; i < 5; i++) {  // skip page header
                            this->readLine(istream, currentLine);
                        }
                        nodePos--;
                        break;
                    }
                    if (currentLine[0] != '0' or currentLine.find("CENTER") != string::npos) {
                        continue; // only first line of this node has node id (and von mises)
                    }
                    istringLine.clear();
                    tokens.clear();
                    istringLine.str(currentLine);
                    copy(istream_iterator<string>(istringLine), istream_iterator<string>(),
                            back_inserter(tokens));
                    int nodeId = stoi(tokens[1]);
                    double vonMises = stod(tokens[tokens.size() - 1] /* sometimes smaller values have (or not) spaces between them, should cut using columns */);
                    shared_ptr<ObjectiveSet> objectiveSet = nullptr;
                    if (currentSubCase == NO_SUBCASE) {
                        objectiveSet = model.commonObjectiveSet;
                    } else {
                        objectiveSet = model.getOrCreateObjectiveSet(currentSubCase, ObjectiveSet::Type::ASSERTION);
                    }
                    assertions.push_back(
                            make_shared<NodalCellVonMisesAssertion>(model, objectiveSet, configuration.testTolerance, cellId,
                                    nodeId, vonMises));
                    if (configuration.outputSolver.getSolverName() == SolverName::CODE_ASTER) {
                        // Workaround to avoid MAILLE in COMM file
                        const auto cellPosition = model.mesh.findCellPosition(cellId);
                        const string& groupName = Cell::MedName(cellPosition);
                        if (not model.mesh.hasGroup(groupName)) {
                            const auto& cellGrp = model.mesh.createCellGroup(groupName, Group::NO_ORIGINAL_ID, "Single cell group over vmis elno assertion");
                            cellGrp->addCellPosition(cellPosition);
                        }
                    }
                }
            }
		}
	} catch (const exception &e) {
		string message("Error ");
		message += string(e.what()) + " parsing:";
		message += configuration.resultFile.string();
		message += " Line number " + to_string(lineNumber);
		message += " Line: " + currentLine;
		cerr << message << endl;
		if (ConfigurationParameters::TranslationMode::MODE_STRICT == configuration.translationMode) {
			throw e;
		} else {
			//cerr << "Conditions not added, parsing next section" << endl;
		}
	}
	return subcase_id /* It could have consumed the next subcase id so we give it back for the next section parsing */;
}

int F06Parser::addAssertionsToModel(int currentSubcase, double loadStep, Model &model,
		const ConfigurationParameters& configuration, ifstream& istream) {

	vector<shared_ptr<Assertion>> assertions;
	int nextSubcase = readDisplacementSection(currentSubcase,model, configuration, istream, assertions, loadStep);
	shared_ptr<Analysis> analysis;
	if (currentSubcase != NO_SUBCASE) {
		analysis = model.analyses.find(currentSubcase);
		analysis->add(Reference<ObjectiveSet>{ObjectiveSet::Type::ASSERTION, currentSubcase});
		if (analysis == nullptr and model.configuration.logLevel >= LogLevel::INFO) {
			cout << "Could not find subcase : " << currentSubcase << " in model." << endl;
		}
	} else if (not model.analyses.empty()) {
		// LD If no subcase indicated, the first one is used if exists.
		analysis = model.analyses.first();
	}
	for (const auto& assertion : assertions) {
		if (analysis != nullptr) {
			model.add(assertion);
			if (model.configuration.logLevel >= LogLevel::TRACE) {
				cout << "Adding NodalDisplacementAssertion : " << *assertion << " to subcase: "
						<< currentSubcase << endl;
			}
		} else if (model.configuration.logLevel >= LogLevel::DEBUG) {
			cout << "Discarding NodalDisplacementAssertion : " << *assertion
					<< " because subcase id: " << currentSubcase << " was not found." << endl;
		}
	}
	return nextSubcase;
}

int F06Parser::addFrequencyAssertionsToModel(int currentSubCase, Model& model,
		const ConfigurationParameters& configuration, ifstream& istream) {
	vector<shared_ptr<Assertion>> assertions;
	int nextSubcase = readEigenvalueSection(currentSubCase, model, configuration, istream, assertions);
	shared_ptr<Analysis> analysis;
	if (currentSubCase != NO_SUBCASE) {
		analysis = model.analyses.find(currentSubCase);
		if (analysis == nullptr) {
            cout << "Cannot find analysis:" << currentSubCase << " in model, assertion are incoherents." << endl;
            return nextSubcase;
		}
		analysis->add(Reference<ObjectiveSet>{ObjectiveSet::Type::ASSERTION, currentSubCase});
		if (analysis == nullptr and model.configuration.logLevel >= LogLevel::INFO) {
			cout << "Could not find subcase : " << currentSubCase << " in model." << endl;
		}
    } else if (model.analyses.empty()) {
        cout << "There is no analysis in model." << endl;
        return nextSubcase;
	} else {
		// LD If no subcase indicated, the first one is used.
		// FIXME: what if the model don't have an analysis and the default one is
		// created inside the finish()? GC
		analysis = model.analyses.first();
	}
	for (const auto& assertion : assertions) {
		if (analysis != nullptr) {
			model.add(assertion);
			if (model.configuration.logLevel >= LogLevel::TRACE) {
				cout << "Adding FrequencyAssertion : " << *assertion << " to subcase: "
						<< currentSubCase << endl;
			}
		} else if (model.configuration.logLevel >= LogLevel::INFO) {
			cout << "Discarding FrequencyAssertion : " << *assertion << " because subcase id: "
					<< currentSubCase << " was not found." << endl;
		}
	}
	return nextSubcase;
}

int F06Parser::addComplexAssertionsToModel(int currentSubCase, double frequency, Model& model,
		const ConfigurationParameters& configuration, ifstream& istream) {
	vector<shared_ptr<Assertion>> assertions;
	int nextSubcase = readComplexDisplacementSection(currentSubCase, model, configuration, istream, assertions,
			frequency);
	shared_ptr<Analysis> analysis;
	if (currentSubCase != NO_SUBCASE) {
		analysis = model.analyses.find(currentSubCase);
		analysis->add(Reference<ObjectiveSet>{ObjectiveSet::Type::ASSERTION, currentSubCase});
		if (analysis == nullptr and model.configuration.logLevel >= LogLevel::INFO) {
			cout << "Could not find subcase : " << currentSubCase << " in model." << endl;
		}
	} else {
		// LD If no subcase indicated, the first one is used.
		// FIXME: what if the model don't have an analysis and the default one is
		// created inside the finish()? GC
		analysis = *model.analyses.begin();
	}
	for (const auto& assertion : assertions) {
		if (analysis != nullptr) {
			model.add(assertion);
			if (model.configuration.logLevel >= LogLevel::TRACE) {
				cout << "Adding Complex Displacement Assertion : " << *assertion << " to subcase: "
						<< currentSubCase << endl;
			}
		} else if (model.configuration.logLevel >= LogLevel::INFO) {
			cout << "Discarding Complex Displacement Assertion : " << *assertion
					<< " because subcase id: " << currentSubCase << " was not found." << endl;
		}
	}
	return nextSubcase;
}

int F06Parser::addVonMisesAssertionsToModel(int currentSubcase, Model &model,
		const ConfigurationParameters& configuration, ifstream& istream) {

	vector<shared_ptr<Assertion>> assertions;
	int nextSubcase = readStressesForSolidsSection(currentSubcase, model, configuration, istream, assertions);
	shared_ptr<Analysis> analysis;
	if (currentSubcase != NO_SUBCASE) {
		analysis = model.analyses.find(currentSubcase);
		analysis->add(Reference<ObjectiveSet>{ObjectiveSet::Type::ASSERTION, currentSubcase});
		if (analysis == nullptr and model.configuration.logLevel >= LogLevel::INFO) {
			cout << "Could not find subcase : " << currentSubcase << " in model." << endl;
		}
	} else if (not model.analyses.empty()) {
		// LD If no subcase indicated, the first one is used if exists.
		analysis = model.analyses.first();
	}
	for (const auto& assertion : assertions) {
		if (analysis != nullptr) {
			model.add(assertion);
			if (model.configuration.logLevel >= LogLevel::TRACE) {
				cout << "Adding VonMisesAssertion : " << *assertion << " to subcase: "
						<< currentSubcase << endl;
			}
		} else if (model.configuration.logLevel >= LogLevel::DEBUG) {
			cout << "Discarding VonMisesAssertion : " << *assertion
					<< " because subcase id: " << currentSubcase << " was not found." << endl;
		}
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
			parsedSubCase = stoi(subcaseN);
		} catch (invalid_argument&) {
			//many different subcase keywords, exception may happen, ignore
		}
	}
	return parsedSubCase;
}

void F06Parser::add_assertions(const ConfigurationParameters& configuration,
		Model& model) {
	if (!configuration.resultFile.empty()) {
		ifstream istream(configuration.resultFile.string());
		string currentLine;
		int currentSubCase = NO_SUBCASE;
		double loadStep = -1;
		double frequency = -1;
		int pointId = -1;
		while (this->readLine(istream, currentLine)) {
			trim(currentLine);
			size_t subCasePosition = currentLine.find("SUBCASE");
			if (subCasePosition != string::npos) {
				string subcaseN = trim_copy(currentLine.substr(subCasePosition + 7));
				//cout << "SUBCASE "<<subcaseN<< "  " << currentLine <<endl;
				currentSubCase = parseSubcase(currentSubCase, currentLine);
				loadStep = -1;
			}
            size_t pointIdPosition = currentLine.find("POINT-ID = ");
			if (pointIdPosition != string::npos) {
				string pointIdString = trim_copy(currentLine.substr(pointIdPosition + 11));
				//cout << "POINT-ID "<<pointId<< "  " << currentLine <<endl;
				istringstream istringLine(pointIdString);
				istringLine >> pointId;
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
				currentSubCase = addAssertionsToModel(currentSubCase, loadStep, model,
						configuration, istream);
			} else if (currentLine == "R E A L   E I G E N V A L U E S") {
				currentSubCase = addFrequencyAssertionsToModel(currentSubCase, model, configuration, istream);
			} else if (currentLine == "C O M P L E X   D I S P L A C E M E N T   V E C T O R") {
				currentSubCase = addComplexAssertionsToModel(currentSubCase, frequency, model,
						configuration, istream);
			}
			size_t stressesPos = currentLine.find("S T R E S S E S");
			size_t solidElementsPos = currentLine.find("S O L I D   E L E M E N T S");
			if (stressesPos != string::npos and solidElementsPos != string::npos) {
				currentSubCase = addVonMisesAssertionsToModel(currentSubCase, model,
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
			if (not trim_copy(line).empty()) {
				lineAvailable = true;
				break;
			}
		}
	}
	return lineAvailable;
}

}
} /* namespace vega */
