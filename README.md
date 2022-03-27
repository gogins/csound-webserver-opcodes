# csound-webserver
![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/csound-webserver/total.svg)<br>

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

The csound_webserver opcodes embed an internal Web server into the Csound 
performance, implement a JSON-RPC interface to the running instance of 
Csound, and optionally serve a Web page from the embedded Web server. That 
page can be embedded into the Csound orchestra code, or it can be a regular 
HTML file that refers to other resources. The opcodes will optionally run a
standard external Web browser to open the served HTML page or other resources.

The purpose of these opcodes is to enable the user to define user interfaces, 
generate scores, or control performances using JavaScript, and otherwise use 
all of the many, many capabilities of standard Web browsers in the context of 
the Csound performance.

These opcodes have been developed to overcome shortcomings that became 
apparent in csound-extended-node (need for a package configuration file to run 
pieces), webkit-opcodes (lack of consistency between Apple's WebKit and GTK's 
WebKit), and CsoundQt-html5 (HTML not always available in release packages).

These are the opcodes: 
```
i_webserver_handle webserver_create S_base_uri [, i_port [, i_diagnostics_enabled]]
webserver_open_resource i_webserver_handle, S_resource [, S_browser_command]
webserver_open_html i_webserver_handle, S_html_text [, S_browser_command]
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

For the `webserver_open_html` opcode, the stubs for this interface are 
injected into the HTML `<head>` element.

For the `webserver_open_resource` opcode, any Web page must include the 
`csound.js` script in the HTML `<head>` element.

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

## Syntax
```
i_webserver_handle webserver_create S_base_uri [, i_port [, i_diagnostics_enabled]]

```
## Initialization

*S_base_uri* - The base URI of the embedded Web server. Any resources made 
available by the Web server must be relative to this base URI.

*i_port* - The number of a port on `localhost` that the internal Web server 
will listen on. Usually 8080 will work.

*i_diagnostics_enabled* - If 0 (the default), diagnostic messages are not 
printed; if non-0, diagnostic messages are printed.

*i_webserver_handle* - Returns a handle to the newly created internal Web 
server. The other csound_webserver opcodes must take such a handle as their 
first parameter. 

## Performance

Once created, and whether or not it actually serves any resources, the 
internal Web server keeps running until the end of the Csound performance.

# webserver_open_resource

`webserver_open_resource` - Makes the named resource available from the 
internal Web server. The Web server's base URI plus the named resource 
should form a valid URL.

Please note, Web page opened with this opcode will not have access to Csound 
unless the body of those pages includes the `csound.js` script for the Csound 
proxy. That will not normally be the case for Web pages from the Internet. 
Thus, `webserver_open_resource` is primarily useful for opening Internet 
resources such as documentation.

## Syntax

webserver_open_resource i_webserver_handle, S_resource [, S_browser_command]

## Initialization

*i_webserver_handle* - The handle of a Web server created by 
`webserver_create`.

*S_resource* - The name of an Internet resource to be served by the internal 
Web server. This name, when appended to the base URI of the Web server, 
should form a valid, loadable URI.

*S_browser_command* - The name of a Web browser that should immediately open 
the named resource. This parameter is optional, and will default to Safari on 
macOS, Firefox on Linux, and Edge on Windows. The browser name can be 
continued with browser-specific command-line options, e.g. to resize the 
browser window.

## Performance

The named resource is opened by the named browser in a separate process, and 
can remain open for the duration of the Csound performance. If the named 
resource includes the `csound.js` script, JavaScript running in the context 
of that resource can call many Csound API methods from a global `csound` 
object, using JSON-RPC.

# webserver_open_html

`webserver_open_html` - Makes the specified text available from the 
internal Web server. Normally, this text is a complete HTML page that can 
contain scripts and hyperlinks. The text is served relative to the base 
URI of the internal Web server.

HTML pages opened with this opcode have injected into them the `csound.js` 
script for the Csound proxy. 

## Syntax

webserver_open_html i_webserver_handle, S_html_text [, S_browser_command]

## Initialization

*i_webserver_handle* - The handle of a Web server created by 
`webserver_create`.

*S_html_text* - The complete text for the resource to be served; normally, 
this is a regular HTML page and contain anything that such a page contains.

*S_browser_command* - The name of a Web browser that should immediately open 
the resource. This parameter is optional, and will default to Safari on 
macOS, Firefox on Linux, and Edge on Windows. The browser name can be 
continued with browser-specific command-line options, e.g. to resize the 
browser window.

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
