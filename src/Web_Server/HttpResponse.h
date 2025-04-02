#pragma once

#include <string>

using std::string;

class HttpResponse
{
private:
    string httpVersion;         // HTTP version
    int statusCode;             // Status code
    string statusMessage;       // Status message
    string headerContentType;   // Content-Type header value
    size_t headerContentLength; // Content-Length header value 
    string headerConnection;    // Connection header value
    string allow;               // Allow header value
    string body;                // The response body content


public:
    // Constructors
    HttpResponse();
    HttpResponse(int code, const string& message);

    // Setters for response properties
    void setStatus(int code, const string& message); // Set status code and message
    void setContentType(const string& type); // Set Content-Type header
    void setContentLength(size_t length); // Set Content-Length header
    void setAllow(const string& methods); // Set Allow header for supported methods
    void setBody(const string& content); // Set the response body and update Content-Length
    void setConnection(const string& connection); // Set Connection header

    // Static methods to create standard HTTP responses
    static HttpResponse createBadRequestResponse(); // Create 400 Bad Request response
    static HttpResponse createNotFoundResponse(); // Create 404 Not Found response
    static HttpResponse createMethodNotAllowedResponse(const string& allowedMethods); // Create 405 Method Not Allowed response
    static HttpResponse createNotImplementedResponse(); // Create 501 Not Implemented response
    static HttpResponse createInternalErrorResponse(); // Create 500 Internal Server Error response


    // Methods to handle specific HTTP verbs
    // OPTIONS
    static HttpResponse createOptionsResponse(const string& supportedMethods);
    // GET
    static HttpResponse createGetResponse(const string& filePath);
    // HEAD
    static HttpResponse createHeadResponse(const string& fileName);
    // POST
    static HttpResponse createPostResponse(const string& requestBody);
    // PUT
    static HttpResponse createPutResponse(const string& fileName, const string& requestBody);
    // DELETE
    static HttpResponse createDeleteResponse(const string& fileName);
    // TRACE
    static HttpResponse createTraceResponse(const string& originalRequest);

    // Utility methods to read file content
    static string readFileContent_without_temp(const string& filePath); // Read file content directly from given file path
    static string readFileContent(const string& fileName); // Read file content from the default directory (e.g., "C:\\temp")


    // Function to convert the response to a string format
    string toString() const;
};
