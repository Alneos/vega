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
#include "build_properties.h"
#include "../../Nastran/NastranTokenizer.h"

namespace fs = boost::filesystem;
using namespace std;
using namespace vega;
using namespace nastran;

BOOST_AUTO_TEST_CASE(nastran_short_with_comments) {
    string nastranLine = "$comment comment \nKEYWORD 12345\n2NDLINE 1234567 1234567 ";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    //comments are skipped
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), string("KEYWORD"));
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), string("12345"));
    //newline
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), "2NDLINE");
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), "1234567");
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), "1234567");
    //end
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_EOF);

}

BOOST_AUTO_TEST_CASE(nastran_short_with_tabs) {
    string nastranLine = "$comment comment \nKEYWORD 12345\t123\t\t1234567";
    istringstream istr(nastranLine);

    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    //comments are skipped
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), "KEYWORD");
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), "12345");
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), "123");
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(true,""), "");
    BOOST_CHECK_EQUAL(tokenizer.nextString(), "1234567");
}

//BOOST_AUTO_TEST_CASE(comments_and_empty_lines) {
//    //this is a bug under windows
//    string nastranLine = "$comnent comment \nKEYWORD 12345\n$ comment\n\n$ comment\n\n";
//    istringstream istr(nastranLine);
//
//    NastranTokenizer tokenizer(istr);
//    tokenizer.bulkSection();
//    try{
//    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
//    BOOST_CHECK_EQUAL(tokenizer.nextString(), string("KEYWORD"));
//    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
//
//        BOOST_CHECK_EQUAL(tokenizer.nextString(), string("12345"));
//    }
//    catch (exception &e){
//        string message = string("Reading field threw exception: ") + e.what();
//        BOOST_FAIL(message);
//    }
//    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SYMBOL_EOF);
//    //BOOST_CHECK_EQUAL(tokenizer.nextString(), string("123"));
//    //BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
//    //BOOST_CHECK_EQUAL(tokenizer.nextString(), string("1234567"));
//}

BOOST_AUTO_TEST_CASE(nastran_short_2_line_incomplete_line) {
    string nastranLine = "$comment comment \nKEYWORD 12345\n        01234567";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    //comments are skipped
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), string("KEYWORD"));
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), string("12345"));
    for (int i = 0; i < 7; i++) {
        BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
        BOOST_CHECK_EQUAL(tokenizer.nextString(true,""), "");
    }
    //newline
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), "01234567");
    //end
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_EOF);

}

BOOST_AUTO_TEST_CASE(nastran_short_with_blank_at_end) {
    string nastranLine =
            "KEYWORD1       0       1       2       3       4       5       6       7                    \nKEYWORD2";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    //comments are skipped
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), string("KEYWORD1"));
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
    for (int i = 0; i < 7; i++) {
        BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
        BOOST_CHECK_EQUAL(tokenizer.nextString(), to_string(i));
    }
    //next keyword
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), string("KEYWORD2"));
    //end
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_EOF);
}

//supported only if continuation line is contiguous
BOOST_AUTO_TEST_CASE(nastran_long_format_explicit_cont) {
    string nastranLine =
            "*KEYWORD0123456789      0123456789      0123456789      0123456789       +\n+       0123456789";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), string("KEYWORD"));
    for (int i = 0; i < 5; i++) {
        BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
        BOOST_CHECK_EQUAL(tokenizer.nextInt(), 123456789);
    }
    //end
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_EOF);
}

BOOST_AUTO_TEST_CASE(nastran_long_format_auto_cont) {
    string nastranLine =
            "*KEYWORD0123456789      0123456789      0123456789      0123456789\n        12345678 9";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    string curr_keyword = tokenizer.nextString();
    BOOST_CHECK_EQUAL(curr_keyword, string("KEYWORD"));
    for (int i = 0; i < 4; i++) {
        BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
        BOOST_CHECK_EQUAL(tokenizer.nextInt(), 123456789);
    }
    //second line is short format
    BOOST_CHECK_EQUAL(tokenizer.nextInt(), 12345678);
    BOOST_CHECK_EQUAL(tokenizer.nextInt(), 9);
    //end
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_EOF);
}

