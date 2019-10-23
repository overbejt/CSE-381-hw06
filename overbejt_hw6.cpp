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
void scrapeUrl(char** &list, string url);
void printCounts(char** &list);

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
}  // End of the 'initWordMap' method


/**
 * This is a helper method that will scrape the content from a given Url.  
 * It will count the words, and it will count the English words.
 * 
 * @param list Argv from main, put the counts in the order 
 *             (url word_count English_count).  *Without the parenthesis.
 * 
 * @param url A url that needs to be scrapped.
 */
void scrapeUrl(char** &list, string url) {    
}  // End of the 'scrapeUrl' method


/**
 * A helper method that will print out all of the counts in the order that the 
 * url were supplied in.  
 * 
 * @param list Argv from main.  It will contain the counts like 
 *             "url word_count English_count".  *Without the quotes.
 */
void printCounts(char** &list) {
    for (int i = 2; i < list.size(); i++) {
        stringstream ss(list[i]);
        string url;
        int wordCount, englishCount;
        ss >> url >> wordCount >> englishCount;
        cout << "URL: " << url;
        cout << ", words: " << wordCount;
        cout << ", English words: " << englishCount << endl;
    }
}  // End of the 'printCounts' method


/*
 * 
 */
int main(int argc, char** argv) {
    // Fill the word map
    initWordMap();
    // Scrape the url
    // Print out the counts
    return 0;
}

// END OF FILE
