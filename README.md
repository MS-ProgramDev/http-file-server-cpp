# Basic HTTP Server in C++

A simple file-based HTTP server implemented in C++.  
Supports full CRUD operations via standard HTTP methods.

## Features

- Handles HTTP methods: `GET`, `POST`, `PUT`, `DELETE`, `HEAD`, `OPTIONS`, `TRACE`
- File read/write/delete support from `C:\temp` folder
- Simple routing with `query string` support (e.g., `?lang=en`)
- Custom HTML responses
- Console-based logging for POST and PUT
- Fully testable with Wireshark

## Example

Send a PUT request to create or update a file:

```
PUT localhost:80/newfile.txt
Body: "This is the content for the new file."
```

## Folder Structure

- `/src` – C++ source files
- `/html` – Static HTML files to be served
- `/docs` – API documentation & testing explanation (Wireshark captures included)

## Author

Maor Sapo – 2025

---

✅ Want to test? Place HTML files inside `C:\temp`, then run the server and test using Postman or browser.
