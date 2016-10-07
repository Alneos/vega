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
#include <istream>
#include "ConfigurationParameters.h"
#include <string>

namespace vega {
//Forward declarations
class Model;
class Assertion;

class Globals {
public:
	static const int UNAVAILABLE_INT = INT_MIN;
	static const double UNAVAILABLE_DOUBLE;
};


std::string ParserMessageException(std::string arg, std::string fname, int lineNum, std::string key);
std::string ParserMessageWarning(std::string arg, std::string fname, int lineNum, std::string key);
std::string WriterMessageException(std::string arg, std::string fname, std::string key);
std::string WriterMessageWarning(std::string arg, std::string fname, std::string key);


class ParsingException: public std::exception {
private:
	std::string msg;

public:
	ParsingException(std::string message, std::string filename, int lineNum, std::string key="");
	operator const char*() const;
	virtual const char* what() const throw ();
	virtual ~ParsingException() throw ();
};


/**
 * Base class for all Tokenizers.
 */
class Tokenizer {
	friend class Parser;
protected:
	Tokenizer(std::istream& stream, vega::LogLevel logLevel = vega::LogLevel::INFO,
			const std::string fileName = "UNKNOWN",
			const vega::ConfigurationParameters::TranslationMode translationMode = vega::ConfigurationParameters::BEST_EFFORT);
	std::istream& instrream;
	vega::LogLevel logLevel;
	std::string fileName;    /**< Current fileName: only used for printout and error managment. **/
	vega::ConfigurationParameters::TranslationMode translationMode;
	int lineNumber;
	std::string currentKeyword; /**< Current Keyword: only used for printout and error managment. **/

public:
	virtual ~Tokenizer() {
	}
	inline vega::LogLevel getLogLevel()const {return logLevel;};
	inline std::string getFileName() const {return fileName;};
	//inline vega::ConfigurationParameters::TranslationMode getTranslationMode() const {return translationMode;};
	inline int getLineNumber() const {return lineNumber;};
	inline std::string getCurrentKeyword() const {return currentKeyword;};
	void setCurrentKeyword(std::string cK) {currentKeyword=cK;};

    /**
     * Generic handler for parsing exceptions.
     * Throw a ParsingException in strict mode, which shuts the program, and a string otherwise, which
     * should skip the problematic command.
     */
	void handleParsingError(const std::string& message);

    /**
     * Generic handler for parsing warnings.
     * Print a warning message
     */
	//TODO: Decide of a politic for warning.
	void handleParsingWarning(const std::string& message);


};

/**
 * Base class for all Parsers
 */
class Parser {

protected:
	Parser();
public:
	ConfigurationParameters::TranslationMode translationMode;
	/**
	 * Read a model from a specific file format.
	 *
	 * configuration: a set of configuration parameters usually specified on the command line
	 */
	virtual std::shared_ptr<Model> parse(const ConfigurationParameters& configuration) = 0;
	virtual ~Parser() {
	}

    /**
     * Generic handler for parsing exceptions.
     * Throw a ParsingException in strict mode, which shuts the program, and a string otherwise, which
     * should skip the problematic command.
     */
	void handleParsingError(const std::string& message, Tokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Generic handler for parsing exception.
     * Throw a ParsingException in strict mode, which shuts the program, and a string otherwise, which
     * should skip the problematic command.
     */
	//TODO: Do a difference of treatment in case of a "very-strict" conversion.
	void handleParsingWarning(const std::string& message, Tokenizer& tok, std::shared_ptr<Model> model);

};

class WritingException: public std::exception {
private:
	std::string msg;

public:
	WritingException(std::string message, std::string key = std::string(""), std::string filename = std::string(""));
	operator const char*() const;
	virtual const char* what() const throw ();
	virtual ~WritingException() throw ();
};

class Writer {
private:
	friend std::ostream& operator<<(std::ostream& out, Writer& f);
	/**
	 * returns a std::string representation of the Writer useful for debugging
	 */
	virtual std::string toString() = 0;
protected:
	Writer();
public:
	virtual ~Writer() {
	}
	ConfigurationParameters::TranslationMode translationMode;
	/**
	 * Write a model to the disk.
	 *
	 * configuration: a set of configuration parameters usually specified on the command line
	 *
	 * returns: the main model file written, to be used with the runner.
	 */
	virtual std::string writeModel(const std::shared_ptr<Model> model_ptr,
			const ConfigurationParameters &configuration) = 0;

    /**
     * Generic handler for writing exceptions.
     * Throw a WritingException in strict mode, which shuts the program.
     * Otherwise, print a message in cerr.
     */
	void handleWritingError(const std::string& message, const std::string& keyword="", const std::string& file="");

    /**
     * Generic handler for writing warnings.
     * Print a warning message
     */
	//TODO: Decide of a politic for warning.
	void handleWritingWarning(const std::string& message, const std::string& keyword="", const std::string& file="");
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
