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
#include <ext/stdio_filebuf.h>
#include <unistd.h>
#include <sys/wait.h>
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <thread>

// Using namespaces to save screen space 
using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
using TcpStreamPtr = std::shared_ptr<tcp::iostream>;

// Prototyping methods

// Named-constants to keep pipe code readable below
const int READ = 0, WRITE = 1;

/** Convenience method to split a given string into words.

    This method is just a copy-paste of example code from lecture
    slides.

    \param[in] str The string to be split

    \return A vector of words
 */
std::vector<std::string> split(const std::string& str) {
    std::istringstream is(str);
    std::string word;
    std::vector<std::string> list;
    while (is >> std::quoted(word)) {
        list.push_back(word);
    }
    return list;
}  // End of the 'split' method

/** Helper method to send the data to client in chunks.
    
    This method is a helper method that is used to send data to the
    client line-by-line.

    \param[in] mimeType The Mime Type to be included in the header.

    \param[in] status The HTTP status code (e.g. "200 OK" or "404 Not
    Found") etc. to be sent to the client.
    
    \param[in] pid An optional PID for the child process.  If it is
    -1, it is ignored.  Otherwise it is used to determine the exit
    code of the child process and send it back to the client.

    \param[in] is The input stream from where the data is to be read.

    \param[out] os The output stream to where the result is to be
    written.
 */
void sendData(const std::string& mimeType, const std::string& status,
        int pid, std::istream& is, std::ostream& os) {
    // First write the fixed HTTP header.
    os << "HTTP/1.1 " << status << "\r\n"
            << "Content-Type: " << mimeType << "\r\n"
            << "Transfer-Encoding: chunked\r\n"
            << "Connection: Close\r\n\r\n";
    // Read line-by line from child-process and write results to
    // client.
    std::string line;
    while (std::getline(is, line)) {
        // Add required "\n" to terminate the line.
        line += "\n";
        // Add size of line in hex
        os << std::hex << line.size() << "\r\n";
        // Write the actual data for the line.
        os << line << "\r\n";
    }
    // Check if we need to end out exit code
    if (pid != -1) {
        // Wait for process to finish and get exit code.
        int exitCode = 0;
        waitpid(pid, &exitCode, 0);
        // Create exit code information and send to client.
        line = "Exit code: " + std::to_string(exitCode);
        os << std::hex << line.size() << "\r\n" << line << "\r\n";
    }
    // Send trailer out to end stream to client.
    os << "0\r\n\r\n";
}  // End of the 'SendData' method

/** Run the specified command and send output back to the user.

    This method runs the specified command and sends the data back to
    the client using chunked-style response.

    \param[in] cmd The command to be executed

    \param[in] args The command-line arguments, wich each one
    separated by one or more blank spaces.

    \param[out] os The output stream to which outputs from child
    process are to be sent.
 */
void exec(std::string cmd, std::string args, std::ostream& os) {
    // Split string into individual command-line arguments.
    std::vector<std::string> cmdArgs = split(args);
    // Add command as the first of cmdArgs as per convention.
    cmdArgs.insert(cmdArgs.begin(), cmd);
    // Setup pipes to obtain inputs from child process
    int pipefd[2];
    pipe(pipefd);
    // Finally fork and exec with parent having more work to do.
    const int pid = fork();
    if (pid == 0) {
        close(pipefd[READ]);  // Close unused end.
        dup2(pipefd[WRITE], 1);  // Tie/redirect std::cout of command
        runChild(cmdArgs);
    } else {
        // In parent process. First close unused end of the pipe and
        // read standard inputs.
        close(pipefd[WRITE]);
        __gnu_cxx::stdio_filebuf<char> fb(pipefd[READ], std::ios::in, 1);
        std::istream is(&fb);
        // Have helper method process the output of child-process
        sendData("text/plain", "200 OK", pid, is, os);
    }
}  // End of the 'exec' method

/** Helper method to send HTTP 404 message back to the client.

    This method is called in cases where the specified file name is
    invalid.  This method uses the specified path and sends a suitable
    404 response back to client.

    \param[out] os The output stream to where the data is to be
    written.

    \param[in] path The file path that is invalid.
 */
void send404(std::ostream& os, const std::string& path) {
    const std::string msg = "Invalid request: " + path;
    std::istringstream is(msg);
    // Use the sendData method to send out the error message
    sendData("text/plain", "404 Not Found", -1, is, os);
}  // End of the 'send404' method

/**
 * This method is a convenience method that extracts file or command
 * from a string of the form: "GET <path> HTTP/1.1"
 * 
 * @param req The request from which the file path is to be extracted.
 * @return The path to the file requested
 */
