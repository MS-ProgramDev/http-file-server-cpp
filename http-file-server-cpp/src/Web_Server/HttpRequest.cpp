#include "HttpRequest.h"
#include "HttpResponse.h" 
#include <algorithm>
#include <cctype>
#include <stdexcept> // For std::invalid_argument


// Using specific types from the std namespace
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::stoi;
using std::cout;
using std::endl;

// Initialize static constants
const size_t HttpRequest::MAX_METHOD_LENGTH = 7;
const size_t HttpRequest::MAX_URI_LENGTH = 2048;

// Initialize the static member with valid HTTP methods
const unordered_set<string> HttpRequest::validMethods = {
    "GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "TRACE"
};

// Constructor
HttpRequest::HttpRequest() = default;

// Handles the incoming HTTP request by parsing it
bool HttpRequest::handleRequest(const string& http_request)
{
    originalRequest = http_request; //for the trace
    size_t posHeaders = http_request.find("\r\n");
    if (posHeaders == string::npos)
        return false;

    string requestLine = http_request.substr(0, posHeaders + 2);
    if (!parseRequestLine(requestLine, method, uri, httpVersion))
        return false;

    size_t posBody = http_request.find("\r\n\r\n", posHeaders + 2);
    if (posBody == string::npos)
        return false;

    string headers = http_request.substr(posHeaders + 2, posBody - (posHeaders + 2));
    if (!parseHeaders(headers))
        return false;

    if ((method == "POST" || method == "PUT" || method == "PATCH") && headerContentLength == 0)
        return false;

    if (headerContentLength > 0) {
        body = http_request.substr(posBody + 4, headerContentLength);
    }    
    return true;
}

// Parses the headers from the HTTP request
bool HttpRequest::parseHeaders(const string& headers)
{
    size_t start = 0;

    while (start < headers.length())
    {
        // Find the position of the colon that separates the header name and value
        size_t colonPos = headers.find(':', start);
        if (colonPos == string::npos)
        {
            return false; // Invalid header format
        }

        // Extract the header name
        string headerName = headers.substr(start, colonPos - start);

        // Find the start of the header value (skip spaces after the colon)
        size_t valueStart = headers.find_first_not_of(' ', colonPos + 1);
        if (valueStart == string::npos)
        {
            return false; // Invalid header value
        }

        // Find the end of the header line
        size_t endPos = headers.find("\r\n", valueStart);
        if (endPos == string::npos)
        {
            endPos = headers.length();
        }

        // Extract the header value
        string headerValue = headers.substr(valueStart, endPos - valueStart);

        // Handle specific headers
        if (headerName == "Host")
        {
            headerHost = headerValue;
        }
        else if (headerName == "Content-Length")
        {
            try
            {
                headerContentLength = stoi(headerValue);
            }
            catch (const std::invalid_argument&)
            {
                return false; // Invalid Content-Length value
            }
        }
        else if (headerName == "Connection")
        {
            headerConnection = headerValue; // Store the Connection header value
        }

        // Move to the next header
        start = endPos + 2;
    }
    if(headerConnection.empty())
        headerConnection = "keep-alive";


    // Ensure the Host header is present
    return !headerHost.empty();
}


// Checks if the HTTP method is valid
bool HttpRequest::isValidMethod(const string& method)
{
    return validMethods.find(method) != validMethods.end();
}

// Checks if the URI contains invalid characters
bool HttpRequest::isUriContainsInvalidChars(const string& uri)
{
    for (char ch : uri) {
        if (ch == ' ' || ch == '"' || ch == '\\') {
            return true;
        }
    }
    return false;
}

// Parses the Request Line of the HTTP request
bool HttpRequest::parseRequestLine(string& request, string& method, string& uri, string& httpVersion)
{
    size_t pos = request.find(' ');
    if (pos == string::npos || pos > MAX_METHOD_LENGTH)
        return false;

    string found_method = request.substr(0, pos);
    if (!isValidMethod(found_method))
        return false;
    method = found_method;

    size_t start_pos_uri = pos + 1;
    if (request[start_pos_uri] != '/')
        return false;

    size_t pos_end_url = request.find(' ', start_pos_uri);
    if (pos_end_url == string::npos || pos_end_url - start_pos_uri > MAX_URI_LENGTH)
        return false;

    string found_uri = request.substr(start_pos_uri, pos_end_url - start_pos_uri);

    found_uri.erase(std::remove(found_uri.begin(), found_uri.end(), '\n'), found_uri.end());
    found_uri.erase(std::remove(found_uri.begin(), found_uri.end(), '\r'), found_uri.end());

    if (isUriContainsInvalidChars(found_uri))
        return false;
    uri = found_uri;

    size_t pos_start_version = pos_end_url + 1;
    size_t pos_end_version = request.find("\r\n", pos_start_version);
    if (pos_end_version == string::npos)
        return false;

    string found_version = request.substr(pos_start_version, pos_end_version - pos_start_version);
    if (found_version != "HTTP/1.1")
        return false;

    httpVersion = found_version;

    parseUriLang();
    return true;
}

string HttpRequest::getSupportedMethods() const
{
    //std::cout << "Supported methods size: " << validMethods.size() << std::endl;
    size_t index = 0;
    string methods;
    
    for (auto it = validMethods.begin(); it != validMethods.end(); ++it, ++index)
    {
        if (index > 0)
            methods += ", "; // Add a comma after the first method
        methods += *it; // Add the method to the string
        std::cout << *it << " "; // Print the method
    }
    /*
    for (const auto& method : validMethods)
    {
        if (!methods.empty())
            methods += ", ";
        methods += method;
    }
    */
    //std::cout << "Supported methods: " << methods << std::endl;
    return methods;
}



