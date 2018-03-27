## Summary
Multi-threaded HTTP proxy server able to efficiently service multiple simultaneous browser connections.

On each new browser connection:
- Receives the headers from the client browser
- Sanitizes/filters the request URI
- Resolves host and establishes socket connections
- Streams response data back to browser

### Usage
```
proxy <listen-port-no>
```
Set browser proxy settings to use the machine host name and proxy port.

### Building
```bash
# Release version
make
# Debug version with logging and tracing enabled
make debug
```

### Project Files

|File|Description|
|:---|:----------|
|AddrInfo.cpp         | Container to manage DNS entries
|ClientConnection.cpp | Establishes and owns the client connection
|HttpRequest.cpp      | Parses request headers over a stream
|HttpResponse.cpp     | Generates HTTP error responses
|HttpUri.cpp          | Parses and sanitizes URIs
|ListenConnection.cpp | Wrapper over POSIX sockets API
|helpers.cpp          | Miscellaneous helper methods
|proxy.cpp            | Main entry point of the proxy program

### Design Notes

- HTTP headers are parsed is IAW RFC-3986
- URI is parsed and decomposed into section IAW page 6 of RFC-3986:
  - Scheme
  - Authority
  - Remaining relative path
- Authority is further parsed IAW page 18 of RFC-2986:

  `authority = [ userinfo "@" ] host [ ":" port ]`
- Returns HTTP 500 Server Error to the user in case of invalid request or other errors.
- Response headers from requested URI are parsed and content is streamed until connection is terminated.
    
