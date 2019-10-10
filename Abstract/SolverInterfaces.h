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
#include <iostream>
#include <vector>
#include <istream>
#include "ConfigurationParameters.h"
#include "Utility.h"
#include <string>

namespace vega {
//Forward declarations
class Model;
class Assertion;

std::string ParsingMessageException(std::string arg, std::string fname, int lineNum, std::string key);
std::string ParsingMessageWarning(std::string arg, std::string fname, int lineNum, std::string key);
std::string WritingMessageException(std::string arg, std::string fname, std::string key);
std::string WritingMessageWarning(std::string arg, std::string fname, std::string key);


class ParsingException: public std::exception {
private:
	std::string msg;

public:
	ParsingException(std::string message, std::string filename, int lineNum, std::string key="");
	operator const char*() const;
	const char* what() const throw () override;
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
			const vega::ConfigurationParameters::TranslationMode translationMode = vega::ConfigurationParameters::TranslationMode::BEST_EFFORT);
	std::istream& instrream;
	vega::LogLevel logLevel;
	std::string fileName;    /**< Current fileName: only used for printout and error managment. **/
	vega::ConfigurationParameters::TranslationMode translationMode;
	int lineNumber;
	std::string currentKeyword; /**< Current Keyword: only used for printout and error managment. **/

public:
	virtual ~Tokenizer() = default;
	inline vega::LogLevel getLogLevel()const noexcept {return logLevel;};
	inline std::string getFileName() const noexcept {return fileName;};
	//inline vega::ConfigurationParameters::TranslationMode getTranslationMode() const {return translationMode;};
	inline int getLineNumber() const noexcept {return lineNumber;};
	inline std::string getCurrentKeyword() const noexcept {return currentKeyword;};
	void setCurrentKeyword(std::string cK) noexcept {currentKeyword=cK;};

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
	Parser() = default;
public:
	virtual ~Parser() = default;
	ConfigurationParameters::TranslationMode translationMode = ConfigurationParameters::TranslationMode::BEST_EFFORT;
	/**
	 * Read a model from a specific file format.
	 *
	 * configuration: a set of configuration parameters usually specified on the command line
	 */
	virtual std::unique_ptr<Model> parse(const ConfigurationParameters& configuration) = 0;

    /**
     * Generic handler for parsing exceptions.
     * Throw a ParsingException in strict mode, which shuts the program, and a string otherwise, which
     * should skip the problematic command.
     */
	void handleParsingError(const std::string& message, Tokenizer& tok, Model& model);

    /**
     * Generic handler for parsing exception.
     * Throw a ParsingException in strict mode, which shuts the program, and a string otherwise, which
     * should skip the problematic command.
     */
	//TODO: Do a difference of treatment in case of a "very-strict" conversion.
	void handleParsingWarning(const std::string& message, Tokenizer& tok, Model& model);
};

class WritingException: public std::exception {
private:
	std::string msg;

public:
	WritingException(std::string message, std::string key = "", std::string filename = "");
	operator const char*() const;
	const char* what() const throw () override;
	virtual ~WritingException() throw ();
};

class Writer {
private:
	friend std::ostream& operator<<(std::ostream& out, Writer& f);
	/**
	 * returns a std::string representation of the Writer useful for debugging
	 */
	virtual std::string toString() const = 0;
public:
    virtual ~Writer() = default;
	ConfigurationParameters::TranslationMode translationMode = ConfigurationParameters::TranslationMode::BEST_EFFORT;
	/**
	 * Write a model to the disk.
	 *
	 * configuration: a set of configuration parameters usually specified on the command line
	 *
	 * returns: the main model file written, to be used with the runner.
	 */
	virtual std::string writeModel(Model&, const ConfigurationParameters&) = 0;

    /**
     * Generic handler for writing exceptions.
     * Throw a WritingException in strict mode, which shuts the program.
     * Otherwise, print a message in cerr.
     */
	void handleWritingError(const std::string& message, const std::string& keyword="", const std::string& file="") const;

    /**
     * Generic handler for namespace vega {writing warnings.
     * Print a warning message
     */
	//TODO: Decide of a politic for warning.
	void handleWritingWarning(const std::string& message, const std::string& keyword="", const std::string& file="") const;
};

class Runner {
public:
	enum class ExitCode {
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
	virtual ~Runner() = default;
protected:
	void deletePreviousResultFiles(std::string currentModel,
			const std::vector<std::string> extensions);
	std::string stripExtension(const std::string& currentModel) const;
	ExitCode convertExecResult(const int exitCode) const;
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
			Model& model) = 0;
	virtual ~ResultReader() {
	}

};

} /* namespace vega */

#endif /* INPUTOUTPUT_H_ */
