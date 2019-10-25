/* 
 * File:   overbejt_hw6.cpp
 * Author: Josh Overbeck
 * Copyright (c) 2019 overbejt@miamioh.edu
 * Description:  This is for homework assignment 6 in systems II.  
 *
 * Created on October 21, 2019, 2:14 PM
 */

#include <boost/asio.hpp> 
#include <cctype>
#include <cstdlib>
#include <algorithm>
//  #include <ext/stdio_filebuf.h>
//  #include <unistd.h>
//  #include <sys/wait.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <thread>
#include <unordered_map>
#include <utility>

// Using namespaces to save screen space 
using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
//  using namespace tcp::iostream;  //
//  using TcpStreamPtr = std::shared_ptr<tcp::iostream>;
using WordMap = std::unordered_map<std::string, std::string>;
using StrVec = std::vector<std::string>;

// Declaring a global variables
WordMap wordMap;
StrVec data;

// Prototyping methods
void initWordMap();
int scrapeUrl(int index);
void printCounts();
pair<int, int> countWords(string line);
//  bool setupHttpStream(tcp::iostream& stream, const std::string& path,
//                     const std::string& host = "ceclnx01.cec.miamioh.edu");

/**
 * Helper method to create an TCP I/O stream to send an HTTP request
 * to a web server and obtain file contents as response.  Note that
 * this method does not process the response. However, it reads and
 * discards the HTTP response headers sent by the server.
 *
 * @param path The path to the file on the web server to be
 * obtained. E.g. "~raodm/test.html".
 *
 * @param host An optional host name for the web server. The default
 * host is currently assumed to be ceclnx01.cec.miamioh.edu.
 */
bool setupHttpStream(tcp::iostream& stream, const std::string& path,
                     const std::string& host = "ceclnx01.miamioh.edu") {
    // Establish a TCP connection to the given web server at port
    // number 80.
    stream.connect(host, "80");
    if (!stream.good()) {
        return false;  // invalid connection. Nothing further to do.
    }

    // Send HTTP request to the web server requesting the desired
    // file.  This is the same GET request that a browser generates to
    // obtain the file from the server.
    stream << "GET /"  << path << " HTTP/1.1\r\n"
           << "Host: " << host << "\r\n"
           << "Connection: Close\r\n\r\n";

    // Assuming the file is valid, the web server will send us a
    // response.  First, HTTP response code and ensure it is "200 OK"
    // status code indicating things went well at the server.
    std::string line;
    std::getline(stream, line);  // Read a whole line! yes, important.
    if (line.find("200 OK") == std::string::npos) {
        return false;  // Web server reported an error!
    }

    // Next read and discard HTTP response headers. The response
    // headers contain information that is useful for processing the
    // data. However, here we are doing very simple text
    // processing. Consequently, we ignore the response headers.
    while (std::getline(stream, line) && (line != "\r") && !line.empty()) {}

    // Now that we have skipped over the response headers (indicated
    // by a blank line or just a line with "\r") the next information
    // read from the stream will be the actual response data from the
    // server.  We don't do processing of data in this method. We just
    // return true to indicate successful processing up to this point.
    return true;
}

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
    cout << "WordMap initialized." << endl;  // Debuging -=-=-=-=-=-=-=-=-=-=-=-
}  // End of the 'initWordMap' method


/**
 * This is a helper method that will scrape the content from a given Url.  
 * It will count the words, and it will count the English words.
 * 
 * @param url A url that needs to be scrapped.
 */
int scrapeUrl(int index) {
    // local variables
    string url = data.at(index-2);
    tcp::iostream stream;
    string host = "os1.csi.miamioh.edu";
    string path = "~raodm/cse381/hw6/SlowGet.cgi?file=" + url;
    if (!setupHttpStream(stream, path, host)) {
        // Something went wrong in getting the data from the server.
        std::cout << "Error obtaining data from server.\n";
        return 1;  // Unsuccessful run of program (non-zero exit code)
    }
    // Iterate through each line in the web page
    string line;
    int wordCt = 0, englishCt = 0;
    pair<int, int> counts;
    while (stream >> line) {
        // Get the counts
        counts = countWords(line);
        wordCt += counts.first;
        englishCt += counts.second;
    }
    // Concatenate the string containing url and counts
    data.at(index-2) = "http://" + host + "/" + url + ' ' + 
            to_string(wordCt) + ' ' + to_string(englishCt);
    return 0;
}  // End of the 'scrapeUrl' method


/**
 * A helper method that will print out all of the counts in the order that the 
 * url were supplied in.  
 */
void printCounts() {
    for (size_t i = 0; i < data.size(); i++) {
        stringstream ss(data[i]);
        string url, wordCount, englishCount;
//        int wordCount, englishCount;
        ss >> url >> wordCount >> englishCount;
        cout << "URL: " << url;
        cout << ", words: " << wordCount;
        cout << ", English words: " << englishCount << endl;
    }
}  // End of the 'printCounts' method

/**
 * A helper method that will count the number of words and English words in a 
 * string.
 * 
 * @param line A string that needs the words counted.
 * @return A pair containing the count of words and English words.  They will 
 *         be formatted as "words, English words"
 */
pair<int, int> countWords(string line) {
    int wordsCnt = 0, englishCnt = 0;
    // Strip out punctuation
    std::replace_if(line.begin(), line.end(), ::ispunct, ' ');      
    // Iterate through each word in a line
    string word;
    stringstream ss(line);
    while (ss >> word) {
        // Convert to lowercase
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        wordsCnt++;
        if (wordMap.find(word) != wordMap.end()) {
            englishCnt++;
        }
    }
    pair<int, int> counts(wordsCnt, englishCnt);
    return counts;
}  // End of the 'countWords' method


/*
 * 
 */
int main(int argc, char** argv) {
//    StrVec data;
    // Fill the word map
    initWordMap();

    // Fill the vector with he URLs
    for (int i = 2; i < argc; i++) {
        string meh = argv[i];
        data.push_back(argv[i]);
    }
    // Debuging -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (auto b : data) {
        cout << b << endl;
    }
    // Scrape the URLs
    for (int i = 2; i < argc; ++i) {
        int status = scrapeUrl(i);
    }
    
    // Print out the counts
    printCounts();
    return 0;
}

// END OF FILE
