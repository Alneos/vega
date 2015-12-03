/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * InputOutput.h
 *
 *  Created on: Oct 30, 2013
 *      Author: devel
 */

#ifndef INPUTOUTPUT_H_
#define INPUTOUTPUT_H_

#include <memory>
#include <climits>
#include <iostream>
#include <vector>

namespace vega {
//Forward declarations
class Model;
class ConfigurationParameters;
class Assertion;

class Globals {
public:
	static const int UNAVAILABLE_INT = INT_MIN;
	static const double UNAVAILABLE_DOUBLE;
};

class ParsingException: public std::exception {
private:
	std::string msg;

public:
	ParsingException(std::string message, std::string filename, int lineNum);
	operator const char*() const;
	virtual const char* what() const throw ();
	virtual ~ParsingException() throw ();
};

/**
 * Base class for all Parsers
 */
class Parser {

protected:
	Parser() {
	}

public:
	/**
	 * Read a model from a specific file format.
	 *
	 * configuration: a set of configuration parameters usually specified on the command line
	 */
	virtual std::shared_ptr<Model> parse(const ConfigurationParameters& configuration) = 0;
	virtual ~Parser() {
	}

};

class WriterException: public std::exception {
private:
	std::string msg;

public:
	WriterException(std::string message, std::string filename = std::string(""));
	operator const char*() const;
	virtual const char* what() const throw ();
	virtual ~WriterException() throw ();
};

class Writer {
private:
	friend std::ostream& operator<<(std::ostream& out, Writer& f);
	/**
	 * returns a std::string representation of the Writer useful for debugging
	 */
	virtual std::string toString() = 0;

public:
	virtual ~Writer() {
	}
	/**
	 * Write a model to the disk.
	 *
	 * configuration: a set of configuration parameters usually specified on the command line
	 *
	 * returns: the main model file written, to be used with the runner.
	 */
	virtual std::string writeModel(const std::shared_ptr<Model> model_ptr,
			const ConfigurationParameters &configuration) = 0;
};

class Runner {
public:
	enum ExitCode {
		OK = 0,
		SOLVER_NOT_FOUND = 100,
		TRANSLATION_SYNTAX_ERROR = 101,
		SOLVER_KILLED = 102,
		SOLVER_EXIT_NOT_ZERO = 103,
		SOLVER_RESULT_NOT_FOUND = 104,
		TEST_FAIL = 105
	};
	/**
	 * Run a solver.
	 *
	 * configuration: a set of configuration parameters usually specified on the command line
	 */
	virtual ExitCode execSolver(const ConfigurationParameters &configuration,
			std::string modelFile) = 0;
	virtual ~Runner() {
	}
protected:
	void deletePreviousResultFiles(std::string currentModel,
			const std::vector<std::string> extensions);
	std::string stripExtension(const std::string& currentModel) const;
	ExitCode convertExecResult(int exitCode) const;
};

/**
 * This abstract class represent the base class for all the classes that read the results of a solver.
 *
 * TODO:
 * At the moment it generates a vector<shared_ptr<Assertion>> for the solver tests, but it should be
 * separated in a vector of Results (for clarity and to allow different strategy of testing)
 */
class ResultReader {

public:
	/**
	 * Reads the results of an analysis from a file.
	 *
	 * configuration: a set of configuration parameters usually specified on the command line
	 */
	virtual void add_assertions(const ConfigurationParameters& configuration,
			std::shared_ptr<Model> model) = 0;
	virtual ~ResultReader() {
	}

};

} /* namespace vega */

#endif /* INPUTOUTPUT_H_ */
