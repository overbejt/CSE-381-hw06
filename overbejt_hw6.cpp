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
using ThrVec = std::vector<std::thread>;

// Declaring a global variables
WordMap wordMap;
StrVec data;

// Prototyping methods
void initWordMap();
void scrapeUrl(int index);
void printCounts();
pair<int, int> countWords(string line);
void thrdMain(int thrdCnt);
void thrdSet(ThrVec& threads, int interval, int begin);

/**
 * Helper method to create an TCP I/O stream to send an HTTP request
 * to a web server and obtain file contents as response.  Note that
 * this method does not process the response. However, it reads and
 * discards the HTTP response headers sent by the server.
 * 
 * [This method came directly from exercise 1]
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
}  // End of the 'initWordMap' method


/**
 * This is a helper method that will scrape the content from a given Url.  
 * It will count the words, and it will count the English words.
 * 
 * @param index The index of the url that needs to be scrapped.
 */
void scrapeUrl(int index) {
    // local variables
    string url = data.at(index);
    tcp::iostream stream;
    string host = "os1.csi.miamioh.edu";
    string path = "~raodm/cse381/hw6/SlowGet.cgi?file=" + url;
    if (!setupHttpStream(stream, path, host)) {
        // Something went wrong in getting the data from the server.
        std::cout << "Error obtaining data from server.\n";
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
    data.at(index) = "http://" + host + "/" + path + ' ' + 
            to_string(wordCt) + ' ' + to_string(englishCt);
}  // End of the 'scrapeUrl' method


/**
 * A helper method that will print out all of the counts in the order that the 
 * url were supplied in.  
 */
void printCounts() {
    for (size_t i = 0; i < data.size(); i++) {
        stringstream ss(data[i]);
        string url, wordCount, englishCount;
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

/**
 * This is a helper method that will iterate tasks to threads.
 * 
 * @param interval The interval that this thread will work on.
 * @param threads A reference to the thread list from  the 'thrdMain' method.
 * @param begin The index in the StrVec data that this thread begins working on.
 */
void thrdSet(ThrVec& threads, int interval, int begin) {
    // Specify the end of the loop
    int end = interval + begin;
    // Loop and make threads work
    int increment = 0;
    for (int i = begin; i < end; ++i) {
        threads.push_back(thread(scrapeUrl, (begin + increment)));
        increment++;
    }
}  // End of the 'thrdSet' method

/**
 * A helper method to manage the threads.
 * 
 * @param thrdCnt A count of the threads to use.
 */
void thrdMain(int thrdCnt) {
    if (thrdCnt > 1) {
        ThrVec thrList;
        // 1.) Find the interval for each thread to work on.
        int interval = data.size() / thrdCnt;
        int remainder = data.size() % thrdCnt;
        // 2.) Loop and thread. 
        int start = 0;
        for (int i = 0; i < thrdCnt; ++i) {
            // On the last round, include the remainder in the interval
            if (i == thrdCnt - 1) {
                interval = interval + remainder;
            }
            // Send the thread to work
            thrdSet(thrList, interval, start);
            start += interval;
        }
        // 3.) Join the threads.
        for (auto& t : thrList) {
            t.join();
        }
    } else {
        // Scrape the URLs, single thread
        for (int i = 0; i < data.size(); ++i) {
            scrapeUrl(i);
        }
    }
}  // End of the 'thrdMain' method


/*
 * 
 */
int main(int argc, char** argv) {        
    // Fill the word map
    initWordMap();

    // Fill the vector with he URLs
    for (int i = 2; i < argc; i++) {
        string meh = argv[i];
        data.push_back(argv[i]);
    }
        
    // Run Thread Main
    int thrCnt = atoi(argv[1]);
    thrdMain(thrCnt);
        
    // Print out the counts
    printCounts();
    return 0;
}

// END OF FILE
