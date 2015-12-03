/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * NastranTokenizer.cpp
 *
 *  Created on: Sep 1, 2013
 *      Author: devel
 */

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include "NastranTokenizer.h"
#include "../Abstract/SolverInterfaces.h"
#include <ciso646>

using namespace std;
using boost::tokenizer;
using boost::offset_separator;
using boost::lexical_cast;
using boost::trim_copy;

const int NastranTokenizer::UNAVAILABLE_INT = vega::Globals::UNAVAILABLE_INT;
const double NastranTokenizer::UNAVAILABLE_DOUBLE = vega::Globals::UNAVAILABLE_DOUBLE;

NastranTokenizer::NastranTokenizer(istream& input, vega::LogLevel logLevel, const string fileName) :
		instrream(input), currentField(0), logLevel(logLevel), fileName(fileName), lineNumber(0), currentSection(
				SECTION_EXECUTIVE) {
	this->nextSymbolType = NastranTokenizer::SYMBOL_KEYWORD;
}

NastranTokenizer::~NastranTokenizer() {
}

string NastranTokenizer::nextSymbolString() {
	if (this->nextSymbolType == SYMBOL_EOF) {
		ostringstream oss;
		oss << "Attempt to read past the end of file. Line: " << this->lineNumber << endl;
		throw runtime_error(oss.str());
	}
//first line
	while (currentLineVector.size() == 0) {
		nextLine();
		if (this->nextSymbolType == SYMBOL_EOF) {
			ostringstream oss;
			oss << "Attempt to read past the end of file. Line: " << this->lineNumber << endl;
			throw runtime_error(oss.str());
		}
	}

	string result = currentLineVector[currentField];
	if (this->nextSymbolType == SYMBOL_KEYWORD) {
		boost::to_upper(result);
	}
	this->currentField++;
	if (this->currentField >= this->currentLineVector.size()) {
		nextLine();
	} else {
		this->nextSymbolType = SYMBOL_FIELD;
	}
	return boost::trim_right_copy(result);
}

NastranTokenizer::LineType NastranTokenizer::getLineType(const string& line) {
	const string beginning = line.substr(0, 8);
	if (beginning.find(",") == string::npos) {
		if (beginning.find("*") == string::npos) {
			return SHORT_FORMAT;
		} else {
			return LONG_FORMAT;
		}
	} else {
		return FREE_FORMAT;
	}

}

bool NastranTokenizer::readLineSkipComment(string& line) {
	bool eof = true;
	while (getline(this->instrream, line)) {
		lineNumber += 1;
		if (!line.empty() and !boost::all(line,isblank) and line[0] != '$') {
			boost::iterator_range<string::iterator> middle_dollar = boost::find_first(line, "$");
			if (middle_dollar) {
				boost::erase_tail(line,
						static_cast<int>(line.size())
								- static_cast<int>(distance(line.begin(), middle_dollar.begin())));
			}
			//if the line is not blank exit the loop
			if (!boost::all(line, isblank)){
				eof = false;
				break;
			}
		}
	}
	return eof;
}

void NastranTokenizer::splitFreeFormat(string line, bool firstLine) {
	if (firstLine) {
		split(currentLineVector, line, boost::is_any_of(","));
	} else {
		//skip first field;
		string lineNoContinuation = line.substr(line.find(',') + 1);
		vector<string> otherLine;
		split(otherLine, lineNoContinuation, boost::is_any_of(","));
		currentLineVector.insert(currentLineVector.end(), otherLine.begin(), otherLine.end());
	}
	bool explicitContinuation = false;
	for (size_t fieldIndex = 1; fieldIndex < currentLineVector.size(); fieldIndex += 8) {
		string field = trim_copy(currentLineVector[fieldIndex]);
		if (field[0] == '+') {
			explicitContinuation = true;
			currentLineVector.erase(currentLineVector.begin() + fieldIndex);
		}
	}
	char c = static_cast<char>(this->instrream.peek());
	string line2;
    if (explicitContinuation || c == ',' || c == '+' || c == '*') {
		readLineSkipComment(line2);
		splitFreeFormat(line2, false);
	}
}

