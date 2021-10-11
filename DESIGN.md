# Design

This browser should be usable both from C++ and, as opcodes, from Csound.

It thus makes sense to develop first for C++, in order to define the interface and implement the functionality, and then to expose the interface through very thin opcodes.

The base class is WebKitWebView. The interface should be something like this:
```
struct CsoundWebView {
  CsoundThreaded csound;
  CsoundThreaded() : csound(nullptr) {};
  virtual ~CsoundWebView() {};
  static CsoundWebView create(CSOUND *csound_);
  virtual int close() = 0;
  virtual int open() = 0;
  virtual int load_uri(std::string uri) = 0;
  virtual int load_html(std::string html) = 0;
  virtual int execute_javascript(std::string javascript) = 0;
  virtual std::variant execute_javascript_synchronously(std::string javascript) = 0;
  virtual int open_inspector() = 0;
 };
 ```
  
