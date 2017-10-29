### Alternative to read a line from a socket using buffering
[https://stackoverflow.com/a/1584620](https://stackoverflow.com/a/1584620)

``` cpp
bool ReadLine (int fd, string* line) {
  // We read-ahead, so we store in static buffer
  // what we already read, but not yet returned by ReadLine.
  static string buffer;

  // Do the real reading from fd until buffer has '\n'.
  string::iterator pos;
  while ((pos = find (buffer.begin(), buffer.end(), '\n')) == buffer.end ()) {
    char buf [1025];
    int n = read (fd, buf, 1024);
    if (n == -1) {    // handle errors
      *line = buffer;
      buffer = "";
      return false;
    }
    buf [n] = 0;
    buffer += buf;
  }

  // Split the buffer around '\n' found and return first part.
  *line = string (buffer.begin(), pos);
  buffer = string (pos + 1, buffer.end());
  return true;
}
```
