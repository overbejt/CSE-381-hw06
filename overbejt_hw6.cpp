/* 
 * File:   overbejt_hw6.cpp
 * Author: Josh Overbeck
 * Copyright (c) 2019 overbejt@miamioh.edu
 * Description:  This is for homework assignment 6 in systems II.  
 *
 * Created on October 21, 2019, 2:14 PM
 */

#include <cctype>
#include <algorithm>
//  #include <ext/stdio_filebuf.h>
//  #include <unistd.h>
//  #include <sys/wait.h>
//  #include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <thread>
#include <unordered_map>

// Using namespaces to save screen space 
using namespace std;
//  using namespace boost::asio;
//  using namespace boost::asio::ip;
//  using TcpStreamPtr = std::shared_ptr<tcp::iostream>;
using WordMap = std::unordered_map<std::string, std::string>;

// Declaring a global variable for the word Map
WordMap wordMap;

// Prototyping methods
void initWordMap();

/**
 * This is a helper method that will populate the word map with the data from 
 * the file english.txt.
 */
void initWordMap() {
    ifstream is;
    is.open("english.txt", std::ifstream::in);
    string word;
    while (is >> word) {
        wordMap.insert({word, word});
    }
    
    for (const auto ell : wordMap) {
        cout << ell.first << endl;
    }
}  // End of the 'initWordMap' method


/*
 * 
 */
int main(int argc, char** argv) {
    initWordMap();
    return 0;
}

// END OF FILE