void NastranTokenizer::parseBulkSectionLine(string line) {
	LineType lineType = getLineType(line);
	switch (lineType) {
	case LONG_FORMAT:
		splitFixedFormat(line, true, true);
		break;
	case SHORT_FORMAT:
		splitFixedFormat(line, false, true);
		break;
	case FREE_FORMAT:
		splitFreeFormat(line, true);
		break;
	default:
		throw vega::ParsingException("line format not recognized: Line N ", this->fileName,
				lineNumber);
	}
}

void NastranTokenizer::executiveControlSection() {
	this->currentSection = SECTION_EXECUTIVE;
}


void NastranTokenizer::bulkSection() {
	this->currentSection = SECTION_BULK;
//if not first line, read again the current line
	if (currentLineVector.size() != 0) {
		currentLineVector.clear();
		//enough in 99% of lines
		currentLineVector.reserve(64);
		currentField = 0;
		parseBulkSectionLine(this->currentLine);
	}
}

void NastranTokenizer::parseParameters() {
	split(currentLineVector, this->currentLine, boost::is_any_of("\\="));
}

void NastranTokenizer::replaceTabs(string& line) {
	bool found = true;
	do {
		size_t pos = line.find("\t");
		found = pos != string::npos;
		if (found) {
			int numSpacesNeeded = 4 - (static_cast<int>(pos) % TAB_SIZE);
			string filler(numSpacesNeeded, ' ');
			boost::replace_first(line, "\t", filler);
		}
	} while (found);
}

bool NastranTokenizer::isNextInt() {
	if (nextSymbolType != NastranTokenizer::SYMBOL_FIELD) {
		return false;
	}
	string curField = trim_copy(currentLineVector[currentField]);
	return !curField.empty() && (strspn(curField.c_str(), "-0123456789") == curField.size());
}

bool NastranTokenizer::isNextDouble() {
	if (nextSymbolType != NastranTokenizer::SYMBOL_FIELD) {
		return false;
	}
	string curField = trim_copy(currentLineVector[currentField]);
	boost::algorithm::erase_all(curField, " ");
	return !curField.empty() && (strspn(curField.c_str(), "-+0123456789.eEdD") == curField.size());
}

bool NastranTokenizer::isNextEmpty() {
	if (nextSymbolType != NastranTokenizer::SYMBOL_FIELD) {
		return false;
	}
	string curField = trim_copy(currentLineVector[currentField]);
	return curField.empty();
}

bool NastranTokenizer::isEmptyUntilNextKeyword() {
	if (nextSymbolType == NastranTokenizer::SYMBOL_KEYWORD) {
		return true;
	} else if (nextSymbolType == NastranTokenizer::SYMBOL_EOF) {
		return true;
	}
	bool result = true;
	for (size_t i = currentField; i < this->currentLineVector.size() && result; i++) {
		string curField = trim_copy(currentLineVector[i]);
		result &= curField.empty();
	}
	return result;
}

void NastranTokenizer::nextLine() {

	currentLineVector.clear();
//enough in 99% of lines
	currentLineVector.reserve(128);
	currentField = 0;

	bool iseof = readLineSkipComment(this->currentLine);
	if (!iseof) {
		switch (currentSection) {
		case SECTION_EXECUTIVE:
			boost::algorithm::trim(this->currentLine);
			split(currentLineVector, this->currentLine, boost::is_any_of("\t\\= "), boost::algorithm::token_compress_on);
			break;
		case SECTION_BULK:
			parseBulkSectionLine(this->currentLine);
			break;
		}
		this->nextSymbolType = SYMBOL_KEYWORD;
	} else {
		this->nextSymbolType = SYMBOL_EOF;
	}
}

