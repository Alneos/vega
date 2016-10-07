/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * InputOutput.cpp
 *
 *  Created on: Oct 30, 2013
 *      Author: devel
 */

#include "SolverInterfaces.h"
#include "Model.h"
#include "ConfigurationParameters.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <stdio.h>
#if defined VDEBUG && defined __GNUC__
#include <execinfo.h>
#endif

namespace vega {

namespace fs = boost::filesystem;
using boost::lexical_cast;
using namespace std;

const double Globals::UNAVAILABLE_DOUBLE = -DBL_MAX;


string MessageException(string arg, string fname, int lineNum, string key){
	string msg;
	msg.append("Parsing error in ");
	msg.append(key);
	msg.append(" (file ");
	msg.append(fname);
	msg.append(" line ");
	msg.append((lexical_cast<string>(lineNum)));
	msg.append("): ");
	msg.append(arg);
	return msg;
}

string MessageWarning(string arg, string fname, int lineNum, string key){
	string msg;
	msg.append("Parsing warning in ");
	msg.append(key);
	msg.append(" (file ");
	msg.append(fname);
	msg.append(" line ");
	msg.append((lexical_cast<string>(lineNum)));
	msg.append("): ");
	msg.append(arg);
	return msg;
}


ParsingException::ParsingException(string arg, string fname, int lineNum, string key) {

	msg.append(MessageException(arg, fname, lineNum, key));

#ifdef __GNUC__
	//defined in top level cmake file
#ifdef VDEBUG
	void *array[10];
	size_t size;
	char **strings;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);
	msg.append("\n");
	for (size_t i = 0; i < size; i++) {
		msg.append(strings[i]);
		msg.append("\n");
	}
	free (strings);
#endif
#endif

}

ParsingException::operator const char*() const {
	return msg.c_str();
}

const char* ParsingException::what() const throw () {
	return msg.c_str();
}

ParsingException::~ParsingException() throw () {
}

WriterException::WriterException(string arg, string fname) {
	msg.append("Writing error ");
	if (!fname.empty()) {
		msg.append("in file ");
		msg.append(fname);
	}
	msg.append(": ");
	msg.append(arg);

#ifdef __GNUC__
	//flag defined by cmake in Release build
#ifdef VDEBUG
	void *array[10];
	size_t size;
	char **strings;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);
	msg.append("\n");
	for (size_t i = 0; i < size; i++) {
		msg.append(strings[i]);
		msg.append("\n");
	}
	free (strings);
#endif
#endif

}

WriterException::operator const char*() const {
	return msg.c_str();
}

const char* WriterException::what() const throw () {
	return msg.c_str();
}

WriterException::~WriterException() throw () {
}


Tokenizer::Tokenizer(istream& stream, vega::LogLevel logLevel,  string fileName, vega::ConfigurationParameters::TranslationMode translationMode) :
	instrream(stream), logLevel(logLevel), fileName(fileName), translationMode(translationMode), lineNumber(0), currentKeyword(""){
}

void Tokenizer::handleParsingError(const string& message) {

	switch (translationMode) {
	case ConfigurationParameters::MODE_STRICT:
		throw ParsingException(message, fileName, lineNumber, currentKeyword);
	case ConfigurationParameters::MESH_AT_LEAST:
		//model->onlyMesh = true;
		cerr << MessageException(message, fileName, lineNumber, currentKeyword) << endl;
		throw std::string("skipCommand");
		break;
	case ConfigurationParameters::BEST_EFFORT:
		cerr << MessageException(message, fileName, lineNumber, currentKeyword) << endl;
		throw std::string("skipCommand");
		break;
	default:
		cerr << "Unknown enum in Translation mode, assuming MODE_STRICT" << endl;
		throw ParsingException(message, fileName, lineNumber, currentKeyword);
	}
}

void Tokenizer::handleParsingWarning(const string& message) {
	cerr << MessageWarning(message, fileName, lineNumber, currentKeyword);
}



Parser::Parser(){
	this->translationMode= ConfigurationParameters::BEST_EFFORT;
}

void Parser::handleParsingError(const string& message, Tokenizer& tok,
		shared_ptr<Model> model) {

	switch (translationMode) {
	case ConfigurationParameters::MODE_STRICT:
		throw ParsingException(message, tok.fileName, tok.lineNumber, tok.currentKeyword);
	case ConfigurationParameters::MESH_AT_LEAST:
		model->onlyMesh = true;
		cerr << MessageException(message, tok.fileName, tok.lineNumber, tok.currentKeyword) << endl;
		throw std::string("skipCommand");
		break;
	case ConfigurationParameters::BEST_EFFORT:
		cerr << MessageException(message, tok.fileName, tok.lineNumber, tok.currentKeyword) << endl;
		throw std::string("skipCommand");
		break;
	default:
		cerr << "Unknown enum in Translation mode, assuming MODE_STRICT" << endl;
		throw ParsingException(message, tok.fileName, tok.lineNumber, tok.currentKeyword);
	}
}

void Parser::handleParsingWarning(const string& message, Tokenizer& tok,
		shared_ptr<Model> model) {
	UNUSEDV(model);
	cerr << MessageWarning(message, tok.fileName, tok.lineNumber, tok.currentKeyword) << endl;
}


ostream& operator<<(ostream& out, Writer& f) {
	out << f.toString() << endl;
	return out;
}

string Runner::stripExtension(const string& currentModel) const {
	size_t dotPos = currentModel.find_last_of(".");
	string basePath;
	if (dotPos == string::npos) {
		basePath = currentModel;
	} else {
		basePath = currentModel.substr(0, dotPos);
	}
	return basePath;
}

void Runner::deletePreviousResultFiles(string currentModel, const vector<string> extensions) {
	string basePath = stripExtension(currentModel);
	for (const string extension : extensions) {
		string fileToDelete = basePath + extension;
		if (fs::exists(fileToDelete)) {
			if (remove(fileToDelete.c_str()) != 0) {
				string message = string("error removing result file ") + fileToDelete;
				perror(message.c_str());
			}
		}
	}
}

Runner::ExitCode Runner::convertExecResult(int exitCode) const {
	if (exitCode == 0) {
		return OK;
	}
	ExitCode result = SOLVER_EXIT_NOT_ZERO;
#ifdef __GNUC__
	//bash exit codes http://tldp.org/LDP/abs/html/exitcodes.html
	//int lowByte = 0x00FF & exitCode;
	int highByte = (exitCode && 0xFF00) >> 8;
	if (highByte == 127) {
		result = SOLVER_NOT_FOUND;
	} else if (highByte > 128 && highByte <= 165) {
		result = SOLVER_KILLED;
	} else if (exitCode == -1) {
		result = SOLVER_NOT_FOUND;
	}
#else

#endif
	return result;
}

}

