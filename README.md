# csound-webserver
![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/csound-webserver/total.svg)<br>

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

The csound_webserver opcodes embed an internal Web server into the Csound 
performance, provide an XMLHttpRequest interface to the running instance of 
Csound, and optionally serve an HTML page from the embedded Web server. That 
page can be embedded into the Csound orchestra code, or it can be a regular 
HTML file that refers to other resources. The opcodes by default will open a  
standard external Web browser to run the served HTML page.

The purpose of these opcodes is to define user interfaces, generate scores, or 
control performances using JavaScript, and otherwise use all of the many, many 
capabilities of standard Webwebservers in the context of the Csound 
performance.

These opcodes have been developed to overcome shortcomings that became apparent 
in csound-extended-node (need for a package configuration file to run pieces), 
webserver-opcodes (lack of consistency between Apple's WebKit and GTK's WebKit),
and CsoundQt-html5.

These are the opcodes: 
```
i_webserver_handle webserver_create S_base_uri [, i_rpc_port [, i_diagnostics_enabled]]
webserver_open_url i_webserver_handle, S_url [, S_browser_command]
webserver_open_html i_webserver_handle, S_html [, S_browser_command]
```
The following JavaScript interface can be used from the JavaScript context of 
a Web page opened by these opcodes. As far as possible, the methods of this 
interface are the same as that in `csound.hpp`:
```
CompileCsdText
CompileOrc
EvalCode
Get0dBFS
GetAudioChannel
GetControlChannel
GetDebug
GetKsmps
GetNchnls
GetNchnlsInput
GetScoreOffsetSeconds
GetScoreTime
GetSr
GetStringChannel
InputMessage
IsScorePending
Message (this is the only asynchronous method)
ReadScore
RewindScore
ScoreEvent
SetControlChannel
SetDebug
SetMessageCallback
SetScoreOffsetSeconds
SetScorePending
SetStringChannel
TableGet
TableLength
TableSet
```

For the `webserver_open_html` opcode, the skeletons for this interface are 
injected into the HTML `<head>` element.

For the `webserver_open_url` opcode, the Web page must include the `csound.js` 
script in the HTML `<head>` element.

Naturally, all Csound API methods that destroy or create Csound, start 
or stop the performance, or configure Csound's audio or MIDI input or output 
drivers have had to be omitted from this interface.

Please note, these methods are asynchronous. You may need to use the JSON-RPC 
callbacks to obtain results, perhaps using promises to keep the results in the 
correct order.

# webserver_create

`webserver_create` - Creates and runs an instance of the internal Web server 
embedded into the Csound performance.

## Description

Creates an instance an instance of the internal Web server embedded 
into the Csound performance. The embedded Web server can 
serve any resources located in its base URI.


Csound itself, or C++ code compiled using the Clang opcodes for Csound, 
can also execute JavaScript code in the JavaScript context of an opened 
Web page using the `webserver_run_javascript` opcode (Csound) or function 
(C++ code running in Csound).

Thus, the interface between Csound and the Web pages that Csound creates 
is fully bidirectional.

## Syntax
```
i_browser_handle webserver_create S_uri [, i_rpc_port [, i_diagnostics_enabled]]

```
## Initialization

*S_uri* - The base URI of the embedded Web server.

*i_rpc_port* - The number of a port on `localhost` that the Csound proxy will
use for JSON-RPC calls. If omitted, the port defaults to 8383.

*i_diagnostics_enabled* - If 0 (the default), diagnostic messages are not printed; 
if non-0, diagnostic messages are printed.

*i_browser_handle* - Returns a handle to the newly createdwebserver. 
The other WebKit opcodes must take such a handle as their first pfield. One 
browser can open any number of Web pages, each in its own top-level window.

## Performance

Once created, and whether or not it actually displays any Web pages, the 
browser remains in scope until the end of the Csound performance.

# webserver_open_uri

`webserver_open_uri` - Opens a new top-level window and displays in it the 
content defined in the universal resource identifier. This can be a local file 
or an Internet resource.

Please note, pages opened with this opcode will not have access to Csound 
unless the body of those pages includes the `csound.js` script for the Csound 
proxy. That will not normally be the case for Web pages from the Internet. 
Thus, `webserver_open_uri` is primarily useful for opening Internet resources 
such as documentation.

## Syntax

webserver_open_uri i_webserver_handle, S_window_title, S_uri, i_width, i_height [, i_fullscreen]

## Initialization

*i_webserver_handle* - The handle of awebserver created by `webserver_create`.

*S_window_title* - The title to be displayed by the top-levelwebserver window.

*S_uri* - The Uniform Resource Identifier of an Internet resource to be loaded by 
thewebserver.

*i_width* - The width of the top-levelwebserver window in pixels.

*i_height* - The height of the top-levelwebserver window in pixels.

*i_fullscreen* - If 0 (the default value), the size of thewebserver window is 
`i_width` x `i_height`; if non-0, thewebserver window is fullscreen.

## Performance

Thewebserver window remains open for the remainder of the Csound performance. 
Window events and JavaScript callbacks within thewebserver are dispatched every 
kperiod.

Right-clicking on thewebserver opens a context menu with a command to open the 
browser's inspector, or debugger. It can be used to view HTML and JavaScript 
code, inspect elements of the Document Object Model, and to set breakpoints or 
inspect variables in JavaScript code.

Once the Web page has opened, Csound can run JavaScript in the JavaScript 
context of that page using the `webserver_run_javascript` opcode.

# webserver_open_html

`webserver_open_html` - Opens a new top-level window and displays in it the content 
defined by the S_html parameter, typically a multi-line string constant contained 
within the `{{` and `}}` delimiters.

