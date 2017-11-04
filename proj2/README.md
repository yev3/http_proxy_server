################################################################################
Project 2 Phase 1
CPSC 5510 Networking
Yevgeni Kamenski, Zachary Madigan, David Pierce
################################################################################

## NAME
    proxy - recieves an HTTP request, establishes a TCP connection with the
    requested web server, and recieves and displays the HTTP response.

## SYNOPSIS
    proxy port#

## DESCRIPTION
    Proxy listens on the user-specified port, and when it recieves an HTTP
    request from a client, like:
        GET http://www.cnn.com/ HTTP/1.0

    It parses the client's HTTP request and makes a TCP connection to the
    requested server, and displays the response from the web server. It then exits

## DETAILED DESCRIPTION
    When proxy receives http headers, it expects the first line to be the
    request line in format listed in the RFC-3986, and only supports
    the "GET" request type.

    It parses the given URL into sections as described on page 6 of RFC-3986
      - scheme
      - authority
      - remaining relative path

    Checks that the scheme specified is "http", if that is not used, it
    responds to the user with HTTP 500 Server Error 
    because the proxy does not support anything other than http://

    Authority is further parsed per RFC-2986 page 18:
    authority = [ userinfo "@" ] host [ ":" port ]

    Any encountered user info sections are discarded as they are part of the
    request headers, but port is extracted if present, otherwise it is set to
    a default value of 80.
	
## STRENGTHS
	- Code is extensible and modular

## WEAKNESSES
	- No support for multiple clients
	- No object caching
	- Files/objects not sent back to browser

## FILES
     include/                    Directory with the header files
     src/AddrInfo.cpp            Class to traverse the DNS entries linked list
     src/ClientConnection.cpp    Class to establish and close the client conn
     src/helpers.cpp             Miscellaneous helper methods
     src/HttpRequest.cpp         Class to load and parse the request headers
     src/HttpResponse.cpp        Class to generate HTTP 500 response
     src/HttpUri.cpp             Class to parse and extract the URL items
     src/ListenConnection.cpp    Class to manage a listening connection
     src/proxy.cpp               Main entry point of the proxy program
     Makefile                    GNU Make build file
     README                      This readme file

## COMPILING
    The following commands are run in the source directory:
    make                Compiles release version of the proxy
    make debug          Compiles the debug version of the proxy, which may print
                        additional diagnostic information to the console.
    make clean          Removes the executable and object files to start fresh
