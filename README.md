Nostalgia bucklespring keyboard sound

````
$ sudo apt-get install libopenal-dev libalut-dev libxtst-dev
$ make
$ ./buckle
````


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