void HttpRequest::parseUriLang()
{
    // Find the start of the query string
    size_t queryStart = uri.find("?lang=");
    if (queryStart != string::npos && queryStart + 6 < uri.size())
    {
        // Extract the language value (2 characters after ?lang=)
        string langValue = uri.substr(queryStart + 6, 2);

        // Check if the language is supported
        if (langValue == "he" || langValue == "en" || langValue == "fr")
        {
            headerLang = langValue; // Update the language field
        }
        // Remove the query string from the URI
        uri = uri.substr(0, queryStart);
        cout << uri;
    }
}


string HttpRequest::extractFilePath() const
{
    string rootDirectory = "C:\\temp\\";
    string adjustedFilePath = uri; // Start with the requested URI

    // Default language to English if not set
    string effectiveLanguage = headerLang.empty() ? "en" : headerLang;

    if (!adjustedFilePath.empty() && adjustedFilePath[0] == '/')
    {
        adjustedFilePath = adjustedFilePath.substr(1);
    }

    //(_en, _he, _fr)
    size_t langPos = adjustedFilePath.find_last_of('_');
    size_t dotPos = adjustedFilePath.find_last_of('.');

    if (langPos != string::npos && dotPos != string::npos && langPos < dotPos)
    {
        string existingLang = adjustedFilePath.substr(langPos + 1, dotPos - langPos - 1);
        if (existingLang == "en" || existingLang == "he" || existingLang == "fr")
        {
            string fullPath = rootDirectory + adjustedFilePath;
            cout << "Full Path: " << fullPath << endl; // Debugging output
            return fullPath;
        }
    }
    // Append the language suffix before the extension
    if (dotPos != string::npos)
    {
        // Insert the language suffix before the extension
        adjustedFilePath = adjustedFilePath.substr(0, dotPos) + "_" + effectiveLanguage + adjustedFilePath.substr(dotPos);
    }
    else
    {
        // If no extension exists, simply append the language suffix
        adjustedFilePath += "_" + effectiveLanguage;
    }

    string fullPath = rootDirectory + adjustedFilePath;
    cout << "Full Path: " << fullPath << endl; // Debugging output
    return fullPath;
}
    /*
    // Append the language suffix before the extension
    size_t dotPos = adjustedFilePath.find_last_of('.');
    if (dotPos != string::npos)
    {
        // Insert the language suffix before the extension
        adjustedFilePath = adjustedFilePath.substr(0, dotPos) + "_" + effectiveLanguage + adjustedFilePath.substr(dotPos);
    }
    else
    {
        // If no extension exists, simply append the language suffix
        adjustedFilePath += "_" + effectiveLanguage;
    }

    string fullPath = rootDirectory + adjustedFilePath;
    cout << fullPath;
    return fullPath;
}
*/

HttpResponse HttpRequest::handleGetRequest()
{
    // Extract the file path based on the language
    string filePath = extractFilePath();

    // Use static response creation function to generate the response
    return HttpResponse::createGetResponse(filePath);
}


// Handles HEAD requests
HttpResponse HttpRequest::handleHeadRequest()
{
    string filePath = extractFilePath();
    return HttpResponse::createHeadResponse(filePath);
}

// Handles POST requests
HttpResponse HttpRequest::handlePostRequest() 
{
    return HttpResponse::createPostResponse(body);
}

// Handles PUT requests
HttpResponse HttpRequest::handlePutRequest()
{
    string filePath = uri;
    return HttpResponse::createPutResponse(filePath, body);
}

// Handles DELETE requests
HttpResponse HttpRequest::handleDeleteRequest()
{
    string filePath = uri;
    return HttpResponse::createDeleteResponse(filePath);
}

// Handles OPTIONS requests
HttpResponse HttpRequest::handleOptionsRequest()
{
   // string supportedMethods = getSupportedMethods();
  // Manually define the supported methods
    string supportedMethods = "GET, POST, PUT, DELETE, OPTIONS, TRACE, HEAD";
    return HttpResponse::createOptionsResponse(supportedMethods);
}

// Handles TRACE requests
HttpResponse HttpRequest::handleTraceRequest()
{
    return HttpResponse::createTraceResponse(originalRequest); 
}

// Handles unsupported HTTP methods
HttpResponse HttpRequest::handleUnsupportedMethod()
{
    string allowedMethods = getSupportedMethods();
    cout << "got here";
    return HttpResponse::createMethodNotAllowedResponse(allowedMethods);
}






// Handles the HTTP request and returns the appropriate HttpResponse
HttpResponse HttpRequest::handlePerMethodRequest()
{
    if (method == "GET")
    {
        return handleGetRequest();
    }
    else if (method == "HEAD")
    {
        return handleHeadRequest();
    }
    else if (method == "POST")
    {
        return handlePostRequest();
    }
    else if (method == "PUT")
    {
        return handlePutRequest();
    }
    else if (method == "DELETE")
    {
        return handleDeleteRequest();
    }
    else if (method == "OPTIONS")
    {
        return handleOptionsRequest();
    }
    else if (method == "TRACE")
    {
        return handleTraceRequest();
    }
    else
    {
        return handleUnsupportedMethod();
    }
}

