#include "plugin.h"
#define MOLOT_LITE_STEREO 1
#include "moloteng.h"
#include "Oversampler.h"

class MolotStereoLite
{
public:
	MolotStereoLite(double);
	~MolotStereoLite();
	void pluginRun();
	void pluginActivate();
	void pluginDeActivate();
	
	MOLOT_STEREO_LITE	Host;

private:
	StereoCompressor	m_comp;
	double				Value[LV2_NUM_INS];
	iplug::OverSampler<float>* m_oversampler;

	void UpdateParameters();
};
