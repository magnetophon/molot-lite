#include <new>
#define I_AM_LADSPA 1
#include "Molot_Stereo_Lite.h"

#define LADSPA_ID_NUM 12448101
#define PLUGIN_STR_NAME "Molot Stereo Lite"
#define PLUGIN_CLASS_NAME "MolotStereoLite"

static const char Blank[] = "";

typedef struct {
	int	Hint;
	float	Lower, Upper;
} PORTRANGEHINT;

typedef struct _LV2_Descriptor {
	unsigned long		UniqueID;
	const char *		Label;
	int					Props;
	const char *		Name;
	const char *		Maker;
	const char *		Copyright;
	unsigned long		PortCount;
	const int *			PortDescs;
	const char * const * PortNames;
	const PORTRANGEHINT *	PortHints;
	void *				ImpData;
	LV2_Handle   (*instantiateF)(const struct _LV2_Descriptor *, unsigned long);
	void (*connect_port)(LV2_Handle, unsigned long, void *);
	void (*activateF)(LV2_Handle);
	void (*runF)(LV2_Handle, unsigned long);
	void (*run_2)(void);
	void (*run_3)(void);
	void (*deactivateF)(LV2_Handle);
	void (*cleanupF)(LV2_Handle);
} LV2_Descriptor;

static const PORTRANGEHINT PortRangeHints[LV2_NUM_ALL_PORTS] = {
	{0, 0.0f, 0.0f},
	{0, 0.0f, 0.0f},
	{0, 0.0f, 0.0f},
	{0, 0.0f, 0.0f},
	{195, -12.f, 12.f},
	{67, 40.f, 260.f},
	{195, -40.f, 0.f},
	{323, 0.f, 1.f},
	{195, 1.f, 100.f},
	{67, 0.f, 1.f},
	{195, 50.f, 1000.f},
	{195, 1.1f, 11.f},
	{195, -12.f, 12.f},
	{67, 0.f, 99.f},
	{67, 0.f, 5.f}};

static const char *	PortNames[LV2_NUM_ALL_PORTS] = {
	Blank,
	Blank,
	Blank,
	Blank,
	"Input",
	"Filter",
	"Threshold",
	"Knee",
	"Attack",
	"Atk Mode",
	"Release",
	"Ratio",
	"Makeup",
	"Dry / Wet",
	"Stereo mode"};

static const int					PortDescriptors[LV2_NUM_ALL_PORTS] = {
	9,
	9,
	10,
	10,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5};





/*************************** run() ***************************
 * Called by host to add DSP to the waveform data in the
 * input buffer(s), and stores the new data in the output
 * buffer(s).
 *
 * instance = Ptr to our MolotStereoLite.
 * sampleFrames =	How many floats to process per channel.
 */

static void run(LV2_Handle instance, unsigned long sampleFrames)
{
	MolotStereoLite *	plugin;

	if ((plugin = static_cast<MolotStereoLite *> (instance)))
	{
		plugin->Host.Begin = 0;

		plugin->Host.End = sampleFrames - plugin->Host.Begin;
		plugin->pluginRun();
	}
}





/************************* cleanup() **************************
 * Called by host to free the plugin instance (i.e. the
 * MolotStereoLite gotten via instantiate()).
 *
 * instance =	Our own MolotStereoLite.
 */

static void cleanup(LV2_Handle instance)
{
	if (instance)
	{
		// Get rid of the MolotStereoLite
		{
		MolotStereoLite * plugin;

		plugin = static_cast<MolotStereoLite *> (instance);
		delete plugin;
		}
	}
}





/*********************** instantiate() ************************
 * Called by host to alloc/init a new plugin instance.
 *
 * RETURNS: If success, a pointer to our own MolotStereoLite
 * that holds all data needed to implement our plugin.
 */

static LV2_Handle instantiate(const LV2_Descriptor * descriptor, unsigned long rate)
{
	MolotStereoLite *	plugin;

	// Create an instance of MolotStereoLite (defined in Molot_Stereo_Lite.h)
	plugin = new (std::nothrow) MolotStereoLite((double)rate);
	if (plugin)
	{
		plugin->Host.SampleRate = (float)rate;

	}

	// Return our MolotStereoLite to the host (and the
	// host passes back to our functions)
	return (LV2_Handle)plugin;
}





/************************ activate() *************************
 * Called by host to reset the state of a plugin, once before
 * our host starts making multiple calls to our run function.
 *
 * instance =	Our own MolotStereoLite.
 */

static void activate(LV2_Handle instance)
{
	MolotStereoLite *	plugin;

	if ((plugin = static_cast<MolotStereoLite *> (instance))) plugin->pluginActivate();
}





/*********************** deActivate() ************************
 * Called by host once after done making multiple calls to our
 * run function.
 *
 * instance =	Our own MolotStereoLite.
 */

static void deActivate(LV2_Handle instance)
{
	MolotStereoLite *	plugin;

	if ((plugin = static_cast<MolotStereoLite *> (instance))) plugin->pluginDeActivate();
}





/*********************** connectPort() ************************
 * Called by host to connect a port to a data location (ie, allow
 * us to store a pointer to a host-supplied data or buffer. NOTE:
 * all host data is a float).
 *
 * instance =	Our own MolotStereoLite.
 * port =		Indicates what hostData is being supplied.
 * hostData =	Pointer to host float data, or buffer of floats.
 */

static void connectPort(LV2_Handle instance, unsigned long port, void * hostData)
{
	MolotStereoLite *	plugin;

	plugin = static_cast<MolotStereoLite *> (instance);
	if (plugin && port < LV2_NUM_ALL_PORTS)
	{
				if (port < LV2_NUM_AUDIO_INS)
			plugin->Host.AudioIn[port] = (float *)hostData;
		else
		{
			port -= LV2_NUM_AUDIO_INS;
			if (port < LV2_NUM_AUDIO_OUTS)
				plugin->Host.AudioOut[port] = (float *)hostData;
			else
			{
				port -= LV2_NUM_AUDIO_OUTS;
				plugin->Host.In[port] = (float *)hostData;
			}
		}
	}
}





/*
 */
static const LV2_Descriptor  PluginDescriptor = {
	LADSPA_ID_NUM,
	PLUGIN_CLASS_NAME,
	4,
	PLUGIN_STR_NAME,
	Blank,
	Blank,
	LV2_NUM_ALL_PORTS,
	PortDescriptors,
	PortNames,
	PortRangeHints,
	0,
	instantiate,
	connectPort,
	activate,
	run,
	0, 0,
	deActivate,
	cleanup,
};

const LV2_Descriptor * ladspa_descriptor(unsigned long index)
{
	if (!index) return &PluginDescriptor;
	return 0;
}
