/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * NastranTokenizer_test.cpp
 *
 *  Created on: Sep 1, 2013
 *      Author: devel
 */

#define BOOST_TEST_MODULE nastran_tokenizer_tests
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <cstddef>
#include <fstream>
#include <vector>

namespace fs = boost::filesystem;
using namespace std;

#include "build_properties.h"
#include "../../Nastran/NastranTokenizer.h"

BOOST_AUTO_TEST_CASE(nastran_short_with_comments) {
	string nastranLine = "$comment comment \nKEYWORD 12345\n2NDLINE 1234567 1234567 ";
	istringstream istr(nastranLine);

	NastranTokenizer tokenizer(istr);
	tokenizer.bulkSection();
	//comments are skipped
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("KEYWORD"));
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("12345"));
	//newline
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), "2NDLINE");
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), "1234567");
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), "1234567");
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_EOF);

}

BOOST_AUTO_TEST_CASE(nastran_short_with_tabs) {
	string nastranLine = "$comment comment \nKEYWORD 12345\t123\t\t1234567";
	istringstream istr(nastranLine);

	NastranTokenizer tokenizer(istr);
	tokenizer.bulkSection();
	//comments are skipped
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("KEYWORD"));
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("12345"));
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("123"));
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("1234567"));
}
/*
BOOST_AUTO_TEST_CASE(comments_and_empty_lines) {
	//this is a bug under windows
	string nastranLine = "$comnent comment \nKEYWORD 12345\n$ comment\n\n$ comment\n\n";
	istringstream istr(nastranLine);

	NastranTokenizer tokenizer(istr);
	tokenizer.bulkSection();
	try{
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("KEYWORD"));
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);

		BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("12345"));
	}
	catch (exception &e){
		string message = string("Reading field threw exception: ") + e.what();
		BOOST_FAIL(message);
	}
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_EOF);
	//BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("123"));
	//BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
	//BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("1234567"));
}
	  */

BOOST_AUTO_TEST_CASE(nastran_short_2_line_incomplete_line) {
	string nastranLine = "$comnent comment \nKEYWORD 12345\n        01234567";
	istringstream istr(nastranLine);
	NastranTokenizer tokenizer(istr);
	tokenizer.bulkSection();
	//comments are skipped
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("KEYWORD"));
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("12345"));
	for (int i = 0; i < 7; i++) {
		BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
		BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), "");
	}
	//newline
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), "01234567");
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_EOF);

}

BOOST_AUTO_TEST_CASE(nastran_short_with_blank_at_end) {
	string nastranLine =
			"KEYWORD1       0       1       2       3       4       5       6       7                    \nKEYWORD2";
	istringstream istr(nastranLine);
	NastranTokenizer tokenizer(istr);
	tokenizer.bulkSection();
	//comments are skipped
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("KEYWORD1"));
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
	for (int i = 0; i < 7; i++) {
		BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
		BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), to_string(i));
	}
	//next keyword
	tokenizer.nextLine();
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("KEYWORD2"));
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_EOF);
}

//supported only if continuation line is contigous
BOOST_AUTO_TEST_CASE(nastran_long_format_explicit_cont) {
	string nastranLine =
			"*KEYWORD0123456789      0123456789      0123456789      0123456789       +\n+       0123456789";
	istringstream istr(nastranLine);
	NastranTokenizer tokenizer(istr);
	tokenizer.bulkSection();
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("KEYWORD"));
	for (int i = 0; i < 5; i++) {
		BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
		BOOST_CHECK_EQUAL(tokenizer.nextInt(), 123456789);
	}
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_EOF);
}

BOOST_AUTO_TEST_CASE(nastran_long_format_auto_cont) {
	string nastranLine =
			"*KEYWORD0123456789      0123456789      0123456789      0123456789\n        12345678 9";
	istringstream istr(nastranLine);
	NastranTokenizer tokenizer(istr);
	tokenizer.bulkSection();
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	string curr_keyword = tokenizer.nextSymbolString();
	BOOST_CHECK_EQUAL(curr_keyword, string("KEYWORD"));
	for (int i = 0; i < 4; i++) {
		BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
		BOOST_CHECK_EQUAL(tokenizer.nextInt(), 123456789);
	}
	//second line is short format
	BOOST_CHECK_EQUAL(tokenizer.nextInt(), 12345678);
	BOOST_CHECK_EQUAL(tokenizer.nextInt(), 9);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_EOF);
}

BOOST_AUTO_TEST_CASE(nastran_free_format_continuation_long) {
	string nastranLine = "KEYWORD,89,89,89,89,89,89,89,89\n*,89";
	istringstream istr(nastranLine);
	NastranTokenizer tokenizer(istr);
	tokenizer.bulkSection();
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolString(), string("KEYWORD"));
	for (int i = 0; i < 9; i++) {
		BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
		BOOST_CHECK_EQUAL(tokenizer.nextInt(), 89);
	}
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_EOF);
}

