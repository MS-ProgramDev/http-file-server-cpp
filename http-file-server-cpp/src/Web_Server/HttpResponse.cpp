#include "HttpResponse.h"
#include <sstream>
#include <fstream>
#include <iostream>

using std::ifstream;
using std::ostringstream;
using std::to_string;


// Constructors
HttpResponse::HttpResponse()
    : httpVersion("HTTP/1.1"), statusCode(200), statusMessage("OK"),
    headerContentType(""), headerContentLength(0), allow(""), body("") {
}

HttpResponse::HttpResponse(int code, const string& message)
    : httpVersion("HTTP/1.1"), statusCode(code), statusMessage(message),
    headerContentType(""), headerContentLength(0), allow(""), body("") {
}


// Setters
void HttpResponse::setStatus(int code, const string& message)
{
    statusCode = code;
    statusMessage = message;
}

void HttpResponse::setContentType(const string& type)
{
    headerContentType = type;
}

void HttpResponse::setContentLength(size_t length)
{
    headerContentLength = length;
}

void HttpResponse::setBody(const string& content)
{
    body = content;
    setContentLength(content.size());
}

void HttpResponse::setAllow(const string& methods)
{
    allow = methods; 
}

void HttpResponse::setConnection(const string& connection)
{
    headerConnection = connection;
}

// Utility function to read file content
string HttpResponse::readFileContent(const string& fileName)
{
    // Construct the full path for the file in C:\temp
    const string filePath = "C:\\temp\\" + fileName;
    // Open the file for reading
    ifstream file(filePath);
    if (!file.is_open())
    {
        // Return an empty string if the file cannot be opened
        return "";
    }

    // Read the entire file content into a string stream
    ostringstream buffer;
    buffer << file.rdbuf();

    // Convert the buffer to a string and return
    return buffer.str();
}

string HttpResponse::readFileContent_without_temp(const string& filePath)
{
    // Construct the full path for the file in C:\temp
    //const string filePath = "C:\\temp\\" + fileName;
    // Open the file for reading
    ifstream file(filePath);
    if (!file.is_open())
    {
        // Return an empty string if the file cannot be opened
        return "";
    }

    // Read the entire file content into a string stream
    ostringstream buffer;
    buffer << file.rdbuf();

    // Convert the buffer to a string and return
    return buffer.str();
}


// Static methods for common responses
HttpResponse HttpResponse::createBadRequestResponse()
{
    HttpResponse response(400, "Bad Request");
    response.setContentType("text/html");
    response.setConnection("close");
    // Read the HTML file content
    string fileContent = readFileContent("bad_request.html");
    if (!fileContent.empty())
    {
        response.setBody(fileContent);
    }
    else
    {
        // Fallback to a default response if the file cannot be read
        response.setBody("<!DOCTYPE html><html><body><h1>400 Bad Request</h1></body></html>");
    }

    return response;
}


HttpResponse HttpResponse::createNotFoundResponse()
{
    HttpResponse response(404, "Not Found");
    response.setContentType("text/html");
    response.setConnection("close");
    // Load the HTML file for 404 response
    string fileContent = readFileContent("not_found.html");
    if (!fileContent.empty())
    {
        response.setBody(fileContent);
    }
    else
    {
        // Fallback content in case the file cannot be read
        response.setBody("<!DOCTYPE html><html><body><h1>404 Page Not Found</h1></body></html>");
    }

    return response;
}


HttpResponse HttpResponse::createMethodNotAllowedResponse(const string& allowedMethods)
{
    ;
    HttpResponse response(405, "Method Not Allowed");
    response.setContentType("text/html");
    response.setAllow(allowedMethods);
    response.setConnection("close");

    // Load the HTML file for 405 response
    string fileContent = readFileContent("method_not_allowed.html");
    if (!fileContent.empty())
    {
        response.setBody(fileContent);
    }
    else
    {
        // Fallback content in case the file cannot be read
        response.setBody("<!DOCTYPE html><html><body><h1>405 Method Not Allowed</h1></body></html>");
    }

    return response;
}


