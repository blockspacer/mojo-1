`http_parser` is a platform-independent package for parsing and serializing
various HTTP-related formats. It's designed to be usable on both the browser and
the server, and thus avoids referencing any types from `dart:io` or `dart:html`.
It includes:

* Support for parsing and formatting dates according to [HTTP/1.1][2616], the
  HTTP/1.1 standard.

* A `MediaType` class that represents an HTTP media type, as used in `Accept`
  and `Content-Type` headers. This class supports both parsing and formatting
  media types according to [HTTP/1.1][2616].

* A `CompatibleWebSocket` class that supports both the client and server sides
  of the [WebSocket protocol][6455] independently of any specific server
  implementation.

[2616]: http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
[6455]: https://tools.ietf.org/html/rfc6455
