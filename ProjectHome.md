# Audio Tools #

Audio tools for testing audio equipment in conjunction with a PC. Windows and Mac versions have been released. Display shows frequency and relative level. Absolute level is not possible because of all the variables in the sound system.

## Audio Signal Generator ##

![http://audiotools.googlecode.com/files/Siggen.png](http://audiotools.googlecode.com/files/Siggen.png)

  * Frequency range 10Hz - 25KHz
  * Level range -6.0dB - -80dB

### Using ###
The frequency knob responds to mouse dragging. It is possible to twirl it using the mouse. The fine frequency and level sliders allow for fine adjustments of frequency and output level. The frequency knob is also adjustable using the left and right arrow keys. The fine frequency slider is adjustable using the up and down arrow keys, the level slider using the page up and down keys.

## Audio Level Measuring Set ##

![http://audiotools.googlecode.com/files/LMS.png](http://audiotools.googlecode.com/files/LMS.png)

Audio level measuring set. Measures frequency and relative level with a frequency spectrum display. Absolute level is not possible because of all the variables in the sound system.

### Using ###

The level measuring set displays frequency and relative level with a frequency spectrum display. Input is from the currently selected audio input.

## Audio Selective Level Measuring Set ##
![http://audiotools.googlecode.com/files/SLMS.png](http://audiotools.googlecode.com/files/SLMS.png)

Audio selective level measuring set. Measures relative level at selected frequency with a frequency spectrum display. Absolute level is not possible because of all the variables in the sound system.

### Using ###

The selective level measuring set displays selected frequency and relative level with a frequency spectrum display. Use mouse to turn knob, or left and right keys to adjust selected frequency. Input is from the currently selected audio input.

## Audio Oscilloscope ##
![http://audiotools.googlecode.com/files/Scope.png](http://audiotools.googlecode.com/files/Scope.png)

  * Timebase 0.1ms - 0.5sec
  * Resolution 0.022ms @ 44100 samples/sec
  * Single shot
  * Storage
  * Cursor

### Using ###
Use the toolbar buttons to access functionality. From left to right: Input level control, bright line turns off sync, single shot, trigger, sync polarity, timebase menu, storage mode, display clear, shift left, shift right, shift to start, shift to end, reset. Clicking on the scope display turns on the cursor line, which may be shifted left and right using the left and right arrow keys. The cursor line shows the time from the start of the trace in current timebase units, and the nominal trace value at that point. There is no calibration because of the various level controls in the audio system. The shift to start and reset toolbar buttons clear the cursor.

### Bugs ###
In accordance with Just in Time Programming<sup>TM</sup> principles, the input level control does not exist until the Input Level toolbar button is clicked, also, in accordance with Resource Conservation<sup>GM</sup>, it is conserved until the Scope program is closed. This means that if you change the audio input source using the [Recording Devices](http://www.howtogeek.com/howto/39532/how-to-enable-stereo-mix-in-windows-7-to-record-audio) pane after using the level control, it will be still connected to the original source. To fix this quit and restart the program.
