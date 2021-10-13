#include <new>
#include <algorithm>
#include <cmath>
#include "Molot_Mono_Lite.h"

#define SIZEOF_URI_STRING 20
#define SIZEOF_EXT_URI_STRING 25

// Our own URI we made up for our plugin. Must match the one in our TTL file
static const char UriStr[] = URI_STRING;

static const char MapUriStr[] = "http://lv2plug.in/ns/ext/urid#map";





/*************************** run() ***************************
 * Called by host to add DSP to the waveform data in the
 * input buffer(s), and stores the new data in the output
 * buffer(s).
 *
 * instance = Ptr to our MolotMonoLite.
 * sampleFrames =	How many floats to process per channel.
 */

static void run(LV2_Handle instance, uint32_t sampleFrames)
{
	MolotMonoLite *	plugin;

	if ((plugin = static_cast<MolotMonoLite *> (instance)))
	{
		plugin->Host.Begin = 0;

		plugin->Host.End = sampleFrames - plugin->Host.Begin;
		plugin->pluginRun();
	}
}





/************************* cleanup() **************************
 * Called by host to free the plugin instance (i.e. the
 * MolotMonoLite gotten via instantiate()).
 *
 * instance =	Our own MolotMonoLite.
 */

static void cleanup(LV2_Handle instance)
{
	if (instance)
	{
		// Get rid of the MolotMonoLite
		{
		MolotMonoLite * plugin;

		plugin = static_cast<MolotMonoLite *> (instance);

		if (plugin->Host.CairoSurface)
			cairo_surface_destroy (plugin->Host.CairoSurface);
		if (plugin->Host.CairoPattern)
			cairo_pattern_destroy (plugin->Host.CairoPattern);

		delete plugin;
		}
	}
}





/*********************** instantiate() ************************
 * Called by host to alloc/init a new plugin instance.
 *
 * RETURNS: If success, a pointer to our own MolotMonoLite
 * that holds all data needed to implement our plugin.
 */

static LV2_Handle instantiate(const LV2_Descriptor * descriptor, double rate, const char * path, const LV2_Feature * const * features)
{
	MolotMonoLite *	plugin;

	// Create an instance of MolotMonoLite (defined in Molot_Mono_Lite.h)
	plugin = new (std::nothrow) MolotMonoLite((double)rate);
	if (plugin)
	{
		plugin->Host.SampleRate = (float)rate;

	}

	plugin->Host.QueueDraw = nullptr;

	for (size_t i = 0; features[i]; ++i)
	{
		if (!strcmp (features[i]->URI, LV2_INLINEDISPLAY__queue_draw))
			plugin->Host.QueueDraw = (LV2_Inline_Display *)features[i]->data;
	}

	plugin->Host.DisplayChannels = 0;
	plugin->Host.DisplayLevel[0] = 0.0f;

	plugin->Host.CairoSurface = nullptr;
	plugin->Host.CairoPattern = nullptr;

	// Return our MolotMonoLite to the host (and the
	// host passes back to our functions)
	return (LV2_Handle)plugin;
}





/************************ activate() *************************
 * Called by host to reset the state of a plugin, once before
 * our host starts making multiple calls to our run function.
 *
 * instance =	Our own MolotMonoLite.
 */

static void activate(LV2_Handle instance)
{
	MolotMonoLite *	plugin;

	if ((plugin = static_cast<MolotMonoLite *> (instance))) plugin->pluginActivate();
}





/*********************** deActivate() ************************
 * Called by host once after done making multiple calls to our
 * run function.
 *
 * instance =	Our own MolotMonoLite.
 */

static void deActivate(LV2_Handle instance)
{
	MolotMonoLite *	plugin;

	if ((plugin = static_cast<MolotMonoLite *> (instance))) plugin->pluginDeActivate();
}





/*********************** connectPort() ************************
 * Called by host to connect a port to a data location (ie, allow
 * us to store a pointer to a host-supplied data or buffer. NOTE:
 * all host data is a float).
 *
 * instance =	Our own MolotMonoLite.
 * port =		Indicates what hostData is being supplied.
 * hostData =	Pointer to host float data, or buffer of floats.
 */

static void connectPort(LV2_Handle instance, uint32_t port, void * hostData)
{
	MolotMonoLite *	plugin;

	plugin = static_cast<MolotMonoLite *> (instance);
	if (plugin && port < LV2_NUM_ALL_PORTS)
	{
		if (port < LV2_NUM_AUDIO_INS)
		{
			plugin->Host.AudioIn[port] = (float *)hostData;
			return;
		}
		port -= LV2_NUM_AUDIO_INS;

		if (port < LV2_NUM_AUDIO_OUTS)
		{
			plugin->Host.AudioOut[port] = (float *)hostData;
			return;
		}
		port -= LV2_NUM_AUDIO_OUTS;

		if (port < LV2_NUM_INS)
		{
			plugin->Host.In[port] = (float *)hostData;
			return;
		}
		port -= LV2_NUM_INS;

		if (port < LV2_NUM_OUTS)
		{
			plugin->Host.Out[port] = (float *)hostData;
			return;
		}
		port -= LV2_NUM_OUTS;
	}
}





