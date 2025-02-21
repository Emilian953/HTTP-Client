# 📚 Library Management Client

This is a **C** client application that communicates with a REST API server to manage a collection of books. The application allows users to **register, authenticate, access the library, view books, add new books, and delete books**.

# 🛠 Technologies and Libraries Used
- **C** for client development 
-  **TCP Sockets** (`sys/socket.h`, `netinet/in.h`, `arpa/inet.h`) for network communication 
-  **HTTP (GET, POST, DELETE)** request handling 
-  **Dynamic memory management** (`malloc`, `realloc`, `free`) 
- **String manipulation** (`strstr`, `strchr`, `snprintf`, `fgets`) 
-  **Custom buffer handling** (`buffer.c`, `buffer.h`) for storing and processing network responses



# 📂 Project Structure

| Fisier | Descriere |
 |---------------|-----------| 
 | `client.c` | Main implementation of the client application | 
 | `helpers.c` | Utility functions for string manipulation and network connections | 
 | `buffer.c` | Functions for handling dynamic memory buffers used in communication |
 

## ⚡ Features

### 🔹 **1. User Registration (`register`)**
- Opens a connection to the server (`open_connection`).
- Prompts the user to enter a **username** and **password**.
- Builds a JSON payload and sends a **POST** request to: `/api/v1/tema/auth/register` 
- Receives and displays the server response.
- Closes the connection (`close_connection`).

### 🔹 **2. User Authentication (`login`)**
- Opens a connection to the server.
- Prompts the user for a **username** and **password**.
- Sends a **POST** request to:`/api/v1/tema/auth/login` 
- Extracts and stores the **session cookie** using `extract_cookie()`.
- Displays success or error messages.
- Closes the connection.

### 🔹 **3. Library Access (`enter_library`)**
- Opens a connection and sends a **GET** request to:`/api/v1/tema/library/access` 
- Extracts and stores the **JWT token** required for book access (`extract_token()`).
- Closes the connection.

### 🔹 **4. Retrieve Book List (`get_books`)**
- Opens a connection and sends a **GET** request to:`/api/v1/tema/library/books` 
- Uses the session **cookie** and **JWT token** for authentication.
- Receives and displays the list of books in the library.
- Closes the connection.

### 🔹 **5. Retrieve Book Details (`get_book`)**
- Prompts the user for a **book ID**.
- Opens a connection and sends a **GET** request to:`/api/v1/tema/library/books/:bookId` 
- Receives and displays the book details.
- Closes the connection.

### 🔹 **6. Add a New Book (`add_book`)**
- Prompts the user for **title, author, genre, publisher, and page count**.
- Builds a JSON payload and sends a **POST** request to:`/api/v1/tema/library/books` 
- Uses the session **cookie** and **JWT token** for authentication.
- Displays the server response.
- Closes the connection.

### 🔹 **7. Delete a Book (`delete_book`)**
- Prompts the user for a **book ID**.
- Opens a connection and sends a **DELETE** request to:`/api/v1/tema/library/books/:bookId` 
- Uses the session **cookie** and **JWT token** for authentication.
- Displays the server response.
- Closes the connection.

### 🔹 **8. Logout (`logout`)**
- Opens a connection and sends a **GET** request to:`/api/v1/tema/auth/logout` 
- Invalidates the **session cookie** and **JWT token**.
- Closes the connection.

### 🔹 **9. Exit (`exit`)**
- Frees allocated memory for the session cookie and token.
- Terminates the program execution.


## 🔑 Session Cookie and Token Management 
- The **session cookie** is stored after login and used in subsequent requests. 
-  The **JWT token** is obtained after `enter_library` and is required for book-related operations. 
-  On **logout**, both the session cookie and token are freed.
