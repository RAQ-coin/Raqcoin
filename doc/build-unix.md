Copyright (c) 2018 AbcmintCore Developers

Raqcoincoie is released under the terms of the GNU GPL v. 3 license. 
See https://www.gnu.org/licenses/gpl-3.0.en.html for more information.
This product includes software developed by the OpenSSL Project for use in the [OpenSSL Toolkit](http://www.openssl.org/). This product includes
cryptographic software written by Eric Young ([eay@cryptsoft.com](mailto:eay@cryptsoft.com)), and UPnP software written by Thomas Bernard.

UNIX BUILD NOTES
====================

To Build
---------------------

	cd src/
	make -f makefile.unix		# Headless raqcoin

See readme-qt.rst for instructions on building Raqcoin-Qt, the graphical user interface.

Dependencies
---------------------

 Library     Purpose           Description
 -------     -------           -----------
 libssl      SSL Support       Secure communications
 libdb       Berkeley DB       Blockchain & wallet storage
 libboost    Boost             C++ Library
 miniupnpc   UPnP Support      Optional firewall-jumping support

[miniupnpc](http://miniupnp.free.fr/) may be used for UPnP port mapping.  It can be downloaded from [here](
http://miniupnp.tuxfamily.org/files/).  UPnP support is compiled in and
turned off by default.  Set USE_UPNP to a different value to control this:

	USE_UPNP=     No UPnP support miniupnp not required
	USE_UPNP=0    (the default) UPnP support turned off by default at runtime
	USE_UPNP=1    UPnP support turned on by default at runtime

IPv6 support may be disabled by setting:

	USE_IPV6=0    Disable IPv6 support

Licenses of statically linked libraries:
 Berkeley DB   New BSD license with additional requirement that linked
               software must be free open source
 Boost         MIT-like license
 miniupnpc     New (3-clause) BSD license

- Versions used in this release:
-  GCC           6.2.0
-  OpenSSL       1.1.0g
-  Berkeley DB   5.1.29
-  Boost         1.65
-  miniupnpc     2.1

Dependency Build Instructions: Ubuntu & Debian
----------------------------------------------
Build requirements:

	sudo apt-get install build-essential
	sudo apt-get install libssl-dev

for Ubuntu 12.04:

	sudo apt-get install libboost-all-dev

 db4.8 packages are available [here](https://launchpad.net/~abcmint/+archive/abcmint).

 Ubuntu precise has packages for libdb5.1-dev and libdb5.1++-dev.

Optional:

	sudo apt-get install libminiupnpc-dev (see USE_UPNP compile flag)

Notes
-----
The release is built with GCC and then "strip raqcoin" to strip the debug
symbols, which reduces the executable size by about 90%.


miniupnpc
---------
	tar -xzvf miniupnpc-2.1.tar.gz
	cd miniupnpc-2.1
	make
	sudo su
	make install


Berkeley DB
-----------
You need Berkeley DB 5.1.  If you have to build Berkeley DB yourself:

	../dist/configure --enable-cxx
	make


Boost
-----
If you need to build Boost yourself:

	sudo su
	./bootstrap.sh
	./bjam install


Security
--------
To help make your raqcoin installation more secure by making certain attacks impossible to
exploit even if a vulnerability is found, you can take the following measures:

* Position Independent Executable
    Build position independent code to take advantage of Address Space Layout Randomization
    offered by some kernels. An attacker who is able to cause execution of code at an arbitrary
    memory location is thwarted if he doesn't know where anything useful is located.
    The stack and heap are randomly located by default but this allows the code section to be
    randomly located as well.

    On an Amd64 processor where a library was not compiled with -fPIC, this will cause an error
    such as: "relocation R_X86_64_32 against `......' can not be used when making a shared object;"

    To build with PIE, use:

    	make -f makefile.unix ... -e PIE=1

    To test that you have built PIE executable, install scanelf, part of paxutils, and use:

    	scanelf -e ./abcmint

    The output should contain:
     TYPE
    ET_DYN

* Non-executable Stack
    If the stack is executable then trivial stack based buffer overflow exploits are possible if
    vulnerable buffers are found. By default, raqcoin should be built with a non-executable stack
    but if one of the libraries it uses asks for an executable stack or someone makes a mistake
    and uses a compiler extension which requires an executable stack, it will silently build an
    executable without the non-executable stack protection.

    To verify that the stack is non-executable after compiling use:
    `scanelf -e ./abcmint`

    the output should contain:
	STK/REL/PTL
	RW- R-- RW-

    The STK RW- means that the stack is readable and writeable but not executable.
