# waptools
A wifi/access pointer scanner module for ruby (made in C)

# Requirements
This module requires both `ruby-dev` and `libiw-dev` to be installed.
You can install these via your package manager.

For debian:
```bash
$ apt-get update && apt-get install ruby-dev libiw-dev
```

# Build
If you haven't already, clone this repository and `cd` into it:

```bash
$ git clone https://github.com/ripmeep/waptools/ && cd waptools
```

Then, run `extconf.rb` with ruby:

```bash
$ ruby extconf.rb
creating extconf.h
creating Makefile
```

This will create 2 files, `extconf.h` which you DO NOT need to touch.
It will also create a file called `Makefile`.
Once this has been created, open `Makefile` in a text editor of your choice and find the line below (it should be line 140 but this can differ for some users):

```bash
LIBS = $(LIBRUBYARG_SHARED)  -lm   -lc
```

Add the string `-liw` to the end of the line so it looks like this:

```bash
LIBS = $(LIBRUBYARG_SHARED)  -lm   -lc -liw
```

Save it and close the editor you are using.
Next, run `make`:

```bash
$ make
compiling waptools.c
linking shared-object waptools.so
```

If any errors/warnings show up at this point, please let me know via the issues tab.
The `make` command should create a file called `waptools.so`. You can rename this to `waptools` if you would like, but make sure to specify that change in your ruby file

```ruby
require './waptools.so'
# OR
require './waptools'
```

# Usage
Now that you have compiled it, you can go ahead and start using it in your ruby project.
Make sure the compiled `waptools.so` file is in a directory that ruby can see and `require` it as such.

For testing, go ahead and run the `test.rb` file in this repository too.

```
$ ruby test.rb wlan0   # change wlan0 to your wireless interface name!
Welcome to WAPTools! This module was made by ripmeep
Instagram: @rip.meep
GitHub:    https://github.com/ripmeep/

SSID     : Some_Wifi
BSSID    : 12:34:56:AB:CD:EF
Frequency: 2.437 GHz
Bitrate  : {"value"=>54, "rate"=>"Mb/s"}
Stats    : Quality=21/70  Signal level=-89 dBm  
Channel  : 6

SSID     : RandomAP
BSSID    : DE:AD:BE:EF:11:22
Frequency: 2.462 GHz
Bitrate  : {"value"=>54, "rate"=>"Mb/s"}
Stats    : Quality=23/70  Signal level=-87 dBm  
Channel  : 11
```