void NastranTokenizer::splitFixedFormat(string& line, const bool longFormat, const bool firstLine) {
	boost::offset_separator f;

	int fieldMax;
	if (longFormat) {
		int offsets[] = { 8, 16, 16, 16, 16, 8 };
		fieldMax = 5;
		f = offset_separator(offsets, offsets + 6);
	} else {
		int offsets[] = { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 };
		fieldMax = 9;
		f = offset_separator(offsets, offsets + 10);

	}
	replaceTabs(line);
	tokenizer<offset_separator> tok(line, f);
	tokenizer<offset_separator>::iterator beg = tok.begin();
	int count = 0;
	if (!firstLine) {
		//todo:check that explicit continuation tokens are the same
		++beg;
		count++;
	}
	bool explicitContinuation = false;
	for (; beg != tok.end(); ++beg) {
		string trimCopy = trim_copy(*beg);
		//erase all the long format specifiers
		if (count == 0) {
			boost::erase_all(trimCopy, "*");
		}
		currentLineVector.push_back(trimCopy);
		if (++count == fieldMax) {
			explicitContinuation = (++beg != tok.end()) && !(trim_copy(*beg).empty());
			if (explicitContinuation && this->logLevel >= vega::LogLevel::TRACE) {
				cout << "explicitContinuation" << endl;
			}
			break;
		}
	}
	string line2;
	if (explicitContinuation) {
		//todo:check that continuation tokens are the same
		bool iseof = readLineSkipComment(line2);
		if (!iseof) {
			splitFixedFormat(line2, longFormat, false);
		} else {
			throw "Continuation Expected: Line N " + this->lineNumber;
		}
	} else {
		//test for automatic continuation
		char c = static_cast<char>(this->instrream.peek());
		if (c == ' ' || c == '+' || c == '*') {
			readLineSkipComment(line2);
			//fill the current line with empty fields
			for (; count < fieldMax; count++) {
				currentLineVector.push_back("");
			}
			bool longFormat = (c == '*');
			splitFixedFormat(line2, longFormat, false);
		}
	}

}

string NastranTokenizer::nextString(bool returnDefaultIfNotFoundOrBlank, string defaultValue) {
	string result;
	if (returnDefaultIfNotFoundOrBlank && (this->nextSymbolType != SYMBOL_FIELD)) {
		return defaultValue;
	}

	string value = nextSymbolString();
	if (returnDefaultIfNotFoundOrBlank && !defaultValue.empty() && trim_copy(value).empty()) {
		value = defaultValue;
	}
	return value;
}

void NastranTokenizer::skip(int fields) {
	if (this->nextSymbolType == SYMBOL_EOF) {
		throw "Attempt to read past the end of file. Line:" + this->lineNumber;
	}

	this->currentField = min((unsigned int) this->currentLineVector.size(),
			this->currentField + fields);

	if (this->currentField == this->currentLineVector.size()) {
		nextLine();
	} else {
		this->nextSymbolType = SYMBOL_FIELD;
	}
}

void NastranTokenizer::skipToNotEmpty() {
	while (this->isNextEmpty())
		this->skip(1);
}

int NastranTokenizer::nextInt(bool returnDefaultIfNotFoundOrBlank, int defaultValue) {
	int result;
	if (returnDefaultIfNotFoundOrBlank && this->nextSymbolType != SYMBOL_FIELD) {
		return defaultValue;
	}
	string value = trim_copy(nextSymbolString());
	if (returnDefaultIfNotFoundOrBlank && value.empty()) {
		return defaultValue;
	}
	try {
		result = lexical_cast<int>(value);
	} catch (boost::bad_lexical_cast &) {
		string currentFieldstr =
				currentField == 0 ? string("LAST") : (lexical_cast<string>(currentField - 1));
		string message = "Value: [" + value + "] can't be converted to int. Field Num:"
				+ currentFieldstr;
		throw vega::ParsingException(message, fileName, lineNumber);
	}
	return result;
}

double NastranTokenizer::nextDouble(bool returnDefaultIfNotFoundOrBlank, double defaultValue) {
	double result;
	if (returnDefaultIfNotFoundOrBlank && this->nextSymbolType != SYMBOL_FIELD) {
		return defaultValue;

	}
	string value = trim_copy(nextSymbolString());
	if (returnDefaultIfNotFoundOrBlank && value.empty()) {
		return defaultValue;
	}
	boost::replace_all(value, "d", "e");
	boost::replace_all(value, "D", "E");
	boost::algorithm::erase_all(value, " ");
	size_t position = value.find_first_of("+-", 1);
	if (position != string::npos and position != value.find_first_of("eE", 1) + 1) {
		value.insert(position, "E");
	}
	try {
		result = lexical_cast<double>(value);
	} catch (boost::bad_lexical_cast &e) {
		string currentFieldstr =
				currentField == 0 ? string("LAST") : (lexical_cast<string>(currentField - 1));
		string message = "Value: [" + value + "]can't be converted to double. Field Num: "
				+ currentFieldstr + " " + e.what();
		throw vega::ParsingException(message, fileName, lineNumber);
	}
	return result;
}

vector<string> NastranTokenizer::currentDataLine() const {
	return currentLineVector;
}

const string NastranTokenizer::currentRawDataLine() const {
	return this->currentLine;
}