BOOST_AUTO_TEST_CASE(nastran_free_format_continuation_long) {
    string nastranLine = "KEYWORD,89,89,89,89,89,89,89,89\n*,89";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL(tokenizer.nextString(), string("KEYWORD"));
    for (int i = 0; i < 9; i++) {
        BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
        BOOST_CHECK_EQUAL(tokenizer.nextInt(), 89);
    }
    //end
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_EOF);
}

BOOST_AUTO_TEST_CASE(nastran_free_format_continuation_short) {
    string nastranLine = "KEYWORD,0,1,2,3,4,5,6,7\n,8";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    tokenizer.nextString();
    for (int i = 0; i < 9; i++) {
        BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
        BOOST_CHECK_EQUAL(tokenizer.nextInt(), i);
    }
    //end
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_EOF);
}

BOOST_AUTO_TEST_CASE(is_next_double) {
    // Quick Reference Guide :
    // Real numbers may be entered in a variety of ways. For example, the following are all
    // acceptable versions of the real number seven:
    string nastranLine = "keyword, 7.0,.7E1,0.7+1,.70+1,7.E+0,70.-1";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    tokenizer.nextLine();
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
    tokenizer.nextLine();
    BOOST_CHECK(!tokenizer.isNextEmpty(2));
    BOOST_CHECK_EQUAL(tokenizer.nextString(), "KEYWORD");
    BOOST_CHECK(tokenizer.isNextDouble());
    BOOST_CHECK(tokenizer.nextDouble());
}

BOOST_AUTO_TEST_CASE(nastran_free_format_continuation_symbol) {
    string nastranLine = "chexa, 0, 1, 2, 3, 4, 5, 6, 7, +HX1\n+HX1, 8, 9";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL("CHEXA", tokenizer.nextString());
    for (int i = 0; i < 10; i++) {
        BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_FIELD);
        BOOST_CHECK_EQUAL(tokenizer.nextInt(), i);
    }
    //end
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_EOF);
}

