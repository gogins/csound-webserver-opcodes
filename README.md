# csound-webserver
![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/webkit2-opcodes/total.svg)<br>

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

The csound_webserver opcodes embed a Web server into the csound performance, 
serve an XMLHttpRequest interface to the running instance of Csound, and 
optionally serve an HTML page from the embedded Web server. That page can be 
embedded into the Csound orchestra code, or it can be a regular HTML file.

The opcodes also will optionally open an external Web brower to run the 
served HTML page.

The main purpose of these opcodes is to define user interfaces, generate 
scores or control performances using JavaScript, and otherwise use all of the 
many, many capabilities of standard Web browsers in the context of a Csound 
performance.

These opcodes have been developed to overcome various shortcomings that became 
apparent in csound-extended-node (need for external configuration to run 
pieces) and webkit-opcodes (lack of consistency between Apple's WebKit and 
GTK's WebKit).

These are the opcodes: 
```
i_webserver_handle webserver_create [i_rpc_port [, i_diagnostics_enabled]]
webserver_open_uri i_webserver_handle, S_window_title, S_uri, i_width, i_height [, i_fullscreen]
webserver_open_html i_webserver_handle, S_window_title, S_html, S_base_uri, i_width, i_height [i, fullscreen]
```
In addition, the following JavaScript interface can be used from the 
JavaScript context of a Web page opened by these opcodes. To do this, include 
the `csound.js` script in the body of your HTML code. As far as possible, the 
methods of this interface are the same as that in `csound.hpp`:
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

constructor: function(url)
```

Naturally, all Csound API methods that destroy or create Csound, start 
or stop the performance, or configure Csound's audio or MIDI input or output 
drivers have had to be omitted from this interface.

Please note, these methods are asynchronous. You may need to use the JSON-RPC 
callbacks to obtain results, perhaps using promises to keep the results in the 
correct order.

# webkit_create

`webkit_create` - Creates an instance of the WebKitGTK Web browser embedded 
into the Csound performance.

## Description

Creates an instance an instance of the WebKitGTK Web browser embedded 
into the Csound performance. This is not _quite_ a full-featured browser, but 
only because it lacks user controls, user settings, history, a URL entry 
bar, and so on. Such features can, however, be created by means of user-
defined HTML and JavaScript code.

In every other way, the embedded browser is indeed full-featured. It can 
display local or remote Web pages, execute JavaScript, open WebSockets, show 
animated WebGL models, and do many other things.

In particular, the embedded browser can call back into the ongoing Csound 
performance using a subset of the Csound API defined in the `Csound.js` 
script. This script can call many methods of the Csound API as implemented 
in `csound.hpp`, but all functions involving the creation, destruction, 
starting and stopping, or runtime configuration had to be omitted. The 
Csound interface in `Csound.js` communicates with the WebKit opcodes and 
thus with Csound using JSON-RPC and Ajax.

Csound itself, or C++ code compiled using the Clang opcodes for Csound, 
can also execute JavaScript code in the JavaScript context of an opened 
Web page using the `webkit_run_javascript` opcode (Csound) or function 
(C++ code running in Csound).

Thus, the interface between Csound and the Web pages that Csound creates 
is fully bidirectional.

Please note, calling `webkit_run_javascript` too frequently during 
a dense performance may cause the Web page's user interface to become 
unresponsive. For this reason, I recommend that the Web page not be used 
to display Csound's diagnostic messages, which can be displayed in the usual 
way in the operating system terminal.

## Syntax
```
i_browser_handle webkit_create [i_rpc_port [, i_diagnostics_enabled]]
```
## Initialization

*i_rpc_port* - The number of a port on `localhost` that the Csound proxy will
use for JSON-RPC calls. If omitted, the port defaults to 8383.

*i_diagnostics_enabled* - If 0 (the default), diagnostic messages are not printed; 
if non-0, diagnostic messages are printed.

*i_browser_handle* - Returns a handle to the newly created browser. 
The other WebKit opcodes must take such a handle as their first pfield. One 
browser can open any number of Web pages, each in its own top-level window.

## Performance

Once created, and whether or not it actually displays any Web pages, the 
browser remains in scope until the end of the Csound performance.

# webkit_open_uri

`webkit_open_uri` - Opens a new top-level window and displays in it the 
content defined in the universal resource identifier. This can be a local file 
or an Internet resource.

Please note, pages opened with this opcode will not have access to Csound 
unless the body of those pages includes the `csound.js` script for the Csound 
proxy. That will not normally be the case for Web pages from the Internet. 
Thus, `webkit_open_uri` is primarily useful for opening Internet resources 
such as documentation.

## Syntax

webkit_open_uri i_webkit_handle, S_window_title, S_uri, i_width, i_height [, i_fullscreen]

## Initialization

*i_webkit_handle* - The handle of a browser created by `webkit_create`.

*S_window_title* - The title to be displayed by the top-level browser window.

*S_uri* - The Uniform Resource Identifier of an Internet resource to be loaded by 
the browser.

*i_width* - The width of the top-level browser window in pixels.

*i_height* - The height of the top-level browser window in pixels.

*i_fullscreen* - If 0 (the default value), the size of the browser window is 
`i_width` x `i_height`; if non-0, the browser window is fullscreen.

## Performance

The browser window remains open for the remainder of the Csound performance. 
Window events and JavaScript callbacks within the browser are dispatched every 
kperiod.

Right-clicking on the browser opens a context menu with a command to open the 
browser's inspector, or debugger. It can be used to view HTML and JavaScript 
code, inspect elements of the Document Object Model, and to set breakpoints or 
inspect variables in JavaScript code.

Once the Web page has opened, Csound can run JavaScript in the JavaScript 
context of that page using the `webkit_run_javascript` opcode.

# webkit_open_html

`webkit_open_html` - Opens a new top-level window and displays in it the content 
defined by the S_html parameter, typically a multi-line string constant contained 
within the `{{` and `}}` delimiters.

## Syntax

webkit_open_html i_webkit_handle, S_window_title, S_html, S_base_uri, i_width, i_height [, i_fullscreen]

## Initialization

*i_webkit_handle* - The handle of a browser created by `webkit_create`.

*S_window_title* - The title to be displayed by the top-level browser window.

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
webkit_open_html gi_browser, "Message", gS_html_code, S_base_uri, 900, 650
```

*i_width* - The width of the top-level browser window in pixels.

*i_height* - The height of the top-level browser window in pixels.

*i_fullscreen* - If 0 (the default value), the size of the browser window is 
`i_width` x `i_height`; if non-0, the browser window is fullscreen.

## Performance

The browser window remains open for the remainder of the Csound performance. 
Window events and JavaScript callbacks within the browser are dispatched every 
kperiod.

In order for user-defined code to call back into Csound, include the 
`csound.js` script that defines the Csound proxy as a script element in the 
body of the Web page. The script element can be loaded from the filesystem, or 
it can be included directly in the Web page's code.

Not only can the Web page call methods of the Csound API, but also Csound can 
run JavaScript in that Web page using the `webkit_run_javascript` opcode.

Right-clicking on the browser opens a context menu with a command to open the 
browser's inspector, or debugger. It can be used to view HTML and JavaScript 
code, inspect elements of the Document Object Model, and to set breakpoints or 
inspect variables in JavaScript code.

## Example

See `webkit_example.js`.

# webkit_run_javascript

`webkit_run_javascript` - Executes JavaScript source code asynchronously in 
the JavaScript context of the browser's default Web page. 

Note that there is a C++ version of the opcode that can be called during the 
performance by C++ code compiled by the Clang just-in-time compiler:
```
extern "C" void webkit_run_javascript(int browser_handle, std::string javascript_code);
```

## Description

`webkit_run_javascript` - Executes JavaScript source code asynchronously in 
the JavaScript context of the browser's default Web page. Note that there is a 
C++ version of the opcode that can be called by C++ code that has been 
compiled by the Clang opcodes during the performance. This can be used to 
to send a generated score in JSON format for display on the page, or to call 
existing functions in the JavaScript context. However, this opcode is 
asynchronous and does not directly return the last value produced by the 
evaluation of the JavaScript code.

It is however possible to have the JavaScript code return a value 
asynchronously, via the "on success" callback of the Csound API call.

Please note, it is quite possible to call `webkit_run_javascript` in the 
orchestra header, before the browser has actually been initialized. In such 
cases, the JavaScript code is enqueued. The code is dequeued and executed when 
(a) the browser has been initialized, and (b) its Web page has finished loading. 
So, it is always safe to call this opcode.

## Syntax
```
i_result webkit_run_javascript i_browser_handle, S_javascript_code
```
## Initialization

*i_browser_handle* - The handle of a browser created by `webkit_create`.

*S_javascript_code* - JavaScript source code that will be executed immediately 
in the JavaScript context of a Web page. Such code can be a single function 
call, or a multi-line string constant contained within the `{{` and `}}` 
delimiters that creates an entire JavaScript module.

Please note, this opcode is designed to work with only one Web page opened 
from one browser instance. The results of trying to call from Csound into more 
than one Web page are undefined. If you need Csound to call into more than one 
Web page, you should create a separate browser on a separate port for each 
page.

Also note, calling this opcode too frequently during a dense performance can 
cause the Web page's user interface to become unresponsive.

## Performance

The JavaScript code executes immediately when the browser has been created and 
its Web page has finished loading, but the code can create modules with 
function definitions, class definitions, and so on that will be available for 
other code on the page.

What happens during performance is whatever the JavaScript code does. Such 
code may execute immediately, or it may create a class or library to be 
invoked later on in the performance.
   
# Installation

1. Install [Csound](https://github.com/csound/csound). On Linux, this 
   generally means building from source code.
3. Install the [WebKitGTK](https://webkitgtk.org/) package and its 
   dependencies, preferably as a system package, e.g. 
   `sudo apt-get install libwebkit2gtk-4.0-dev`.
4. Install [libjson-rpc-cpp](https://github.com/cinemast/libjson-rpc-cpp), 
   preferably as a system package, e.g. 
   `sudo apt-get install libjsonrpccpp-dev libjsonrpccpp-tools`.
4. Generate the stubs and skeletons for the RPC channel that Web pages 
   displayed by the opcodes use to call Csound:
   ```
   jsonrpcstub --verbose csoundrpc.json --js-client=Csound --cpp-server=CsoundSkeleton
   ```   
4. Build the `webkit_opcodes` plugin opcode library by executing `build.sh`. 
   You may need to modify this build script for your system.
5. Test by executing `csound webkit_example.csd`. 

# Credits

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com
