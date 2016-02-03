= Nostalgia bucklespring keyboard sound

This project emulates the sound of my old faithful IBM Model-M bucklespring
keyboard while typing on my notebook, with the main function of annoying the
hell out of my coworkers.

![Model M](model-m.jpg)


== Installation

````
$ sudo apt-get install libopenal-dev libalut-dev libxtst-dev
$ make
$ ./buckle
````


== Usage

````
usage: ./buckle [options]

valid options:
  -d DEVICE use OpenAL audio device DEVICE
  -h        show help
  -l        list available openAL audio devices
  -p PATH   load .wav files from directory PATH
  -s WIDTH  set stereo width [0 .. 100]
  -v        increase verbosity / debugging
````

Notes: bucklespring uses the OpenAL library for mixing samples and providing a
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
