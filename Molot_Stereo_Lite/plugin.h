#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#if I_AM_LADSPA == 1
typedef void * LV2_Handle;
#else
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2_rgext.h>
#if I_AM_LADSPA != 1
#include <cairo.h>
#endif

// URI strings
#define URI_STRING "urn:magnetophon:Molot_Stereo_Lite"
#endif

// Port indexes
#define INDEX_AUDIO_IN_1 0
#define INDEX_AUDIO_IN_2 1
#define INDEX_AUDIO_OUT_1 2
#define INDEX_AUDIO_OUT_2 3
#define INDEX_INPUT_4
#define INDEX_FILTER_5
#define INDEX_THRESHOLD_6
#define INDEX_KNEE_7
#define INDEX_ATTACK_8
#define INDEX_ATK_MODE_9
#define INDEX_RELEASE_10
#define INDEX_RATIO_11
#define INDEX_MAKEUP_12
#define INDEX_DRY_WET_13
#define INDEX_STEREO_MODE_14
#define INDEX_OVERSAMPLING_15
#define INDEX_GAIN_REDUCTION_1_16
#define INDEX_GAIN_REDUCTION_2_17

// Misc stuff for the skeleton code
#define LV2_NUM_AUDIO_INS 2
#define LV2_NUM_AUDIO_OUTS 2
#define LV2_NUM_INS 12
#define LV2_NUM_OUTS 2
#define LV2_NUM_ALL_PORTS 18
#define LV2_MIDI_OPTION 0
#define LV2_STATE_OPTION 0
#define LV2_EXT_OPTION 0
#define LV2_GUI_OPTION 0
#define LV2_ACT_OPTION 1
#define LV2_DEACT_OPTION 1

// Indexes for accessing struct MOLOT_STEREO_LITE->In[] array
#define LV2_INPUT 0
#define LV2_FILTER 1
#define LV2_THRESHOLD 2
#define LV2_KNEE 3
#define LV2_ATTACK 4
#define LV2_ATK_MODE 5
#define LV2_RELEASE 6
#define LV2_RATIO 7
#define LV2_MAKEUP 8
#define LV2_DRY_WET 9
#define LV2_STEREO_MODE 10
#define LV2_OVERSAMPLING 11

// Indexes for accessing struct MOLOT_STEREO_LITE->Out[] array
#define LV2_GAIN_REDUCTION_1 0
#define LV2_GAIN_REDUCTION_2 1

// Your plugin struct that the host passes to your DSP functions
typedef struct {
	float *	AudioIn[LV2_NUM_AUDIO_INS];
	float *	AudioOut[LV2_NUM_AUDIO_OUTS];
	float *	In[LV2_NUM_INS];
	float *	Out[LV2_NUM_OUTS];
	uint32_t	Begin;
	uint32_t	End;
	float		SampleRate;
#if I_AM_LADSPA != 1
	int	DisplayChannels;
	float	DisplayLevel[2];
	cairo_surface_t *	CairoSurface;
	cairo_pattern_t *	CairoPattern;
	LV2_Inline_Display *	QueueDraw;
	LV2_Inline_Display_Image_Surface	DisplaySurface;
#endif
} MOLOT_STEREO_LITE ;
