#include "Molot_Mono_Lite.h"


void MolotMonoLite::UpdateParameters()
{
	register double			val;
	register unsigned char	flag;

	flag = 0;
	if ((val = (double)*Host.In[LV2_FILTER]) != Value[LV2_FILTER])
	{
		Value[LV2_FILTER] = val;
		flag = 1;
	}
	if ((val = (double)*Host.In[LV2_ATTACK]) != Value[LV2_ATTACK])
	{
		Value[LV2_ATTACK] = val;
		flag = 1;
	}
	if ((val = (double)*Host.In[LV2_ATK_MODE]) != Value[LV2_ATK_MODE])
	{
		Value[LV2_ATK_MODE] = val;
		flag = 1;
	}
	if ((val = (double)*Host.In[LV2_RELEASE]) != Value[LV2_RELEASE])
	{
		Value[LV2_RELEASE] = val;
		flag = 1;
	}

	if (flag)
		m_comp.setEnvelope((double)Host.SampleRate,
											Value[LV2_ATTACK] / 1000.0,
											Value[LV2_RELEASE] / 1000.0,
											Value[LV2_FILTER] == 40.0 ? 0.0 : Value[LV2_FILTER],
											Value[LV2_ATK_MODE]);  // sharp mode

	flag = 0;
	if ((val = (double)*Host.In[LV2_THRESHOLD]) != Value[LV2_THRESHOLD])
	{
		Value[LV2_THRESHOLD] = val;
		flag = 1;
	}
	if ((val = (double)*Host.In[LV2_KNEE]) != Value[LV2_KNEE])
	{
		Value[LV2_KNEE] = val;
		flag = 1;
	}
	if ((val = (double)*Host.In[LV2_RATIO]) != Value[LV2_RATIO])
	{
		Value[LV2_RATIO] = val;
		flag = 1;
	}
	if (flag)
		m_comp.setEnvelopeK(DB_TO_K(Value[LV2_THRESHOLD]), Value[LV2_KNEE], Value[LV2_RATIO] == 11.0 ? 100 : Value[LV2_RATIO]);

	flag = 0;
	if ((val = (double)*Host.In[LV2_MAKEUP]) != Value[LV2_MAKEUP])
	{
		Value[LV2_MAKEUP] = val;
		flag = 1;
	}
	if ((val = (double)*Host.In[LV2_DRY_WET]) != Value[LV2_DRY_WET])
	{
		Value[LV2_DRY_WET] = val;
		flag = 1;
	}
	if ((val = (double)*Host.In[LV2_INPUT]) != Value[LV2_INPUT])
	{
		Value[LV2_INPUT] = val;
		flag = 1;
	}
	if (flag)
		m_comp.setGain(DB_TO_K(Value[LV2_MAKEUP]), Value[LV2_DRY_WET] / 100.0, DB_TO_K(Value[LV2_INPUT]));
}





/************************** pluginRun() *************************
 * Called by host to add DSP to the waveform data in the
 * input buffer(s), and stores the new data in the output
 * buffer(s).
 *
 * Host.AudioIn[]
 *		An array of pointers to the channels of input (source)
 *		waveform data.
 *
 * Host.AudioOut[]
 *		An array of pointers where to store the channels of output
 *		waveform data.
 *
 * Host.Begin
 *		An index where the data starts in AudioIn[].
 *
 * Host.End
 *		An index where the data stops in AudioIn[].
 *
 * User Input (In[] array)
 *		Input = *Host.In[LV2_INPUT]
 *		Filter = *Host.In[LV2_FILTER]
 *		Threshold = *Host.In[LV2_THRESHOLD]
 *		Knee = *Host.In[LV2_KNEE]
 *		Attack = *Host.In[LV2_ATTACK]
 *		Atk Mode = *Host.In[LV2_ATK_MODE]
 *		Release = *Host.In[LV2_RELEASE]
 *		Ratio = *Host.In[LV2_RATIO]
 *		Makeup = *Host.In[LV2_MAKEUP]
 *		Dry / Wet = *Host.In[LV2_DRY_WET]

 * NOTES: AudioIn[] and AudioOut[] can both point to the same buffer
 * if desired. This means the original waveform data is modified.
 */

void MolotMonoLite::pluginRun()
{
	UpdateParameters();

	while (Host.Begin < Host.End)
	{
		*(Host.AudioOut[0] + Host.Begin) = (float)m_comp.processSample((double)*(Host.AudioIn[0] + Host.Begin));
		++Host.Begin;
	}
}





/********************* Constructor *********************
 * Called once when your plugin is loaded. This is
 * your first function called by the host.
 *
 * Here you allocate any initial resources you need, and
 * store them in variables you add to your MolotMonoLite
 * class (defined in plugin.h).
 *
 * At this point, the MOLOT_MONO_LITE hasn't yet been filled
 * in with info and data (pointers).
 */

MolotMonoLite::MolotMonoLite(double sampleRate)
{
}





/********************* Destructor *********************
 */

MolotMonoLite::~MolotMonoLite(void)
{
}





/********************* pluginActivate() **********************
 * Called by host to reset the state of a plugin, once before
 * our host starts making multiple calls to our run function.
 */

void MolotMonoLite::pluginActivate()
{
	m_comp.reset();
	memset(Value, 0, sizeof(Value));
}





/******************** pluginDeActivate() *********************
 * Called by host once after done making multiple calls to our
 * pluginRun function.
 */

void MolotMonoLite::pluginDeActivate()
{
	pluginActivate();
}
