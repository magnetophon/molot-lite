# Molot Lite

A cut-down version of [Molot](https://www.tokyodawn.net/vladg-molot-compressor), a compressor with a lot of color and character.

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

Oversampling incurs the following latency:  
1x  -> 0 samples  
2x  -> 4 samples  
4x  -> 5 samples  
8x  -> 5 samples  
16x -> 5 samples  


## Dependencies

- `git`
- `build-essential`
- `lv2`
- `pkg-config`
- `cairo`

## Build

```
git clone https://github.com/magnetophon/molot-lite
cd molot-lite
make
make install -C Molot_Mono_Lite
make install -C Molot_Stereo_Lite
```

## Change Log

### Version 1.1.0
- added inline display

### Version 1.0.0
- imported the original source code
- fixed the stereo compressor
- added oversampling
- added gain reduction meters
- added LV2 metadata
- changed "Ratio" from a drop-down to a slider

## Copyright

"Molot Lite" is Copyright (C) 2010-2012  Vladislav Goncharov, and under GPL license <http://www.gnu.org/licenses/>
http://vladgsound.wordpress.com
vl-g@yandex.ru

LV2/LADSPA version by Jeff Glatt, using the "LV2 Create" software.
http://home.roadrunner.com/~jgglatt
jgglatt@roadrunner.com

Various improvements by Jean Pierre Cimalando
jp-dev@inbox.ru
