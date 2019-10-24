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

// Using namespaces to save screen space 
using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
//  using namespace tcp::iostream;  //
//  using TcpStreamPtr = std::shared_ptr<tcp::iostream>;
using WordMap = std::unordered_map<std::string, std::string>;

// Declaring a global variable for the word Map
WordMap wordMap;

// Prototyping methods
void initWordMap();
void scrapeUrl(char** &list, string url, int index);
void printCounts(int &argc, char** &list);

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
                     const std::string& host = "ceclnx01.cec.miamioh.edu") {
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
 * @param list Argv from main, put the counts in the order 
 *             (url word_count English_count).  *Without the parenthesis.
 * 
 * @param url A url that needs to be scrapped.
 */
void scrapeUrl(char** &list, string url, int index) {
    // local variables
    int words = 0, english = 0;
    tcp::iostream stream;
    // Update path to web page
    string path = "~raodm/cse381/ex6/SlowGet.cgi?file=" + url;
    if (!setupHttpStream(stream, path)) {
        // Something went wrong in getting the data from the server.
        std::cout << "Error obtaining data from server.\n";
//        return 1;  // Unsuccessful run of program (non-zero exit code)
    }
    // Iterate through each line in the web page
    string line;
    while (stream >> line) {
        // Strip out punctuation
        std::replace_if(line.begin(), line.end(), ::ispunct, ' ');      
        // Iterate through each word in a line
        string word;
        stringstream ss(line);
        while (ss >> word) {
            // Convert to lowercase
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            words++;
            if (wordMap.find(word) != wordMap.end()) {
                english++;
            }
        }
    }
    
    // Add the counts to argv
    list[index] += ' ' + words + ' ' + english;
}  // End of the 'scrapeUrl' method


/**
 * A helper method that will print out all of the counts in the order that the 
 * url were supplied in.  
 * 
 * @param list Argv from main.  It will contain the counts like 
 *             "url word_count English_count".  *Without the quotes.
 */
void printCounts(int &argc, char** &list) {
    for (int i = 2; i < argc; i++) {
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
