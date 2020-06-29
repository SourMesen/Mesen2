---
title: Lua API reference
weight: 55
chapter: false
toc: false
---

This section documents the Mesen-specific Lua API that is available in scripts via the [script window](/debugging/scriptwindow.html).

## Changelog ##

To get a list of API changes between different versions of Mesen-S, take a look at the [Changelog](/apireference/changelog.html).

## API References ##

* [Callbacks](/apireference/callbacks.html)
* [Drawing](/apireference/drawing.html)
* [Emulation](/apireference/emulation.html)
* [Input](/apireference/input.html)
* [Logging](/apireference/logging.html)
* [Memory Access](/apireference/memoryaccess.html)
* [Miscellaneous](/apireference/misc.html)
* [Enums](/apireference/enums.html)


## Additional features ##

### LuaSocket ###

The Lua implementation found in Mesen-S has a version of [LuaSocket](http://w3.impa.br/~diego/software/luasocket/) ([GitHub](https://github.com/diegonehab/luasocket)) built into it.  The `socket` and `mime` packages are available and can be accessed by using `local socket = require("socket.core")` and `local mime = require("mime.core")`, respectively.  

See [LuaSocket's documentation](http://w3.impa.br/~diego/software/luasocket/reference.html) for more information on how to use this library.

Here is a tiny TCP socket sample that connects to google.com via HTTP and downloads the page:
```
local socket = require("socket.core")
local tcp = sock.tcp()

--Set a 2-second timeout for all request, otherwise the process could hang!
tcp:settimeout(2)
local res = tcp:connect("www.google.com", 80)
tcp:send("GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n")

local text
repeat
   text = tcp:receive()  
   emu.log(text)
until text == nil
```

{{% notice warning %}}
Using sockets without calling the `settimeout(seconds)` function (and specifying a reasonable number of seconds) first can result in the Mesen-S process hanging until the socket finishes the operation it is waiting for.  
For this reason, it is highly recommended to **ALWAYS** call `settimeout(seconds)` on any newly created TCP/etc object before calling any other function on it.
{{% /notice %}}