BOOST_AUTO_TEST_CASE(nastran_free_format_continuation_short) {
	string nastranLine = "KEYWORD,0,1,2,3,4,5,6,7\n,8";
	istringstream istr(nastranLine);
	NastranTokenizer tokenizer(istr);
	tokenizer.bulkSection();
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	tokenizer.nextSymbolString();
	for (int i = 0; i < 9; i++) {
		BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
		BOOST_CHECK_EQUAL(tokenizer.nextInt(), i);
	}
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_EOF);
}

BOOST_AUTO_TEST_CASE(is_next_double) {
	// Quick Reference Guide :
	// Real numbers may be entered in a variety of ways. For example, the following are all
	// acceptable versions of the real number seven:
	string nastranLine = "keyword, 7.0,.7E1,0.7+1,.70+1,7.E+0,70.-1";
	istringstream istr(nastranLine);
	NastranTokenizer tokenizer(istr);
	tokenizer.bulkSection();
	BOOST_CHECK_EQUAL(tokenizer.nextString(), "KEYWORD");
	BOOST_CHECK_EQUAL(tokenizer.nextDouble(), 7.0);
	BOOST_CHECK_EQUAL(tokenizer.nextDouble(), 7.0);
	BOOST_CHECK_EQUAL(tokenizer.nextDouble(), 7.0);
	BOOST_CHECK_EQUAL(tokenizer.nextDouble(), 7.0);
	BOOST_CHECK_EQUAL(tokenizer.nextDouble(), 7.0);
	BOOST_CHECK_EQUAL(tokenizer.nextDouble(), 7.0);
}

BOOST_AUTO_TEST_CASE(double_precision) {
	string nastranLine = "keyword, 5.000388 D+5";
	istringstream istr(nastranLine);
	NastranTokenizer tokenizer(istr);
	tokenizer.bulkSection();
	BOOST_CHECK_EQUAL(tokenizer.nextString(), "KEYWORD");
	BOOST_CHECK(tokenizer.isNextDouble());
	BOOST_CHECK(tokenizer.nextDouble());
}

BOOST_AUTO_TEST_CASE(nastran_free_format_continuation_symbol) {
	string nastranLine = "chexa, 0, 1, 2, 3, 4, 5, 6, 7, +HX1\n+HX1, 8, 9";
	istringstream istr(nastranLine);
	NastranTokenizer tokenizer(istr);
	tokenizer.bulkSection();
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	BOOST_CHECK_EQUAL("CHEXA", tokenizer.nextSymbolString());
	for (int i = 0; i < 10; i++) {
		BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_FIELD);
		BOOST_CHECK_EQUAL(tokenizer.nextInt(), i);
	}
	BOOST_CHECK_EQUAL(tokenizer.nextSymbolType, NastranTokenizer::SYMBOL_EOF);
}

BOOST_AUTO_TEST_CASE(nastran_auto_contiuation_short_incomplete_line) {
	string nastranLine =
	//12345678|2345678|2345678|2345678|
			"PBARL   1       2               BAR\n"
					"        2.      4.\n$\n$\nTEST";
	istringstream istr(nastranLine);
	NastranTokenizer tok(istr);
	tok.bulkSection();
	BOOST_CHECK_EQUAL(tok.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	BOOST_CHECK_EQUAL("PBARL", tok.nextSymbolString());
	int elemId = tok.nextInt();
	BOOST_CHECK_EQUAL(1, elemId);
	int material_id = tok.nextInt();
	BOOST_CHECK_EQUAL(2, material_id);

	string group = tok.nextString(true);
	string type = tok.nextString();
	BOOST_CHECK_EQUAL("BAR", type);
	tok.skip(4);
	double width = tok.nextDouble();
	BOOST_CHECK_EQUAL(2., width);
	double height = tok.nextDouble();
	BOOST_CHECK_EQUAL(4., height);
	BOOST_CHECK_EQUAL(tok.nextSymbolType, NastranTokenizer::SYMBOL_KEYWORD);
	BOOST_CHECK_EQUAL("TEST", tok.nextSymbolString());
}

void countGridElems(NastranTokenizer& tok) {
	int symcount = 0;
	while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
		string symbol = tok.nextSymbolString();
		symcount += symbol.empty() ? 0 : 1;
	}
	BOOST_CHECK_EQUAL(4, symcount);
}

BOOST_AUTO_TEST_CASE(nastran_bar1r_grid) {
	fs::path sourceFname(
			string(PROJECT_BASE_DIR) + "/testdata/nastran/linstat/plate1r/plate1r.dat");
	ifstream source(sourceFname.c_str());
	NastranTokenizer tok(source);
	while (tok.nextSymbolString().find("BEGIN") == string::npos) {
		tok.nextLine();
	}
	tok.bulkSection();
	tok.nextLine();
	BOOST_CHECK_EQUAL("PARAM", tok.nextSymbolString());
	tok.nextLine();
	BOOST_CHECK_EQUAL(NastranTokenizer::SYMBOL_KEYWORD, tok.nextSymbolType);
	BOOST_CHECK_EQUAL("GRID", tok.nextSymbolString());
	countGridElems(tok);
	BOOST_CHECK_EQUAL(NastranTokenizer::SYMBOL_KEYWORD, tok.nextSymbolType);
	BOOST_CHECK_EQUAL("GRID", tok.nextSymbolString());
	countGridElems(tok);

	source.close();
}