std::string getFilePath(const std::string& req) {
    // std::cout << req << std::endl;
    size_t spc1 = req.find(' '), spc2 = req.rfind(' ');
    if ((spc1 == std::string::npos) || (spc2 == std::string::npos)) {
        return "";  // Invalid request.
    }
    std::string path = req.substr(spc1 + 2, spc2 - spc1 - 2);
    if (path.empty()) {
        // Check and return name of default file, if root path is empty.
        path = "index.html";
    }
    return path;
}  // End of the 'getFilePath' method

/** Convenience method to decode HTML/URL encoded strings.

    This method must be used to decode query string parameters
    supplied along with GET request.  This method converts URL encoded
    entities in the from %nn (where 'n' is a hexadecimal digit) to
    corresponding ASCII characters.

    \param[in] str The string to be decoded.  If the string does not
    have any URL encoded characters then this original string is
    returned.  So it is always safe to call this method!

    \return The decoded string.
 */
std::string url_decode(std::string str) {
    // Decode entities in the from "%xx"
    size_t pos = 0;
    while ((pos = str.find_first_of("%+", pos)) != std::string::npos) {
        switch (str.at(pos)) {
            case '+': str.replace(pos, 1, " ");
                break;
            case '%':
            {
                std::string hex = str.substr(pos + 1, 2);
                char ascii = std::stoi(hex, nullptr, 16);
                str.replace(pos, 3, 1, ascii);
            }
        }
        pos++;
    }
    return str;
}  // End of the 'url_decode' method

/**
 * Obtain the mime type of data based on file extension.  The file
 * extension is determined from a given path.  For example, given the
 * path "a/b/c.html", this method extracts the suffix "html" and uses
 * that to determine the MIME type.
 * 
 * @param path The path from where the file extension is to be determined.
 * 
 * @return The mime type associated with the contents of the file.  By
 * default this method returns "text/plain" as the MIME type.
 */
std::string getMimeType(const std::string& path) {
    const size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos) {
        const std::string ext = path.substr(dotPos + 1);
        if (ext == "html") {
            return "text/html";
        } else if (ext == "png") {
            return "image/png";
        } else if (ext == "jpg") {
            return "image/jpeg";
        }
    }
    // In all cases return default mime type.
    return "text/plain";
}  // End of the 'getMimeType' method

/**
 * Process HTTP request (from first line & headers) and
 * provide suitable HTTP response back to the client.
 * 
 * @param is The input stream to read data from client.
 * @param os The output stream to send data to client.
 */
void serveClient(std::istream& is, std::ostream& os) {
    // Read headers from client and print them. This server
    // does not really process client headers
    std::string line, getReq;
    // Read the GET request line.
    std::getline(is, getReq);
    // Skip/ignore all the HTTP request & headers for now.
    while (std::getline(is, line) && (line != "\r") && !line.empty()) {
    }
    
    // Check and dispatch the request appropriately.  For now, we
    // assume that the command will always start with a fixed prefix
    // of "cgi-bin/exec?cmd="
    const std::string path = getFilePath(getReq);
    std::cout << "Path = " << path << std::endl;
    const std::string cgiPrefix = "cgi-bin/exec?cmd=";
    const int prefixLen = cgiPrefix.size();
    if (path.substr(0, prefixLen) == cgiPrefix) {
        // This is a command to be executed by this web-server
        // Extract the command and parameters for exec.
        const size_t argsPos = path.find("&args=", prefixLen);
        const std::string cmd = url_decode(path.substr(prefixLen,
                argsPos - prefixLen));
        // We url_decode the args to process any special characters
        const std::string args = url_decode(path.substr(argsPos + 6));
        // Now run the command and return result back to client.
        exec(cmd, args, os);
    } else {
        // Assume path specified in the GET request is referring to a
        // file. First check and see if the file is valid.
        std::ifstream dataFile(path);
        if (dataFile.good()) {
            // Use helper method to stream contents of file back
            sendData(getMimeType(path), "200 OK", -1, dataFile, os);
        } else {
            // Invalid URL -- invalid file name and it does not start
            // with "cgi-bin/exec?cmd=". So now we send 404
            send404(os, path);
        }
    }
}  // end of the 'url_decode' method
    


/*
 * 
 */
int main(int argc, char** argv) {
    if (argc == 2) {
        // Process 1 request from specified file for functional testing
        std::ifstream input(argv[1]);
        serveClient(input, std::cout);
    } else {
        // Run the server on some available port number.
        runServer(0);
    }
    return 0;
}

// END OF FILE
