/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * This file is part of Vega.
 *
 *   Vega is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Vega is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Vega.  If not, see <http://www.gnu.org/licenses/>.
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
#if defined VDEBUG && defined __GNUC__ && !defined(_WIN32)
#include <execinfo.h>
#endif

namespace vega {

namespace fs = boost::filesystem;
using boost::lexical_cast;
using namespace std;


string ParsingMessageException(string arg, string fname, int lineNum, string key){
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

string ParsingMessageWarning(string arg, string fname, int lineNum, string key){
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

string WritingMessageException(string arg, string key, string fname){
    string msg;
    msg.append("Writing error");
    if (key!=""){
        msg.append(" in ");
        msg.append(key);
    }
    if (fname!=""){
        msg.append(" (file ");
        msg.append(fname);
        msg.append(")");
    }
    msg.append(": ");
    msg.append(arg);
    return msg;
}

string WritingMessageWarning(string arg, string key, string fname){
    string msg;
    msg.append("Writing warning");
    if (key!=""){
        msg.append(" in ");
        msg.append(key);
    }
    if (fname!=""){
        msg.append(" (file ");
        msg.append(fname);
        msg.append(")");
    }
    msg.append(": ");
    msg.append(arg);
    return msg;
}


ParsingException::ParsingException(string arg, string fname, int lineNum, string key) {

    msg.append(ParsingMessageException(arg, fname, lineNum, key));
}

ParsingException::operator const char*() const {
    return msg.c_str();
}

const char* ParsingException::what() const throw () {
    return msg.c_str();
}

ParsingException::~ParsingException() throw () {
}

WritingException::WritingException(string arg, string key, string fname) {
    msg.append(WritingMessageException(arg, key, fname));
}

WritingException::operator const char*() const {
    return msg.c_str();
}

const char* WritingException::what() const throw () {
    return msg.c_str();
}

WritingException::~WritingException() throw () {
}

Tokenizer::Tokenizer(istream& stream, vega::LogLevel logLevel,  string fileName, vega::ConfigurationParameters::TranslationMode translationMode) :
    instrream(stream), logLevel(logLevel), fileName(fileName), translationMode(translationMode), lineNumber(0), currentKeyword(""){
}

void Tokenizer::handleParsingError(const string& message) {

    switch (translationMode) {
    case ConfigurationParameters::TranslationMode::MODE_STRICT:
        // Problem on static over Alpine
        //throw ParsingException(message, fileName, lineNumber, currentKeyword);
        cerr << ParsingException(message, fileName, lineNumber, currentKeyword);
        vega::stacktrace();
        exit(2);
    case ConfigurationParameters::TranslationMode::MESH_AT_LEAST:
        //model->onlyMesh = true;
        cerr << ParsingMessageException(message, fileName, lineNumber, currentKeyword) << endl;
        throw std::string("skipCommand");
        break;
    case ConfigurationParameters::TranslationMode::BEST_EFFORT:
        cerr << ParsingMessageException(message, fileName, lineNumber, currentKeyword) << endl;
        throw std::string("skipCommand");
        break;
    default:
        cerr << "Unknown enum class in Translation mode, assuming MODE_STRICT" << endl;
        throw ParsingException(message, fileName, lineNumber, currentKeyword);
    }
}

void Tokenizer::handleParsingWarning(const string& message) {
    cerr << ParsingMessageWarning(message, fileName, lineNumber, currentKeyword);
}



Parser::Parser(){
    this->translationMode= ConfigurationParameters::TranslationMode::BEST_EFFORT;
}

void Parser::handleParsingError(const string& message, Tokenizer& tok,
        shared_ptr<Model> model) {

    switch (translationMode) {
    case ConfigurationParameters::TranslationMode::MODE_STRICT:
        throw ParsingException(message, tok.fileName, tok.lineNumber, tok.currentKeyword);
    case ConfigurationParameters::TranslationMode::MESH_AT_LEAST:
        model->onlyMesh = true;
        cerr << ParsingMessageException(message, tok.fileName, tok.lineNumber, tok.currentKeyword) << endl;
        throw std::string("skipCommand");
        break;
    case ConfigurationParameters::TranslationMode::BEST_EFFORT:
        cerr << ParsingMessageException(message, tok.fileName, tok.lineNumber, tok.currentKeyword) << endl;
        throw std::string("skipCommand");
        break;
    default:
        cerr << "Unknown enum class in Translation mode, assuming MODE_STRICT" << endl;
        throw ParsingException(message, tok.fileName, tok.lineNumber, tok.currentKeyword);
    }
}

void Parser::handleParsingWarning(const string& message, Tokenizer& tok,
        shared_ptr<Model> model) {
    UNUSEDV(model);
    cerr << ParsingMessageWarning(message, tok.fileName, tok.lineNumber, tok.currentKeyword) << endl;
}

void Writer::handleWritingError(const string& message, const string& keyword, const string& file) {
    switch (translationMode) {
    case ConfigurationParameters::TranslationMode::MODE_STRICT:
        throw WritingException(message, keyword, file);
    case ConfigurationParameters::TranslationMode::MESH_AT_LEAST:
    case ConfigurationParameters::TranslationMode::BEST_EFFORT:
        cerr << WritingMessageException(message, keyword, file) << endl;
        //throw std::string("skipCommand");
        break;
    default:
        cerr << "Unknown enum class in Translation mode, assuming MODE_STRICT" << endl;
        throw WritingException(message, keyword, file);
    }
}

void Writer::handleWritingWarning(const string& message, const string& keyword, const string& file){
    cerr << WritingMessageWarning(message, keyword, file) << endl;
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

Runner::ExitCode Runner::convertExecResult(const int exitCode) const {
    if (exitCode == 0) {
        return ExitCode::OK;
    }
    ExitCode result = ExitCode::SOLVER_EXIT_NOT_ZERO;
#ifdef __GNUC__
    //bash exit codes http://tldp.org/LDP/abs/html/exitcodes.html
    //int lowByte = 0x00FF & exitCode;
    int highByte = (exitCode & 0xFF00) >> 8;
    if (highByte == 127) {
        result = ExitCode::SOLVER_NOT_FOUND;
    } else if (highByte > 128 && highByte <= 165) {
        result = ExitCode::SOLVER_KILLED;
    } else if (exitCode == -1) {
        result = ExitCode::SOLVER_NOT_FOUND;
    }
#else

#endif
    return result;
}

} //namespace vega

