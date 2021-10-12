# webkit-opcodes
![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/webkit2-opcodes/total.svg)<br>

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

The WebKit opcodes embed the WebKitGTK Web browser and its JavaScript runtime 
into Csound as a set of opcodes. They enable a Csound orchestra to include HTML5 
code, including JavaScript code; to open a browser window during the Csound 
performance; to open an Internet resource during the Csound performande; and to 
communicate back forth between the Document Object Model in the browser and 
Csound, both via an interface to the Csound instance exported to the browser's 
JavaScript context, and via C++ when used with the Clang opcodes.

These are the opcodes: 
```
i_webkit_handle webkit_create
webkit_open_uri i_webkit_handle, S_window_title, S_uri, i_width, i_height
webkit_open_html i_webkit_handle, S_window_title, S_html, S_base_uri, i_width, i_height
webkit_visibility i_webkit_handle, i_visible
webkit_run_javascript i_webkit_handle, S_javascript_code, S_return_channel
webkit_inspector i_webkit_handle
```
In addition, the following JavaScript interface to Csound is defined in the 
JavaScript context of each Web page opened by these opcodes. As far as possible 
this interface is the same as that in `csound.hpp`.
```
class Csound {

};
```


# Installation

1. Install the WebKitGTK package and its dependencies.
2. Install Csound.
3. Build the `webkit_opcodes` plugin opcode library by executing `build.sh`. You may need 
to modify this build script for y0ur system.
5. Test by executing `csound webkit2_example.csd`. 

# Credits

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com
