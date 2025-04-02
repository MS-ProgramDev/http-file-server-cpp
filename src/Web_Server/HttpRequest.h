#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include "HttpResponse.h" 

// Using specific types from the std namespace
using std::string;
using std::unordered_map;
using std::unordered_set;


class HttpRequest
{
private:
    // Maximum allowed lengths for method and URI
    static const size_t MAX_METHOD_LENGTH;
    static const size_t MAX_URI_LENGTH;

    // Set of valid HTTP methods
    static const unordered_set<string> validMethods;

    // Member variables to store request details
    string originalRequest;                 // Stores the raw HTTP request (used for TRACE responses)
    string method;                          // HTTP method (e.g., GET, POST, DELETE)
    string uri;                             // The requested URI (e.g., "/index.html")
    string httpVersion;                     // HTTP version (e.g., "HTTP/1.1")
    string body;                            // Request body (used in POST and PUT methods)
    string headerHost;                      // Host header from the request
    string headerLang;                      // Language preference from the request (e.g., "en" or "fr")
    size_t headerContentLength = 0;         // Value of Content-Length header (size of the body in bytes)
    string headerConnection = "keep-alive"; // Connection header value (default is "keep-alive")

public:
    // Constructor
    HttpRequest();

    // Handles the incoming HTTP request by parsing it
    bool handleRequest(const string& http_request);

    // Get the preferred language from the request
    string getLanguage() const { return headerLang; };

    // Handles the HTTP request by dispatching it to the appropriate method handler
    HttpResponse handlePerMethodRequest();

    // Get the value of the Connection header
    string getHeaderConnection() const { return headerConnection; }

private:
    // Parses the headers from the HTTP request
    bool parseHeaders(const string& headers);

    // Checks if the HTTP method is valid
    bool isValidMethod(const string& method);

    // Checks if the URI contains invalid characters
    bool isUriContainsInvalidChars(const string& uri);

    // Parses the Request Line (first line) of the HTTP request
    bool parseRequestLine(string& request, string& method, string& uri, string& httpVersion);

    // Gets a list of supported HTTP methods for the OPTIONS response
    string getSupportedMethods() const;

    // Extracts the file path from the URI
    string extractFilePath() const;

    // Parses the language from the URI (if specified as a query string parameter)
    void parseUriLang();

 
    // Handles GET requests
    HttpResponse handleGetRequest();

    // Handles POST requests
    HttpResponse handlePostRequest();

    // Handles PUT requests
    HttpResponse handlePutRequest();

    // Handles DELETE requests
    HttpResponse handleDeleteRequest();

    // Handles OPTIONS requests
    HttpResponse handleOptionsRequest();

    // Handles TRACE requests
    HttpResponse handleTraceRequest();

    // Handles unsupported HTTP methods
    HttpResponse handleUnsupportedMethod();

    // Handles HEAD requests
    HttpResponse handleHeadRequest();

};
