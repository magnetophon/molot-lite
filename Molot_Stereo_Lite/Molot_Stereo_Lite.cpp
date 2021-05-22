#include "Molot_Stereo_Lite.h"


void MolotStereoLite::UpdateParameters()
{
	double			val;
	unsigned char	flag;

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
	if ((val = (int)*Host.In[LV2_OVERSAMPLING]) != (int)Value[LV2_OVERSAMPLING])
	{
		Value[LV2_OVERSAMPLING] = val;
		flag = 1;
	}

	if (flag)
	{
		m_comp.setEnvelope((double)Host.SampleRate * (1 << (int)Value[LV2_OVERSAMPLING]),
											Value[LV2_ATTACK] / 1000.0,
											Value[LV2_RELEASE] / 1000.0,
											Value[LV2_FILTER] == 40.0 ? 0.0 : Value[LV2_FILTER],
											Value[LV2_ATK_MODE]);  // sharp mode
		m_oversampler->SetOverSampling((iplug::EFactor)(int)Value[LV2_OVERSAMPLING]);
	}

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

	m_comp.setStereoMode((StereoCompressor::stereo_mode_t)*Host.In[LV2_STEREO_MODE]);
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
 *		Stereo mode = *Host.In[LV2_STEREO_MODE]

 * NOTES: AudioIn[] and AudioOut[] can both point to the same buffer
 * if desired. This means the original waveform data is modified.
 */
void MolotStereoLite::pluginRun()
{
	UpdateParameters();

	float* AudioIn[2] =
	{
		Host.AudioIn[0] + Host.Begin,
		Host.AudioIn[1] + Host.Begin,
	};
	float* AudioOut[2] =
	{
		Host.AudioOut[0] + Host.Begin,
		Host.AudioOut[1] + Host.Begin,
	};

	uint32_t TotalCount = Host.End - Host.Begin;

	while (TotalCount > 0)
	{
		uint32_t Count = (TotalCount < 512) ? TotalCount : 512;

		m_oversampler->ProcessBlock(AudioIn, AudioOut, Count, 2, 2,
			[this](float** OsIn, float** OsOut, uint32_t OsCount)
			{
				for (uint32_t i = 0; i < OsCount; ++i)
				{
					double	y1, y2;
					m_comp.processSample((double)OsIn[0][i], (double)OsIn[1][i], &y1, &y2);
					OsOut[0][i] = (float)y1;
					OsOut[1][i] = (float)y2;
				}
			});

		AudioIn[0] += Count;
		AudioIn[1] += Count;
		AudioOut[0] += Count;
		AudioOut[1] += Count;
		TotalCount -= Count;
	}
}





/********************* Constructor *********************
 * Called once when your plugin is loaded. This is
 * your first function called by the host.
 *
 * Here you allocate any initial resources you need, and
 * store them in variables you add to your MolotStereoLite
 * class (defined in plugin.h).
 *
 * At this point, the MOLOT_STEREO_LITE hasn't yet been filled in with info and
 * data (pointers).
 */

MolotStereoLite::MolotStereoLite(double sampleRate)
{
	m_oversampler = new iplug::OverSampler<float>(iplug::kNone, 2, 2);
}





/********************* Destructor *********************
 */

MolotStereoLite::~MolotStereoLite(void)
{
	delete m_oversampler;
}





/********************* pluginActivate() **********************
 * Called by host to reset the state of a plugin, once before
 * our host starts making multiple calls to our run function.
 */

void MolotStereoLite::pluginActivate()
{
	m_comp.reset();
	memset(Value, 0, sizeof(Value));
}





/******************** pluginDeActivate() *********************
 * Called by host once after done making multiple calls to our
 * pluginRun function.
 */

void MolotStereoLite::pluginDeActivate()
{
	pluginActivate();
}