BOOST_AUTO_TEST_CASE(nastran_auto_contiuation_short_incomplete_line) {
    string nastranLine =
    //12345678|2345678|2345678|2345678|
            "PBARL   1       2               BAR\n"
                    "        2.      4.\n$\n$\nTEST";
    istringstream istr(nastranLine);
    NastranTokenizer tok(istr);
    tok.bulkSection();
    tok.nextLine();
    BOOST_CHECK(tok.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL("PBARL", tok.nextString());
    int elemId = tok.nextInt();
    BOOST_CHECK_EQUAL(1, elemId);
    int material_id = tok.nextInt();
    BOOST_CHECK_EQUAL(2, material_id);
    BOOST_CHECK(tok.isNextEmpty(1));
    BOOST_CHECK(!tok.isNextEmpty(2));
    string group = tok.nextString(true);
    string type = tok.nextString();
    BOOST_CHECK_EQUAL("BAR", type);
    tok.skip(4);
    double width = tok.nextDouble();
    BOOST_CHECK_EQUAL(2., width);
    double height = tok.nextDouble();
    BOOST_CHECK_EQUAL(4., height);
    BOOST_CHECK(tok.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    tok.nextLine();
    BOOST_CHECK_EQUAL("TEST", tok.nextString());
}

BOOST_AUTO_TEST_CASE(nastran_THRU_symbol) {
    string nastranLine = "SPC1    20      246     2       THRU    7                                       ";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL("SPC1", tokenizer.nextString());
    BOOST_CHECK(tokenizer.isNextInt());
    BOOST_CHECK_EQUAL(20, tokenizer.nextInt());
    BOOST_CHECK(tokenizer.isNextInt());
    BOOST_CHECK_EQUAL(246, tokenizer.nextInt());
    BOOST_CHECK(tokenizer.isNextInt());
    list<int> ids = tokenizer.nextInts();
    BOOST_CHECK_EQUAL(6, ids.size());
    BOOST_CHECK_EQUAL(2, ids.front());
    BOOST_CHECK_EQUAL(7, ids.back());
    //end
    tokenizer.nextLine();
}

BOOST_AUTO_TEST_CASE(nastran_THRU_symbol_back) {
    string nastranLine = "SPC1    20      246     7       THRU    2                                       ";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL("SPC1", tokenizer.nextString());
    BOOST_CHECK(tokenizer.isNextInt());
    BOOST_CHECK_EQUAL(20, tokenizer.nextInt());
    BOOST_CHECK(tokenizer.isNextInt());
    BOOST_CHECK_EQUAL(246, tokenizer.nextInt());
    BOOST_CHECK(tokenizer.isNextInt());
    list<int> ids = tokenizer.nextInts();
    BOOST_CHECK_EQUAL(6, ids.size());
    BOOST_CHECK_EQUAL(2, ids.front());
    BOOST_CHECK_EQUAL(7, ids.back());
    //end
    tokenizer.nextLine();
}

BOOST_AUTO_TEST_CASE(nastran_THRU_BY_symbol) {
    //                    1234567812345678123456781234567812345678123456781234567812345678
    string nastranLine = "SPC1    20      246     2       THRU    7       BY       2                      ";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL("SPC1", tokenizer.nextString());
    BOOST_CHECK(tokenizer.isNextInt());
    BOOST_CHECK_EQUAL(20, tokenizer.nextInt());
    BOOST_CHECK(tokenizer.isNextInt());
    BOOST_CHECK_EQUAL(246, tokenizer.nextInt());
    BOOST_CHECK(tokenizer.isNextInt());
    list<int> ids = tokenizer.nextInts();
    BOOST_CHECK_EQUAL(4, ids.size());
    BOOST_CHECK_EQUAL(2, ids.front());
    BOOST_CHECK_EQUAL(7, ids.back());
    //end
    tokenizer.nextLine();
}

BOOST_AUTO_TEST_CASE(nastran_THRU_BY_symbol2) {
    //                    $  1   ><  2   ><  3   ><  4   ><  5   ><  6   ><  7   ><  8   ><  9   >
    string nastranLine = "SPC1    4       3       1       THRU    11";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    tokenizer.nextLine();
    BOOST_CHECK(tokenizer.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL("SPC1", tokenizer.nextString());
    BOOST_CHECK(tokenizer.isNextInt());
    BOOST_CHECK_EQUAL(4, tokenizer.nextInt());
    BOOST_CHECK(tokenizer.isNextInt());
    BOOST_CHECK_EQUAL(3, tokenizer.nextInt());
    BOOST_CHECK(tokenizer.isNextInt());
    list<int> ids = tokenizer.nextInts();
    BOOST_CHECK_EQUAL(11, ids.size());
    BOOST_CHECK_EQUAL(1, ids.front());
    BOOST_CHECK_EQUAL(11, ids.back());
    //end
    tokenizer.nextLine();
}

BOOST_AUTO_TEST_CASE(nastran_HM_comment) {
    //                    12345678123456781234567812345678 1234567812345 6781234567812345678
    string nastranLine = "$HMNAME LOADSTEP               2\"Lateral_load\"       1";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    tokenizer.nextLine();
    BOOST_CHECK_EQUAL(1, tokenizer.labelByCommentTypeAndId.size());
}

BOOST_AUTO_TEST_CASE(nastran_HM_nonint_comment) {
    //                    12345678123456781234567812345678 1234567812345 6781234567812345678
    string nastranLine = "$HMNAME BEAMSECTS  RESULTS_AREA 28.27433";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.bulkSection();
    tokenizer.nextLine();
    BOOST_CHECK_EQUAL(0, tokenizer.labelByCommentTypeAndId.size());
}

BOOST_AUTO_TEST_CASE(nastran_DESOPT_ANALYSIS) {
    //                    12345678123456781234567812345678 1234567812345 6781234567812345678
    string nastranLine = "ANALYSIS STATICS";
    istringstream istr(nastranLine);
    NastranTokenizer tokenizer(istr);
    tokenizer.executiveControlSection();
    tokenizer.nextLine();
    BOOST_CHECK_EQUAL(2, tokenizer.currentDataLine().size());
    BOOST_CHECK_EQUAL("ANALYSIS", tokenizer.currentDataLine()[0]);
    BOOST_CHECK_EQUAL("STATICS", tokenizer.currentDataLine()[1]);
}

BOOST_AUTO_TEST_CASE(nastran_two_CPENTA) {
    string nastranLine =
           //12345678|2345678|2345678|2345678|
            "CPENTA    125505       2     239     294     320     714     769     795\n"
            "        \n"
            "CPENTA    125506       2     239     320     371     714     795     846\n"
            "        \n"
            ;
    istringstream istr(nastranLine);
    NastranTokenizer tok(istr);
    tok.bulkSection();
    tok.nextLine();
    BOOST_CHECK(tok.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL("CPENTA", tok.nextString());
    BOOST_CHECK_EQUAL(125505, tok.nextInt());
    BOOST_CHECK_EQUAL(2, tok.nextInt());
    BOOST_CHECK_EQUAL(6, tok.nextInts().size());
    //BOOST_CHECK(tok.isNextEmpty());
    BOOST_TEST_CHECKPOINT("Reading next CPENTA line");
    tok.nextLine();
    BOOST_CHECK(tok.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL("CPENTA", tok.nextString());
    BOOST_CHECK_EQUAL(125506, tok.nextInt());
    BOOST_CHECK_EQUAL(2, tok.nextInt());
    BOOST_CHECK_EQUAL(6, tok.nextInts().size());
    //BOOST_CHECK(tok.isNextEmpty());
}

BOOST_AUTO_TEST_CASE(nastran_comment_inside_card) {
    string nastranLine =
           //12345678|2345678|2345678|2345678|
           "TABLED1      801  LINEAR  LINEAR       1\n"
           "$ +           50.0     1.0  1500.0     1.0    ENDT\n"
           "+           51.0     1.0  2500.0     1.0    ENDT\n"
           ;
    istringstream istr(nastranLine);
    NastranTokenizer tok(istr);
    tok.bulkSection();
    tok.nextLine();
    BOOST_CHECK(tok.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL("TABLED1", tok.nextString());
    BOOST_CHECK_EQUAL(801, tok.nextInt());
    BOOST_CHECK_EQUAL("LINEAR", tok.nextString());
    BOOST_CHECK_EQUAL("LINEAR", tok.nextString());
    BOOST_CHECK_EQUAL(1, tok.nextInt());
    BOOST_CHECK_EQUAL(51, tok.nextDouble());//BOOST_CHECK(tok.isNextEmpty());
}

BOOST_AUTO_TEST_CASE(nastran_cord2c_multiline) {
    string nastranLine =
          //12345678|2345678|2345678|2345678|2345678|2345678|2345678|2345678|2345678|2345678|
           "CORD2C      5001          8.9553     0.0     0.0108.9553     0.0     0.0+       \n"
           "+         8.9553   100.0     0.0\n"
           ;
    istringstream istr(nastranLine);
    NastranTokenizer tok(istr);
    tok.bulkSection();
    tok.nextLine();
    BOOST_CHECK(tok.nextSymbolType == NastranTokenizer::SymbolType::SYMBOL_KEYWORD);
    BOOST_CHECK_EQUAL("CORD2C", tok.nextString());
    BOOST_CHECK_EQUAL(5001, tok.nextInt());
    BOOST_CHECK(tok.isNextEmpty());tok.skip(1);
    BOOST_CHECK_EQUAL(8.9553, tok.nextDouble());
    BOOST_CHECK_EQUAL(0.0, tok.nextDouble());
    BOOST_CHECK_EQUAL(0.0, tok.nextDouble());
    BOOST_CHECK_EQUAL(108.9553, tok.nextDouble());
    BOOST_CHECK_EQUAL(0.0, tok.nextDouble());
    BOOST_CHECK_EQUAL(0.0, tok.nextDouble());
    BOOST_CHECK_EQUAL(8.9553, tok.nextDouble());
    BOOST_CHECK_EQUAL(100.0, tok.nextDouble());
    BOOST_CHECK_EQUAL(0.0, tok.nextDouble());
}