HttpResponse HttpResponse::createNotImplementedResponse()
{
    HttpResponse response(501, "Not Implemented");
    response.setContentType("text/html");
    response.setConnection("close");
    // Load the HTML file for 501 response
    string fileContent = readFileContent("not_implemented.html");
    if (!fileContent.empty())
    {
        response.setBody(fileContent);
    }
    else
    {
        // Fallback content in case the file cannot be read
        response.setBody("<!DOCTYPE html><html><body><h1>501 Not Implemented</h1></body></html>");
    }

    return response;
}

HttpResponse HttpResponse::createInternalErrorResponse()
{
    HttpResponse response(500, "Internal Server Error");
    response.setContentType("text/html");
    response.setConnection("close");
    // Load the HTML file for 500 response
    string fileContent = readFileContent("internal_error.html");
    if (!fileContent.empty())
    {
        response.setBody(fileContent);
    }
    else
    {
        // Fallback content in case the file cannot be read
        response.setBody("<!DOCTYPE html><html><body><h1>500 Internal Server Error</h1><p>An unexpected error occurred on the server.</p></body></html>");
    }

    return response;
}


HttpResponse HttpResponse::createOptionsResponse(const string& supportedMethods)
{
    HttpResponse response(200, "OK");
    response.setContentType("text/html");
    response.setBody(""); // No body required for OPTIONS   
    response.setConnection("keep-alive");
    //std::cout << "Generated response before Allow: " << response.toString() << std::endl;
    response.setAllow(supportedMethods); // Supported methods, e.g., "GET, POST, OPTIONS, TRACE"
   // std::cout << "Generated response AFTER Allow: " << response.toString() << std::endl;

    return response;
}


HttpResponse HttpResponse::createGetResponse(const string& filePath)
{
    HttpResponse response(200, "OK");
    response.setContentType("text/html");
    response.setConnection("keep-alive");
    // Load the requested file
    string fileContent = readFileContent_without_temp(filePath);
    if (!fileContent.empty())
    {
        response.setBody(fileContent);
    }
    else
    {
        // If the file is not found, return a 404 response
        return HttpResponse::createNotFoundResponse();
    }

    return response;
}

HttpResponse HttpResponse::createHeadResponse(const string& filePath)
{
    HttpResponse response(200, "OK");
    response.setContentType("text/html");
    response.setConnection("keep-alive");
    // Load the requested file
    std::cout << filePath;
    string fileContent = readFileContent_without_temp(filePath);
    if (!fileContent.empty())
    {
        response.setContentLength(fileContent.size());
        std::cout << "\nheader Content Length: " << response.headerContentLength << std::endl;
    }
    else
    {
        // If the file is not found, return a 404 response
        return HttpResponse::createNotFoundResponse();
    }

    return response;

}
/*VERSION 2
HttpResponse HttpResponse::createHeadResponse(const string& filePath)
{
    HttpResponse response(200, "OK");
    response.setContentType("text/html");
    response.setConnection("keep-alive");

    // Construct full file path in C:\temp
   // const string filePath = "C:\\temp\\" + fileName;
    std::cout << filePath;
    // Open the file to check if it exists and get its size
    ifstream file(filePath);
    if (!file.is_open())
    {
        // File does not exist, return 404
        return HttpResponse::createNotFoundResponse();
    }


    // Move the file pointer to the end
    file.seekg(0, std::ios::end);

    // Get the current position (file size)
    size_t fileSize = file.tellg();

    file.close();

    // Set Content-Length based on file size
    response.setContentLength(fileSize);

    // No body for HEAD requests
    return response;
}
*/
/*version 1
HttpResponse HttpResponse::createHeadResponse(const string& fileName)
{
    HttpResponse response(200, "OK");
    response.setContentType("text/html");
    response.setConnection("keep-alive");
    // Construct full file path in C:\temp
   //const string filePath = "C:\\temp\\" + fileName;
    // No body for HEAD requests
    string fileContent = readFileContent(fileName);
    if (!fileContent.empty())
    {
        response.setContentLength(fileContent.size());
    }
    else
    {
        // If the file is not found, return a 404 response
        return HttpResponse::createNotFoundResponse();
    }

    return response;
}
*/
HttpResponse HttpResponse::createPostResponse(const string& requestBody)
{
    HttpResponse response(200, "OK"); // Set status to 200 OK
    response.setContentType("text/plain"); // Set content type
    response.setConnection("keep-alive"); // Set connection type

    // File path in the C:\temp directory
    const string filePath = "C:\\temp\\post.txt";

    // Print the request body to the console
    std::cout << "POST Request Body: " << requestBody << std::endl;

    // Open the file in append mode
    std::ofstream file(filePath, std::ios::app);
    if (file.is_open())
    {
        file << requestBody << "\n"; // Write the request body to the file
        file.close(); // Close the file
        std::cout << "POST Request Body appended to " << filePath << std::endl;

        // Response to the client
        response.setBody("<!DOCTYPE html><html><body><h1>POST data appended successfully to post.txt</h1></body></html>");
    }
    else
    {
        // In case of failure to open the file
        std::cout << "Failed to append POST Request Body to post.txt in C:\\temp!" << std::endl;
        return HttpResponse::createInternalErrorResponse();
    }

    return response;
}