## Syntax

webserver_open_html i_webserver_handle, S_window_title, S_html, S_base_uri, i_width, i_height [, i_fullscreen]

## Initialization

*i_webserver_handle* - The handle of awebserver created by `webserver_create`.

*S_window_title* - The title to be displayed by the top-levelwebserver window.

*S_html* - A string containing valid HTML5 code, typically a multi-line string 
constant contained within the `{{` and `}}` delimiters.

*S_base_uri* - A Uniform Resource Identifier specify the base from which relative 
URI addresses are found. This will normally be the filesystem directory 
that contains the Csound piece. Additional Web pages, JavaScript files, images, 
and so on can be loaded from the base URI.

The `file` URL scheme does not permit relative filepaths. However, it is easy, in 
Csound, to construct an absolute filepath given that Csound knows what its current 
working directory is. You can use the `pwd` opcode to get this:
```
S_current_working_directory pwd
S_base_uri sprintf "file://%s/", S_current_working_directory
prints S_base_uri
webserver_open_html gi_browser, "Message", gS_html_code, S_base_uri, 900, 650
```

*i_width* - The width of the top-levelwebserver window in pixels.

*i_height* - The height of the top-levelwebserver window in pixels.

*i_fullscreen* - If 0 (the default value), the size of thewebserver window is 
`i_width` x `i_height`; if non-0, thewebserver window is fullscreen.

## Performance

Thewebserver window remains open for the remainder of the Csound performance. 
Window events and JavaScript callbacks within thewebserver are dispatched every 
kperiod.

In order for user-defined code to call back into Csound, include the 
`csound.js` script that defines the Csound proxy as a script element in the 
body of the Web page. The script element can be loaded from the filesystem, or 
it can be included directly in the Web page's code.

Not only can the Web page call methods of the Csound API, but also Csound can 
run JavaScript in that Web page using the `webserver_run_javascript` opcode.

Right-clicking on thewebserver opens a context menu with a command to open the 
browser's inspector, or debugger. It can be used to view HTML and JavaScript 
code, inspect elements of the Document Object Model, and to set breakpoints or 
inspect variables in JavaScript code.

## Example

See `webserver_example.js`.

# webserver_run_javascript

`webserver_run_javascript` - Executes JavaScript source code asynchronously in 
the JavaScript context of thewebserver's default Web page. 

Note that there is a C++ version of the opcode that can be called during the 
performance by C++ code compiled by the Clang just-in-time compiler:
```
extern "C" void webserver_run_javascript(intwebserver_handle, std::string javascript_code);
```

## Description

`webserver_run_javascript` - Executes JavaScript source code asynchronously in 
the JavaScript context of thewebserver's default Web page. Note that there is a 
C++ version of the opcode that can be called by C++ code that has been 
compiled by the Clang opcodes during the performance. This can be used to 
to send a generated score in JSON format for display on the page, or to call 
existing functions in the JavaScript context. However, this opcode is 
asynchronous and does not directly return the last value produced by the 
evaluation of the JavaScript code.

It is however possible to have the JavaScript code return a value 
asynchronously, via the "on success" callback of the Csound API call.

Please note, it is quite possible to call `webserver_run_javascript` in the 
orchestra header, before thewebserver has actually been initialized. In such 
cases, the JavaScript code is enqueued. The code is dequeued and executed when 
(a) thewebserver has been initialized, and (b) its Web page has finished loading. 
So, it is always safe to call this opcode.

## Syntax
```
i_result webserver_run_javascript i_browser_handle, S_javascript_code
```
## Initialization

*i_browser_handle* - The handle of awebserver created by `webserver_create`.

*S_javascript_code* - JavaScript source code that will be executed immediately 
in the JavaScript context of a Web page. Such code can be a single function 
call, or a multi-line string constant contained within the `{{` and `}}` 
delimiters that creates an entire JavaScript module.

Please note, this opcode is designed to work with only one Web page opened 
from onewebserver instance. The results of trying to call from Csound into more 
than one Web page are undefined. If you need Csound to call into more than one 
Web page, you should create a separatewebserver on a separate port for each 
page.

Also note, calling this opcode too frequently during a dense performance can 
cause the Web page's user interface to become unresponsive.

## Performance

The JavaScript code executes immediately when thewebserver has been created and 
its Web page has finished loading, but the code can create modules with 
function definitions, class definitions, and so on that will be available for 
other code on the page.

What happens during performance is whatever the JavaScript code does. Such 
code may execute immediately, or it may create a class or library to be 
invoked later on in the performance.
   
# Installation

1. Install [Csound](https://github.com/csound/csound). On Linux, this 
   generally means building from source code.
3. Install the [WebKitGTK](https://webservergtk.org/) package and its 
   dependencies, preferably as a system package, e.g. 
   `sudo apt-get install libwebserver2gtk-4.0-dev`.
4. Install [libjson-rpc-cpp](https://github.com/cinemast/libjson-rpc-cpp), 
   preferably as a system package, e.g. 
   `sudo apt-get install libjsonrpccpp-dev libjsonrpccpp-tools`.
4. Generate the stubs and skeletons for the RPC channel that Web pages 
   displayed by the opcodes use to call Csound:
   ```
   jsonrpcstub --verbose csoundrpc.json --js-client=Csound --cpp-server=CsoundSkeleton
   ```   
4. Build the `webserver_opcodes` plugin opcode library by executing `build.sh`. 
   You may need to modify this build script for your system.
5. Test by executing `csound webserver_example.csd`. 

# Credits

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com
