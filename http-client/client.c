#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "buffer.h"

#define HOST "34.246.184.49"
#define PORT 8080

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Function to extract the cookie
char* extract_cookie(char *response) {
    char *cookie_start = strstr(response, "Set-Cookie: ");
    if (!cookie_start) return NULL;
    cookie_start += strlen("Set-Cookie: ");
    char *cookie_end = strstr(cookie_start, ";");
    if (!cookie_end) return NULL;
    size_t cookie_len = cookie_end - cookie_start;
    char *cookie = (char*)malloc(cookie_len + 1);
    strncpy(cookie, cookie_start, cookie_len);
    cookie[cookie_len] = '\0';
    return cookie;
}

// Function to extract the JWT token
char* extract_token(char *response) {
    char *token_start = strstr(response, "{\"token\":\"");
    if (!token_start) return NULL;
    token_start += strlen("{\"token\":\"");
    char *token_end = strchr(token_start, '\"');
    if (!token_end) return NULL;
    size_t token_len = token_end - token_start;
    char *token = (char*)malloc(token_len + 1);
    strncpy(token, token_start, token_len);
    token[token_len] = '\0';
    return token;
}

// Function to send a POST request and print result
void send_post_request_and_print_response(int sockfd, const char *request_body, const char *endpoint, const char *cookie, const char *token, int is_register) {
    char message[2048];
    if (token) {
        snprintf(message, sizeof(message), 
                 "POST %s HTTP/1.1\r\n"
                 "Host: %s:%d\r\n"
                 "Content-Type: application/json\r\n"
                 "Authorization: Bearer %s\r\n"
                 "Content-Length: %ld\r\n"
                 "\r\n%s",
                 endpoint, HOST, PORT, token, strlen(request_body), request_body);
    } else if (cookie) {
        snprintf(message, sizeof(message), 
                 "POST %s HTTP/1.1\r\n"
                 "Host: %s:%d\r\n"
                 "Content-Type: application/json\r\n"
                 "Cookie: %s\r\n"
                 "Content-Length: %ld\r\n"
                 "\r\n%s",
                 endpoint, HOST, PORT, cookie, strlen(request_body), request_body);
    } else {
        snprintf(message, sizeof(message), 
                 "POST %s HTTP/1.1\r\n"
                 "Host: %s:%d\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %ld\r\n"
                 "\r\n%s",
                 endpoint, HOST, PORT, strlen(request_body), request_body);
    }
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (strstr(response, "200 OK") || strstr(response, "201 Created")) {
        if (is_register) {
            printf("SUCCESS: A new user was created!\n");
        } else if (strcmp(endpoint, "/api/v1/tema/library/books") == 0) {
            printf("SUCCESS: Book added to library!\n");
        }
    } else {
        // Handle possible errors
        char *error_start = strstr(response, "{\"error\":\"");
        if (error_start) {
            error_start += strlen("{\"error\":\"");
            char *error_end = strchr(error_start, '\"');
            if (error_end) {
                *error_end = '\0';
                if (strstr(response, "400 Bad Request")) {
                    if (is_register) printf("ERROR: Username already exists!\n");
                    else printf("ERROR: Missing field!\n");
                } else if (strstr(response, "500 Internal Server Error")) {
                    printf("ERROR: Bad format!\n");
                } else if (strstr(response, "403 Forbidden")) {
                    printf("ERROR: Authorization header is missing!\n");
                } else {
                    printf("ERROR: %s\n", error_start);
                }
            }
        } else {
            printf("ERROR: Unexpected response: %s\n", response);
        }
    }
    free(response);
}

