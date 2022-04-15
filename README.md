# csound-webserver {#mainpage}
![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/csound-webserver/total.svg)<br>

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

The csound_webserver opcodes embed a _local_, internal Web server into the 
Csound performance, implement a JSON-RPC interface to the running instance of 
Csound, and optionally serve Web pages from the embedded Web server. Such 
pages can be embedded into the Csound orchestra code, or they can be a regular 
HTML file that refers to other resources. The opcodes will optionally run a
standard external Web browser to open the served HTML page or other resources. 
This behaves more or less as though Csound had embedded within it a complete 
Web browser.

The purpose of these opcodes is to enable the user to define user interfaces, 
generate scores, or control performances using JavaScript, and otherwise use 
all of the many, many capabilities of standard Web browsers in the context of 
the Csound performance.

These opcodes were developed to overcome shortcomings that became apparent in 
csound-extended-node (need for a package configuration file to run pieces), 
webkit-opcodes (lack of consistency between Apple's WebKit and GTK's WebKit), 
and CsoundQt-html5 (HTML not always available in release packages).

These are the opcodes: 
```
i_webserver_handle webserver_create S_base_directory, i_port [, i_diagnostics_enabled]
webserver_open_resource i_webserver_handle, S_resource [, S_browser_command]
webserver_open_html i_webserver_handle, S_html_text [, S_browser_command]
```
The following JavaScript interface can be used from the JavaScript context of 
a Web page opened by these opcodes. As far as possible, the methods of this 
interface are the same as those in `csound.hpp`:
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
SetMessageCallback
SetScoreOffsetSeconds
SetScorePending
SetStringChannel
TableGet
TableLength
TableSet
```

Any Web page that needs to communicate with the running instance of Csound 
must include the `csound_jsonrpc_stub.js` script in the HTML `<head>` element. 
This script is distributed in the `examples` directory.

Please note, these methods are asynchronous, but all methods are declared 
`async` so that that they can either be called asynchronously, or called 
synchronously using `await` inside an `async` function.

Also note, each webserver opcode can in general host any number of Web pages, 
_but only one Web page that embeds `csound_jsonrpc_stub.js`_.

Naturally, all Csound API methods that destroy or create Csound, start 
or stop the performance, or configure Csound's audio or MIDI input or output 
drivers have had to be omitted from this interface.

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
will listen on. Usually 8080 will work.

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

webserver_open_resource i_webserver_handle, S_resource [, S_browser_command]

## Initialization

*i_webserver_handle* - The handle of a Web server created by 
`webserver_create`.

*S_resource* - The name of an Internet resource to be served by the internal 
Web server. This name, when appended to the base URI of the Web server, 
should form a valid, loadable URI. A complete URL, e.g. for resources from 
the Internet, can also be used.

*S_browser_command* - The text of a command that should immediately open the
named resource in a standard Web browser. This parameter should normally be 
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

webserver_open_html i_webserver_handle, S_html_text [, S_browser_command]

## Initialization

*i_webserver_handle* - The handle of a Web server created by 
`webserver_create`.

*S_html_text* - The complete text for the resource to be served; normally, 
this is a regular HTML page and can contain anything that such a page 
contains.

*S_browser_command* - The text of a command that should immediately open the
named resource in a standard Web browser. This parameter should normally be 
`open` on macOS, which usually defaults to Safari; `xdg-open` on Linux, which 
usually defaults to Firefox; or `start` on Windows, which usually defaults to 
Edge. 

## Performance

The Web page or other resource is opened by the Web browser in a separate 
process, and can remain open for the duration of the Csound performance. 
JavaScript running in the context of that resource can call many Csound API 
methods from a global `csound` object, using JSON-RPC.

# Installation

1. Install [Csound](https://github.com/csound/csound). On Linux, this 
   generally means building from source code.
2. Build the `webserver_opcodes` plugin opcode library by executing `build.sh`. 
   You may need to modify this build script for your system.
3. Test by executing `csound webserver_example.csd`. 

# Credits

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com
