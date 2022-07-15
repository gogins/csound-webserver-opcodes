# csound-webserver {#mainpage}
![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/csound-webserver/total.svg)<br>

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

The Csound webserver opcodes enable the user to define user interfaces, 
generate scores, or control performances using HTML and JavaScript, and 
otherwise use all of the many, many capabilities of standard Web browsers 
to enhance Csound.

The webserver opcodes embed one or more _local, internal Web servers_ 
into the Csound performance, implement a JSON-RPC interface to the running 
instance of Csound, and optionally serve Web pages from the embedded Web 
server(s). Such pages can be embedded in the Csound orchestra code, or 
they can be regular HTML files that refer to other resources. The opcodes will 
optionally run a standard external Web browser to open the served HTML page or 
other resources. 

These opcodes were developed to overcome shortcomings that became apparent in 
csound-extended-node (need for a package configuration file to run pieces), 
webkit-opcodes (lack of consistency between Apple's WebKit and GTK's WebKit), 
and CsoundQt-html5 (HTML support has not always been available in release 
packages).

These are the opcodes: 
```
i_webserver_handle webserver_create S_base_directory, i_port [, i_diagnostics_enabled]
webserver_open_resource i_webserver_handle, S_resource [, S_browser_command]
webserver_open_html i_webserver_handle, S_html_text [, S_browser_command]
webserver_send i_webserver_handle, S_channel_name, S_message
```

### Protocols

The Csound webserver opcodes provide asynchronous, bidirectional 
communications between Csound and Web pages hosted by Csound. 

#### Web Pages to Csound

Web pages use a JavaScript interface to create a `Csound` object that uses 
`fetch` to send JSON-RPC calls and receive return values. The `Csound` object 
defines the following methods, which, as far possible, have the same names and 
behavior as those in `csound.hpp`:
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
Message
ReadScore
RewindScore
ScoreEvent
SetControlChannel
SetDebug
// Subscribes to a named event stream and sends its events to a callback.
SetEventSourceCallback
// Creates an event stream named "csound_message_callback",
// sends Csound's diagnostic messaes on that stream and then to a 
// JavaScript callback.
SetMessageCallback
SetScoreOffsetSeconds
SetScorePending
SetStringChannel
TableGet
TableLength
TableSet
```
Any Web page that uses this interface to communicate with the running instance 
of Csound must include the `csound_jsonrpc_stub.js` script in the HTML 
`<head>` element. This script is distributed in the `examples` directory.

Please note, these methods are asynchronous, but all methods are declared 
`async` so that that they can either be called asynchronously, or called 
synchronously using `await` inside an `async` function.

Also note, each internal webserver can in general fetch any number of Web 
pages, but should host only one page that embeds `csound_jsonrpc_stub.js`_.

Naturally, all Csound API methods that destroy or create Csound, start 
or stop the performance, or configure Csound's audio or MIDI input or output 
drivers have had to be omitted from this interface.

#### Csound to Web Pages

The Csound webserver opcodes use server-sent events to send JSON-encoded data 
to Web pages that create an EventSource.

#### Embedded C++ to Web Pages

In addition, C++ code that is compiled by the csound-cxx-opcodes can send a 
server-sent event to an internal Web server by declaring and calling this 
function:
```
extern "C" void webserver_send(CSOUND *csound, int webserver_handle, const char *channel_name, const char *data);
```
The Web server handle is always 0 for the first (or only) Web server created 
in the Csound performance. Here is an example of loading this function:
```
    extern "C" {
        // Declare a pointer to the function.
        void (webserver_send_message_ptr*)(CSOUND *csound, int webserver_handle, const char *channel_name, const char *message);
    };
    // Library handle 0 means: search all symbols in the process. For this to 
    // work, libcsound_webserver.so (or the equivalent filename on other 
    // platforms) must be in the link library list for THIS code, in order for 
    // the function to be resolved.
    message_send_message_ptr = (void (*)(CSOUND *, int, const char *, const char *)) csound->GetLibrarySymbol(0, "webserver_send_message");
```

# webserver_create

`webserver_create` - Creates and runs an instance of the internal Web server 
embedded into the Csound performance. This is a _local_ Web server that always 
runs on `localhost`. That is because audio input and output are via Csound and 
not via WebAudio, and for reasons of security. These limitations may change in 
future versions.

## Description

Creates an instance an instance of the internal Web server embedded into the 
Csound performance. The embedded Web server, which runs on `localhost`, can 
serve any resources located in its base directory, as well as remote resources.

## Syntax
```
i_webserver_handle webserver_create S_base_directory [, i_port [, i_diagnostics_enabled]]
```
## Initialization

*S_base_directory* - The base direcctory of the embedded Web server. Any local 
resources made available by the Web server must be relative to this base 
directory.

*i_port* - The number of a port on `localhost` that the internal Web server 
will listen on. The default is 8080 which usually works.

*i_diagnostics_enabled* - If 0 (the default), diagnostic messages are not 
printed; if non-0, diagnostic messages are printed.

*i_webserver_handle* - Returns a handle to the newly created internal Web 
server. The other Webserver opcodes must take such a handle as their first 
parameter. 

## Performance

Once created, and whether or not it actually serves any resources, the 
internal Web server keeps running until the end of the Csound performance.

# webserver_open_resource

`webserver_open_resource` - Makes the named resource available from the 
internal Web server. If the named resource is not a complete URL, which 
enables the use of an external resource from the Internet, then the URL 
is formed by appending the resource name to the Web server's origin, e.g. 
`http://localhost:8080/` + `resource.html`.

