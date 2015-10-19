# gi
Fixes your mistyped gi tcommands

Do you type a little too fast sometimes? Is the shell just a little too literal for your needs? Don't waste time fixing your typos by hand. Let gi take care of it for you. Just mistype your commands like you're doing now, and `gi` will automatically correct them for you. For instance, if you type `gi tlog`, gi will run `git log` for you... after harassing you a little bit.

## Installation

Simply type `make` and copy the resulting `gi` executable to somewhere in your PATH. It should build on any Unix-ish system but probably not on Windows. For the best experience you should also install cowsay.

# Usage

In the simplest form, just mistype your git commands as `gi t<your git arguments>` and git will do the rest. There are also a couple of environment variables that you can use to control gi's behaivor:

`GI_COWFILE`: If cowsay is present, gi will use it to harass you about your typo. The `GI_COWFILE` environment variable determines which cowfile gi will use.

`STOP_REMINDING_ME_THAT_I_SUCK_AT_TYPING`: Turns off all harassment. gi will simply run the command that you inteded without delay or commentary.