// Function to send a GET request and print result
void send_get_request_and_print_response(int sockfd, const char *endpoint, const char *cookie, const char *token) {
    char message[2048];
    if (token && cookie) {
        snprintf(message, sizeof(message),
                 "GET %s HTTP/1.1\r\n"
                 "Host: %s:%d\r\n"
                 "Authorization: Bearer %s\r\n"
                 "Cookie: %s\r\n"
                 "\r\n",
                 endpoint, HOST, PORT, token, cookie);
    } else if (token) {
        snprintf(message, sizeof(message),
                 "GET %s HTTP/1.1\r\n"
                 "Host: %s:%d\r\n"
                 "Authorization: Bearer %s\r\n"
                 "\r\n",
                 endpoint, HOST, PORT, token);
    } else if (cookie) {
        snprintf(message, sizeof(message),
                 "GET %s HTTP/1.1\r\n"
                 "Host: %s:%d\r\n"
                 "Cookie: %s\r\n"
                 "\r\n",
                 endpoint, HOST, PORT, cookie);
    } else {
        snprintf(message, sizeof(message),
                 "GET %s HTTP/1.1\r\n"
                 "Host: %s:%d\r\n"
                 "\r\n",
                 endpoint, HOST, PORT);
    }
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    
    if (strstr(response, "200 OK")) {
        // Extract and print the list of books
        char *json_start = strchr(response, '[');
        if (json_start) {
            char *json_end = strchr(json_start, ']');
            if (json_end) {
                *json_end = '\0';
                printf("SUCCESS: The list of books: \n%s]\n", json_start);
            } else {
                printf("ERROR: Invalid JSON response\n");
            }
        } else {
            printf("ERROR: No JSON response found\n");
        }
    } else {
        // Handle possible errors
        char *error_start = strstr(response, "{\"error\":\"");
        if (error_start) {
            error_start += strlen("{\"error\":\"");
            char *error_end = strchr(error_start, '\"');
            if (error_end) {
                *error_end = '\0';
                printf("ERROR: %s\n", error_start);
            }
        } else {
            printf("ERROR: Unexpected response: %s\n", response);
        }
    }
    free(response);
}

// Function to send a GET request for a specific book and print the result
void send_get_book_request_and_print_response(int sockfd, int book_id, const char *cookie, const char *token) {
    char message[2048];
    snprintf(message, sizeof(message),
             "GET /api/v1/tema/library/books/%d HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Authorization: Bearer %s\r\n"
             "Cookie: %s\r\n"
             "\r\n",
             book_id, HOST, PORT, token ? token : "", cookie ? cookie : "");
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    // Check for specific error messages
    if (strstr(response, "200 OK")) {
        char *json_start = strchr(response, '{');
        if (json_start) {
            printf("SUCCESS: The book data%s\n", json_start);
        } else {
            printf("ERROR: No book details found in response\n");
        }
    } else {
        // Hnadle possible errors
        char *error_start = strstr(response, "{\"error\":\"");
        if (error_start) {
            error_start += strlen("{\"error\":\"");
            char *error_end = strchr(error_start, '\"');
            if (error_end) {
                *error_end = '\0';
                if (strcmp(error_start, "No book was found!") == 0) {
                    printf("ERROR: No book was found!\n");
                } else if (strcmp(error_start, "Error when decoding token!") == 0) {
                    printf("ERROR: Error when decoding token!\n");
                } else {
                    printf("ERROR: %s\n", error_start);
                }
            }
        } else {
            printf("ERROR: Unexpected response: %s\n", response);
        }
    }
    free(response);
}

// Function to send a DELETE request and print the result
void send_delete_book_request_and_print_response(int sockfd, int book_id, const char *cookie, const char *token) {
    char message[2048];
    snprintf(message, sizeof(message),
             "DELETE /api/v1/tema/library/books/%d HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Authorization: Bearer %s\r\n"
             "Cookie: %s\r\n"
             "\r\n",
             book_id, HOST, PORT, token ? token : "", cookie ? cookie : "");
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (strstr(response, "200 OK")) {
        printf("SUCCESS: Book with id %d was deleted!\n", book_id);
    } else {
        // Handle possible errors
        char *error_start = strstr(response, "{\"error\":\"");
        if (error_start) {
            error_start += strlen("{\"error\":\"");
            char *error_end = strchr(error_start, '\"');
            if (error_end) {
                *error_end = '\0';
                printf("ERROR: %s\n", error_start);
            }
        } else {
            printf("ERROR: Unexpected response: %s\n", response);
        }
    }
    free(response);
}