/********************* render() **********************
 */
static LV2_Inline_Display_Image_Surface *
render (LV2_Handle instance, uint32_t w, uint32_t max_h)
{
	MolotMonoLite *	plugin;
	LV2_Inline_Display_Image_Surface *	surface;
	cairo_surface_t *	cs;

	plugin = static_cast<MolotMonoLite *> (instance);
	surface = &plugin->Host.DisplaySurface;
	cs = plugin->Host.CairoSurface;

	uint32_t h = std::max (11u, std::min (1u | (uint32_t)std::ceil (w / 10.), max_h));

	if (!cs ||
		(uint32_t)cairo_image_surface_get_width(cs) != w ||
		(uint32_t)cairo_image_surface_get_height(cs) != h)
	{
		if (cs)
			cairo_surface_destroy(cs);
		cs = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
		plugin->Host.CairoSurface = cs;
		if (plugin->Host.CairoPattern)
		{
			cairo_pattern_destroy (plugin->Host.CairoPattern);
			plugin->Host.CairoPattern = nullptr;
		}
	}

	double xmin = std::floor (w * 0.05);
	double xmax = std::ceil (w * 0.95);
	double ymin = 2.0;
	double ymax = h - 2.0;
	double vmin = -40.0;
	double vmax = 0.0;
	auto v2p = [vmin, vmax](double v) -> double
	{
		return (v - vmin) / (vmax - vmin);
	};
	auto v2x = [xmin, xmax, v2p](double v) -> double
	{
		return xmin + v2p(v) * (xmax - xmin);
	};

	if (!plugin->Host.CairoPattern)
	{
		cairo_pattern_t* pat = cairo_pattern_create_linear (0.0, 0.0, w, 0);

		cairo_pattern_add_color_stop_rgba (pat, 1.0, .7, .7, .0, 0);
		cairo_pattern_add_color_stop_rgba (pat, 0.9 * v2p (-0.0), .7, .7, .0, 1);
		cairo_pattern_add_color_stop_rgba (pat, 0.9 * v2p (-10.0), .7, .7, .0, 1);
		cairo_pattern_add_color_stop_rgba (pat, 0.9 * v2p (-40.0), .9, .0, .0, 1);
		cairo_pattern_add_color_stop_rgba (pat, 0.0, .9, .0, .0, 0);

		plugin->Host.CairoPattern = pat;
	}

	cairo_t*	cr = cairo_create (cs);
	cairo_rectangle (cr, 0, 0, w, h);
	cairo_set_source_rgba (cr, .2, .2, .2, 1.0);
	cairo_fill (cr);

	cairo_set_line_width (cr, 1);
	cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 1.0);
	cairo_new_path (cr);
	for (double value : {0.0, -10.0, -20.0, -30.0, -40.0})
	{
		double x = std::round (v2x (value)) - 0.5;
		cairo_move_to (cr, x, 0);
		cairo_line_to (cr, x, h);
	}
	cairo_stroke (cr);

	cairo_rectangle (cr, xmin, ymin, xmax - xmin, ymax - ymin);
	cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 0.6);
	cairo_fill (cr);

	const int channels = plugin->Host.DisplayChannels;
	cairo_set_source (cr, plugin->Host.CairoPattern);
	for (int chan = 0; chan < channels; ++chan)
	{
		double rh = (ymax - ymin) / channels;
		double ys = (channels > 1) ? 0.1 * rh : 0.0;
		double x1 = v2x (plugin->Host.DisplayLevel[chan]);
		double x2 = v2x (xmax);
		cairo_rectangle (cr, x1, ymin + chan * rh + ys, x2 - x1, rh - 2 * ys);
		cairo_fill (cr);
	}

	cairo_destroy (cr);
	cairo_surface_flush (cs);
	surface->width  = cairo_image_surface_get_width (cs);
	surface->height = cairo_image_surface_get_height (cs);
	surface->stride = cairo_image_surface_get_stride (cs);
	surface->data   = cairo_image_surface_get_data (cs);
	return surface;
}





/********************* extensionData() **********************
 */
static const void * extensionData(const char * uri)
{
	if (!strcmp (uri, LV2_INLINEDISPLAY__interface))
	{
		static const LV2_Inline_Display_Interface display = { &render };
		return &display;
	}
	return nullptr;
}





/********************* lv2_descriptor() **********************
 * Returns a pointer to our LV2_Descriptor struct at the
 * requested index.
 *
 * index =	The plugin index.
 *
 * NOTE: "index" ranges from 0 to how many plugins we support
 * (minus 1). For this example, we have 1 plugin. We'll assign an
 * index of 0 for it. Any other index values, we return a 0 to
 * indicate we have no further plugin types.
 *
 * This function must be named "lv2_descriptor", and can't be
 * static. Our plugin must contain this function.
 */

static const LV2_Descriptor PluginDescriptor = {
	UriStr,
	instantiate,
	connectPort,
	activate,
	run,
	deActivate,
	cleanup,
	&extensionData};

LV2_SYMBOL_EXPORT const LV2_Descriptor * lv2_descriptor(uint32_t index)
{
	if (!index) return &PluginDescriptor;
	return 0;
}
