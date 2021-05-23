# Molot Lite

A cut-down version of Molot, compressor a compressor with a lot of color and character..
https://www.tokyodawn.net/vladg-molot-compressor

There are mono and stereo versions, for LV2 and LADSPA.

## Usage

If the plugin is taxing your CPU too much, reduce or turn off oversampling.
The input gain affects the amount of distortion.
There are two attack modes:
- Sigma: accentuates the transients.
- Alpha: is smoother.
The stereo version has 6 stereo modes:
- Stereo: The average of the gain reduction of left and right is applied to both channels.
- 2 Mono: Two separate un-linked channels.
- R-S.chn: The right channel becomes the side-chain.
- Mid:
  A mid-side encoder, with only the mid going trough the compressor, followed by a MS decoder.
- Side:
  A mid-side encoder, with only the side going trough the compressor, followed by a MS decoder.
- M/S:
  A mid-side encoder, both channels going trough the compressor, followed by a MS decoder.
  The maximum GR of either channel is applied to both channels.


## Dependencies

- `git`
- `build-essential`
- `lv2

## Build

```
git clone https://github.com/magnetophon/molot-lite
cd molot-lite
make
make install -C Molot_Mono_Lite
make install -C Molot_Stereo_Lite
```

## Change Log

### Version 1.0.0
- imported the original source code
- fixed the stereo compressor
- added oversampling
- added gain reduction meters
- added LV2 metadata

## Copyright

"Molot Lite" is Copyright (C) 2010-2012  Vladislav Goncharov, and under GPL license <http://www.gnu.org/licenses/>
http://vladgsound.wordpress.com
vl-g@yandex.ru

LV2/LADSPA version by Jeff Glatt, using the "LV2 Create" software.
http://http://home.roadrunner.com/~jgglatt
jgglatt@roadrunner.com

Various improvements by Jean Pierre Cimalando
jp-dev@inbox.ru
