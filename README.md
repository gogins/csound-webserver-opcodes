# webkit2-opcodes
![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/webkit2-opcodes/total.svg)<br>

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

The WebKit2 opcodes embed the WebKitGTK Web browser and its JavaScript runtime 
into Csound as a set of opcodes. They enable a Csound orchestra to include HTML5 
code, including JavaScript code; to open a browser window during the Csound 
performance; and to communicate back forth between the Document Object Model in 
the browser and Csound, both via an interface to the Csound instance exported 
to the browser's JavaScript context, and via C++ when used with the Clang opcodes.

# webkit2_window

`webkit2_window` - Opens a top-level window containing an instance of the WebKit2
browser.

## Description

`webkit2_window` - Opens a top-level window containing an instance of the WebKit2
browser. This browser can basically do anything any browser can do, and also contains 
a built-in Csound object. The browser can display a user-defined user interface to 
the Csound orchestra, take control of the Csound performance, display a generated score, 
and so on.

## Syntax
```
i_handle webkit2_window S_window_title, i_width, i_height [, S_html5_code, S_callback_channel]
```
## Initialization

*S_window_title* - Title shown in the frame of the browser window.

*i_width* - Width in pixels of the browser window. A negative value means full-screen.

*i_height* - Height in pixels of the browser window. A negative value means full-screen.

*S_html5_code* - If present, this code is loaded into the browser and displayed. It can 
include any valid HTML5 code including HTML, JavaScript, CSS, etc.

*S_callback_channel* - The name of a Csound control channel on which the results of 
asynchronous method invocations will be returned.

*i_handle* - A negative number is returned if there is an error. On success, a positive 
number is returned that serves as a handle to the browser window for the other opcodes.

## Performance

The browser window is opened at the requested size, and the `S_html5_code`, if present, 
is loaded and executed.

The running Csound instance is injected into the browser's JavaScript context as a 
JavaScript object. The methods of this object are asynchronous. The results of calling 
these methods are returned on a Csound control channel named `S_callback_channel`.

In addition, the Csound object can send and receives messages on any Csound control 
channel.

Thereafter, the window events and other events of the browser are handled by polling the 
browser's event loop every kperiod.

# Installation

1. Install the WebKitGTK package and its dependencies.
2. Install Csound.
3. Build the `webkit2_opcodes` plugin opcode library by executing `build.sh`. You may need 
to modify this build script for y0ur system.
5. Test by executing `csound webkit2_example.csd`. 

# Credits

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com