// Function to send a logout GET request and print the result
void send_logout_request_and_print_response(int sockfd, const char *cookie, const char *token) {
    char message[2048];
    snprintf(message, sizeof(message),
             "GET /api/v1/tema/auth/logout HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Authorization: Bearer %s\r\n"
             "Cookie: %s\r\n"
             "\r\n",
             HOST, PORT, token ? token : "", cookie ? cookie : "");
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (strstr(response, "200 OK")) {
        printf("SUCCESS: Successfully logged out!\n");
    } else {
        // Handle errors
        char *error_start = strstr(response, "{\"error\":\"");
        if (error_start) {
            error_start += strlen("{\"error\":\"");
            char *error_end = strchr(error_start, '\"');
            if (error_end) {
                *error_end = '\0';
                if (strcmp(error_start, "You are not logged in!") == 0) {
                    printf("ERROR: You are not logged in!\n");
                } else {
                    printf("ERROR: %s\n", error_start);
                }
            }
        } else {
            printf("Eroare la delogare: %s\n", response);
        }
    }
    free(response);
}

int main(int argc, char *argv[]) {
    int sockfd;
    char command[50];
    char *cookie = NULL;
    char *token = NULL;

    // Read and handle all commands
    while (1) {
        printf("Introduceți o comandă: ");
        scanf("%s", command);

        if (strcmp(command, "register") == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char username[50], password[50];

            printf("username=");
            scanf("%s", username);

            printf("password=");
            scanf("%s", password);

            char request_body[200];
            snprintf(request_body, sizeof(request_body), "{\"username\":\"%s\",\"password\":\"%s\"}", username, password);
            send_post_request_and_print_response(sockfd, request_body, "/api/v1/tema/auth/register", NULL, NULL, 1);

            close_connection(sockfd);

        } else if (strcmp(command, "login") == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char username[50], password[50];

            printf("username=");
            scanf("%s", username);

            printf("password=");
            scanf("%s", password);

            char request_body[200];
            snprintf(request_body, sizeof(request_body), "{\"username\":\"%s\",\"password\":\"%s\"}", username, password);

            char message[2048];
            snprintf(message, sizeof(message), 
                    "POST /api/v1/tema/auth/login HTTP/1.1\r\n"
                    "Host: %s:%d\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: %ld\r\n"
                    "\r\n%s", 
                    HOST, PORT, strlen(request_body), request_body);
            send_to_server(sockfd, message);
            char *response = receive_from_server(sockfd);

            if (strstr(response, "200 OK")) {
                printf("SUCCESS: You are logged in!\n");
            } else {
                // Handle possible errors
                char *error_start = strstr(response, "{\"error\":\"");
                if (error_start) {
                    error_start += strlen("{\"error\":\"");
                    char *error_end = strchr(error_start, '\"');
                    if (error_end) {
                        *error_end = '\0';
                        printf("ERROR: %s\n", error_start);
                    }
                } else {
                    printf("ERROR: Unexpected response: %s\n", response);
                }
            }

            // Extract and store the session cookie
            if (cookie) {
                free(cookie);
            }
            cookie = extract_cookie(response);

            free(response);
            close_connection(sockfd);

        } else if (strcmp(command, "enter_library") == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char message[2048];
            snprintf(message, sizeof(message),
                    "GET /api/v1/tema/library/access HTTP/1.1\r\n"
                    "Host: %s:%d\r\n"
                    "Cookie: %s\r\n"
                    "\r\n",
                    HOST, PORT, cookie ? cookie : "");
            send_to_server(sockfd, message);
            char *response = receive_from_server(sockfd);

            if (strstr(response, "200 OK")) {
                // Extract token
                if (token) {
                    free(token);
                }
                token = extract_token(response);
                if (token) {
                    printf("SUCCESS: Extracted token: %s\n", token);
                }
            } else {
                printf("ERROR: Couldn't access the library!\n");
            }

            free(response);
            close_connection(sockfd);

        } else if (strcmp(command, "get_books") == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            send_get_request_and_print_response(sockfd, "/api/v1/tema/library/books", cookie, token);
            close_connection(sockfd);

        } else if (strcmp(command, "get_book") == 0) {
            int id;
            printf("id=");
            scanf("%d", &id);

            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            send_get_book_request_and_print_response(sockfd, id, cookie, token);
            close_connection(sockfd);
            
        } else if (strcmp(command, "add_book") == 0) {
            int BUFFER_SIZE = 150;
            char title[BUFFER_SIZE], author[BUFFER_SIZE], genre[BUFFER_SIZE], publisher[BUFFER_SIZE];
            char page_count_str[BUFFER_SIZE];
            int page_count;

            clear_input_buffer();

            printf("title=");
            if (fgets(title, BUFFER_SIZE, stdin) == NULL) {
                printf("Error reading title.\n");
                continue;
            }
            title[strcspn(title, "\n")] = 0;  // Remove newline character

            printf("author=");
            if (fgets(author, BUFFER_SIZE, stdin) == NULL) {
                printf("Error reading author.\n");
                continue;
            }
            author[strcspn(author, "\n")] = 0;

            printf("genre=");
            if (fgets(genre, BUFFER_SIZE, stdin) == NULL) {
                printf("Error reading genre.\n");
                continue;
            }
            genre[strcspn(genre, "\n")] = 0;

            printf("publisher=");
            if (fgets(publisher, BUFFER_SIZE, stdin) == NULL) {
                printf("Error reading publisher.\n");
                continue;
            }
            publisher[strcspn(publisher, "\n")] = 0;

            printf("page_count=");
            if (fgets(page_count_str, BUFFER_SIZE, stdin) == NULL) {
                printf("Error reading page_count.\n");
                continue;
            }
            page_count_str[strcspn(page_count_str, "\n")] = 0;

            // Validate page_count input
            if (sscanf(page_count_str, "%d", &page_count) != 1) {
                printf("Tip de date incorect pentru numarul de pagini\n");
                clear_input_buffer();
                continue;
            }

            char request_body[512];
            snprintf(request_body, sizeof(request_body), 
                    "{\"title\":\"%s\",\"author\":\"%s\",\"genre\":\"%s\",\"publisher\":\"%s\",\"page_count\":%d}",
                    title, author, genre, publisher, page_count);

            // Print the JSON body
            printf("Formatted JSON: %s\n", request_body);

            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            send_post_request_and_print_response(sockfd, request_body, "/api/v1/tema/library/books", cookie, token, 0);
            close_connection(sockfd);

        } else if (strcmp(command, "delete_book") == 0) {
            int id;
            printf("id=");
            scanf("%d", &id);

            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            send_delete_book_request_and_print_response(sockfd, id, cookie, token);
            close_connection(sockfd);

        } else if (strcmp(command, "delete_book") == 0) {
            int id;
            printf("id=");
            scanf("%d", &id);

            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            send_delete_book_request_and_print_response(sockfd, id, cookie, token);
            close_connection(sockfd);

        } else if (strcmp(command, "logout") == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            send_logout_request_and_print_response(sockfd, cookie, token);
            close_connection(sockfd);

            if (cookie) {
                free(cookie);
                cookie = NULL;
            }
            if (token) {
                free(token);
                token = NULL;
            }

        } else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("Comandă necunoscută!\n");
        }
    }

    if (cookie) {
        free(cookie);
    }
    
    if (token) {
        free(token);
    }

    return 0;
}