HttpResponse HttpResponse::createPutResponse(const string& fileName, const string& requestBody)
{
    HttpResponse response(200, "OK"); // Always return 200 OK
    response.setContentType("text/html");
    response.setConnection("keep-alive");

    // Construct full file path in C:\temp
    const string filePath = "C:\\temp\\" + fileName;
    // Open the file for writing
    std::ofstream file(filePath);
    if (file.is_open())
    {
        file << requestBody; // Write the content to the file
        file.close();

        // Confirmation message in the response body
        response.setBody("<!DOCTYPE html><html><body><h1>PUT operation completed</h1></body></html>");
    }
    else
    {
        // If the file cannot be created or written
        return HttpResponse::createInternalErrorResponse();
    }

    return response;
}


HttpResponse HttpResponse::createDeleteResponse(const string& fileName)
{
    HttpResponse response(200, "OK");
    response.setContentType("text/html");
    response.setConnection("keep-alive");
    // Construct full file path in C:\temp
    const string filePath = "C:\\temp\\" + fileName;
    // Attempt to delete the file
    if (std::remove(filePath.c_str()) == 0)
    {
        response.setBody("<!DOCTYPE html><html><body><h1>DELETE operation completed</h1></body></html>");
    }
    else
    {
        // If the file cannot be deleted, return a 404 response
        return HttpResponse::createNotFoundResponse();
    }

    return response;
}

HttpResponse HttpResponse::createTraceResponse(const string& originalRequest)
{
    HttpResponse response(200, "OK");
    response.setContentType("message/http");
    response.setConnection("keep-alive");
    // Echo back the original request
    response.setBody(originalRequest);
    // Set the correct Content-Length based on the body
    response.setContentLength(originalRequest.size());
    std::cout << response.toString();
    return response;
}


// Convert the response to a string format
string HttpResponse::toString() const
{
    std::ostringstream responseStream;

    // Status line
    responseStream << httpVersion << " " << statusCode << " " << statusMessage << "\r\n";

    // Headers
    if (!headerContentType.empty())
        responseStream << "Content-Type: " << headerContentType << "\r\n";

    if (headerContentLength > 0)
        responseStream << "Content-Length: " << headerContentLength << "\r\n";

    if (!allow.empty())
        responseStream << "Allow: " << allow << "\r\n";
   
    if (!headerConnection.empty())
        responseStream << "Connection: " << headerConnection << "\r\n";

    // Blank line to separate headers from body
    responseStream << "\r\n";

    // Body
    responseStream << body;

    return responseStream.str();
}

