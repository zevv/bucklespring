Nostalgia bucklespring keyboard sound
=====================================

This project emulates the sound of my old faithful IBM Model-M space saver
bucklespring keyboard while typing on my notebook, mainly for the purpose of
annoying the hell out of my coworkers.

![Model M](model-m.jpg)

Bucklespring runs as a background process and plays back the sound of each key
pressed and released on your keyboard, just as if you were using an IBM
Model-M. The sound of each key has carefully been sampled, and is played back
while simulating the proper distance and direction for a realistic 3D sound
palette of pure nostalgic bliss.

Installation
------------

### Linux

To compile on debian-based linux distributions, first make sure the require
libraries and header files are installed, then simply run `make`:

````
$ sudo apt-get install libopenal-dev libalure-dev libxtst-dev
$ make
$ ./buckle
````

### MacOS

I've heard rumours that bucklespring also runs on MacOS. I've been told that
the following should do:

```
$ brew install alure pkg-config
$ make
```

I was also told the following: "You need superuser privileges to create the
event tap, unless accessibility is enabled. To do so, select the "Enable access
for assistive devices" checkbox in the Universal Access system preference
pane."

Please let me know if this works for you, because I'm not able to test this
myself due to lack of hardware.


### Windows

The windows build is currently broken, it seems that switching from Freelut to
Alure broke windows, I might fix this one day. 

I suspect there is something wrong with `alureCreateBufferFromFile()` getting
called from another thread, but my knowledge of the win32 platform is so poor
I'm not even able to run a debugger to see what is happening. Help from an
expert is appreciated.


Usage
-----

````
usage: ./buckle [options]

options:

  -d DEVICE use OpenAL audio device DEVICE
  -g GAIN   set playback gain [0..100]
  -h        show help
  -l        list available openAL audio devices
  -p PATH   load .wav files from directory PATH
  -s WIDTH  set stereo width [0..100]
  -v        increase verbosity / debugging
````

OpenAL notes
------------

Bucklespring uses the OpenAL library for mixing samples and providing a
realistic 3D audio playback. The default OpenAL settings can cause a slight
delay in playback. Edit or create the OpenAL configuration file `~/.alsoftrc`
and add the following options:

````
period_size = 32
periods = 4
````

If you are using headphones, enabling the head-related-transfer functions in
OpenAL for a better 3D sound:

````
hrtf = true
````