Please note, Web pages opened with this opcode will not have access to Csound 
unless the body of those pages includes the `csound_jsonrpc_stub.js` script 
for the Csound proxy. That will not normally be the case for Web pages from 
the Internet. Thus, `webserver_open_resource` is primarily useful for opening 
Internet resources such as documentation.

## Syntax
```
webserver_open_resource i_webserver_handle, S_resource [, S_browser_command]
```
## Initialization

*i_webserver_handle* - The handle of a Web server created by 
`webserver_create`.

*S_resource* - The name of an Internet resource to be served by the internal 
Web server. This name, when appended to the base URI of the Web server, 
should form a valid, loadable URI. A complete URL, e.g. for resources from 
the Internet, can also be used.

*S_browser_command* - The text of a command that should immediately open the
named resource in a standard Web browser. This parameter defaults to 
`open` on macOS, which usually defaults to Safari; `xdg-open` on Linux, which 
usually defaults to Firefox; or `start` on Windows, which usually defaults to 
Edge. 

## Performance

The named resource is opened by the named browser in a separate process, and 
can remain open for the duration of the Csound performance. If the named 
resource includes the `csound_jsonrpc_stub.js` script, JavaScript running in 
the context of that resource can call many Csound API methods from a global 
`csound` object, using JSON-RPC.

# webserver_open_html

`webserver_open_html` - Makes the specified text available from the internal 
Web server. Normally, this text is a complete HTML page that can contain 
scripts and hyperlinks. The text is served relative to the base directory of 
the internal Web server.

Please note, Web pages opened with this opcode will not have access to Csound 
unless the body of those pages includes the `csound_jsonrpc_stub.js` script 
for the Csound proxy. 

Also note, only one such page can be hosted by any single webserver opcode.

## Syntax
```
webserver_open_html i_webserver_handle, S_html_text [, S_browser_command]
```
## Initialization

*i_webserver_handle* - The handle of a Web server created by 
`webserver_create`.

*S_html_text* - The complete text for the resource to be served; normally, 
this is a regular HTML page and can contain anything that such a page 
contains.

*S_browser_command* - The text of a command that should immediately open the
named resource in a standard Web browser. This parameter defaults to be 
`open` on macOS, which usually defaults to Safari; `xdg-open` on Linux, which 
usually defaults to Firefox; or `start` on Windows, which usually defaults to 
Edge. 

## Performance

The Web page or other resource is opened by the Web browser in a separate 
process, and can remain open for the duration of the Csound performance. 
JavaScript running in the context of that resource can call many Csound API 
methods from a global `csound` object, using JSON-RPC.

# webserver_send

`webserver_send` - Opens a "channel" through which the running Csound 
orchestra can send data to a Web page opened by `webserver_open_html`. This is 
implemented using server-sent events.

The Web page must define an EventSource for handling those server-sent events. 
For example, to send a notification to the Web page for display in a text 
area, the Web page could contain this code:
```
const csound_notify = new EventSource("csound/notify");
csound_notify.onmessage = function(notification) {
    let notifications_textarea = document.getElementById("notifications_textarea");
    let existing_notifications = notifications_textarea.value;
    notifications_textarea.value = existing_notifications + notification;
}
```

Then, in the Csound orchestra, call the opcode like this to send a 
notification to the Web page:
```
webserver_send i_webserver_handle, "csound/notify", "Hello, World, from Csound!\n"
```

## Syntax
```
webserver_send i_webserver_handle, S_channel_name, S_message
```
## Initialization

*i_webserver_handle* - The handle of a Web server created by 
`webserver_create`.

*S_channel* - The channel name, a resource path that should correspond to a 
matching path in the client.

*S_message* - The body of the message to be sent on the channel. This is mime 
type 'text/event-stream'. It can consist of plain text, JSON text that encodes 
a JavaScript object, or even JavaScript code to be executed in the browser.

## Performance

The first time this opcode is called, the HTTP handler for the server-sent 
event resource is created, and the first message is sent. On subsequent calls, 
the existing handler is used to send the messages.

For each message that is sent, the client handler is notified and receives the 
body of the message.

# Installation

I plan to make these opcodes available from the [Risset](https://github.com/csound-plugins/risset) package manager for Csound.

In the meantime, download the latest (development) binary archive for your operating system from my GitHub repository at https://github.com/gogins/csound-webserver-opcodes/releases. 

Unpack the archive. Add the directory containing the opcodes shared library (for example, on macOS that might be `~/Downloads/csound-webserver-opcodes-1.3.0-Darwin/lib/csound/plugins64-6.0`) to the value of your `OPCODE6DIR64` environment variable.

# Credits

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com
