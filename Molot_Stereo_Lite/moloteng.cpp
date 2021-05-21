/*
 *  Molot compressor engine implementation.
 *  Copyright (C) 2010-2012  Vladislav Goncharov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include "Molot_Stereo_Lite.h"

#define SIDECHAIN_SHELF_FREQ		296.0
#define SIDECHAIN_SHELF_CUT		(-6.5 + 3.48)
#define SIDECHAIN_SHELF_Q			0.7

//===================================================
// Filter2
//===================================================

Filter2::Filter2(double a0, double a1, double a2, double b1, double b2)
{
	m_lp_a[0] = a0;
	m_lp_a[1] = a1;
	m_lp_a[2] = a2;

	m_lp_b[0] = 1.0;
	m_lp_b[1] = -b1;
	m_lp_b[2] = -b2;

	// reset filter state
	memset(m_xv, 0, sizeof(m_xv));
	memset(m_yv, 0, sizeof(m_yv));
}

Filter2::Filter2()
{
	memset(m_lp_a, 0, sizeof(m_lp_a));
	memset(m_lp_b, 0, sizeof(m_lp_b));

	// reset filter state
	memset(m_xv, 0, sizeof(m_xv));
	memset(m_yv, 0, sizeof(m_yv));
}

Filter2& Filter2::operator=(const Filter2& obj)
{
	memcpy(m_lp_a, obj.m_lp_a, sizeof(m_lp_a));
	memcpy(m_lp_b, obj.m_lp_b, sizeof(m_lp_b));

	return *this;
}

double Filter2::processSample(double x)
{
	m_xv[2] = m_xv[1]; m_xv[1] = m_xv[0]; m_xv[0] = x;
	m_yv[2] = m_yv[1]; m_yv[1] = m_yv[0];

	m_yv[0] = m_xv[0] * m_lp_a[0] +
		m_xv[1] * m_lp_a[1] + m_xv[2] * m_lp_a[2] +
		m_yv[1] * m_lp_b[1] + m_yv[2] * m_lp_b[2];

	return m_yv[0];
}

void Filter2::reset(bool reset_to_1)
{
	if (!reset_to_1)
	{
		memset(m_xv, 0, sizeof(m_xv));
		memset(m_yv, 0, sizeof(m_yv));
	}
	else for (int i = 0; i < 2; i++)
	{
		m_xv[i] = m_yv[i] = 1.0;
	}
}






//===================================================
// Filter4
//===================================================


Filter4::Filter4(const Filter2 &flt0, const Filter2 &flt1)
{
	m_flt0 = flt0;
	m_flt1 = flt1;
}

Filter4& Filter4::operator=(const Filter4 &obj)
{
	m_flt0 = obj.m_flt0;
	m_flt1 = obj.m_flt1;

	return *this;
}

double Filter4::processSample(double x)
{
	double xx;

	xx = m_flt0.processSample(x);
	xx = m_flt1.processSample(xx);

	return xx;
}

void Filter4::reset()
{
	m_flt0.reset();
	m_flt1.reset();
}






//===================================================
// LPFilter2
//===================================================

LPFilter2::LPFilter2(double freq_k, double Q)
{
	double alpha, omega0;

	assert(freq_k < 0.5);

	/*
	 * Cookbook formulae for audio EQ biquad filter coefficients
	 * http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
	 */

	omega0 = freq_k * 2.0 * M_PI;
	alpha = sin(omega0) / (2.0 * Q);

	m_lp_a[0] = ((1.0 - cos(omega0)) / 2.0) / (1.0 + alpha);
	m_lp_a[1] = (1.0 - cos(omega0)) / (1.0 + alpha);
	m_lp_a[2] = ((1.0 - cos(omega0)) / 2.0) / (1.0 + alpha);

	m_lp_b[0] = 1.0;
	m_lp_b[1] = -(-2.0 * cos(omega0)) / (1.0 + alpha);
	m_lp_b[2] = -(1.0 - alpha) / (1.0 + alpha);
}








//===================================================
// HPFilter2
//===================================================

HPFilter2::HPFilter2(double freq_k, double Q)
{
	double alpha, omega0;

	assert(freq_k < 0.5);

	/*
	 * Cookbook formulae for audio EQ biquad filter coefficients
	 * http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
	 */

	omega0 = freq_k * 2.0 * M_PI;
	alpha = sin(omega0) / (2.0 * Q);

	m_lp_a[0] = ((1.0 + cos(omega0)) / 2.0) / (1.0 + alpha);
	m_lp_a[1] = -(1.0 + cos(omega0)) / (1.0 + alpha);
	m_lp_a[2] = ((1.0 + cos(omega0)) / 2.0) / (1.0 + alpha);

	m_lp_b[0] = 1.0;
	m_lp_b[1] = -(-2.0 * cos(omega0)) / (1.0 + alpha);
	m_lp_b[2] = -(1.0 - alpha) / (1.0 + alpha);
}






//===================================================
// LSFilter2
//===================================================

LSFilter2::LSFilter2(double freq_k, double S, double A)
{
	double alpha, omega0;

	assert(freq_k < 0.5);

	/*
	 * Cookbook formulae for audio EQ biquad filter coefficients
	 * http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
	 */

	A = sqrt(A);        // 10^(Adb / 40) e.g. Adb / 2.

	omega0 = freq_k * 2.0 * M_PI;
	alpha = sin(omega0) / 2.0 * sqrt((A + 1.0 / A) * (1.0 / S - 1.0) + 2.0);

	m_lp_a[0] = A * ((A + 1.0) - (A - 1.0) * cos(omega0) + 2.0 * sqrt(A) * alpha);
	m_lp_a[1] = 2.0 * A * ((A - 1.0) - (A + 1.0) * cos(omega0));
	m_lp_a[2] = A * ((A + 1.0) - (A - 1.0) * cos(omega0) - 2.0 * sqrt(A) * alpha);

	m_lp_b[0] = (A + 1.0) + (A - 1.0) * cos(omega0) + 2.0 * sqrt(A) * alpha;
	m_lp_b[1] = -2.0 * ((A - 1.0) + (A + 1.0) * cos(omega0));
	m_lp_b[2] = (A + 1.0) + (A - 1.0) * cos(omega0) - 2 * sqrt(A) * alpha;

	m_lp_a[0] /= m_lp_b[0];
	m_lp_a[1] /= m_lp_b[0];
	m_lp_a[2] /= m_lp_b[0];

	m_lp_b[1] /= -m_lp_b[0];
	m_lp_b[2] /= -m_lp_b[0];

	m_lp_b[0] = 1.0;
}

double LSFilter2::S_from_Q(double Q, double A)
{
	/*
	 * 1/Q = sqrt((A + 1/A)*(1/S - 1) + 2)
	 * http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
	 */

	return -(A * A + 1.0) / (-A * A - 1.0 + 2.0 * A - 1.0 / (Q * Q) * A);
}






//===================================================
// LowShelfPhon80
//===================================================

LowShelfPhon80::LowShelfPhon80(double freq_hz, double sample_rate)
:   Filter4(    // 35.15 Hz; Q=0.199; filter calculated for 8 points (20..90 Hz) from phon80 ISO226:2003
        HPFilter2(35.15 / sample_rate, get_Q_from_hz(freq_hz)),
        HPFilter2(35.15 / sample_rate, get_Q_from_hz(freq_hz)))
{
}

// for Q=0.199 got -3db at 262 hZ
double LowShelfPhon80::get_Q_from_hz(double x)
{
	static const double min_freq_hz = 23.756;

	double Q;

	if (x < min_freq_hz)
	{
		assert(0);
		x = min_freq_hz;
	}

	// solved H(x / 35.15, Q)^2 = -3 db equation

	Q = 236410735106.01834177 /  sqrt(1.87373344546477390990e19 * (x * x * x * x) +
		1.1178007134673594641e23 * (x * x) -
		6.9053396600248781681e25) * x;

	return Q;
}







//===================================================
// AREnvelope
//===================================================

AREnvelope::AREnvelope(double attack_freq_k, double release_freq_k, double signal_freq_k, bool sharp_mode)
:   m_fabs_lpf(signal_freq_k),   // try to remove fabs aliasing
    m_filter(attack_freq_k, release_freq_k, signal_freq_k, sharp_mode)
{
}


AREnvelope& AREnvelope::operator=(const AREnvelope &obj)
{
	m_filter = obj.m_filter;

	return *this;
}

double AREnvelope::sec_to_freq_k(double sec, double sample_rate)
{
	double freq;

	if (sec > 0.0)
		freq = 1.0 / sec;
	else
		freq = sample_rate / (2.0 * M_PI);

	return freq / sample_rate;
}

double AREnvelope::tau_to_freq_k(double sec, double sample_rate)
{
	return sec_to_freq_k(sec, sample_rate) / (2.0 * M_PI);
}

double AREnvelope::processSampleAC(double x)
{
	double y1, y2, y3;

	y1 = fabs(x);                       // get x: 0..1
	y2 = m_fabs_lpf.processSample(y1);  // try to remove fabs() aliasing
	y3 = m_filter.processSample(y2);    // follow it

	return y3;
}

double AREnvelope::processSampleDC(double x)
{
	return m_filter.processSample(x);   // follow it
}

void AREnvelope::reset(bool reset_to_1)
{
	m_fabs_lpf.reset();
	m_filter.reset(reset_to_1);
}

#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))

AREnvelope::ARFilter::ARFilter(double attack_freq_k, double release_freq_k, double signal_freq_k, bool sharp_mode)
:   m_diode_lpf(!sharp_mode ?
                MAX(attack_freq_k, release_freq_k) :    // try to remove diode() aliasing but don't affect attack/release (got S-curve)
                signal_freq_k)    
{
	fa = 1.0 / (1.0 / (attack_freq_k * 2.0 * M_PI) + 1.0);
	fr = 1.0 / (1.0 / (release_freq_k * 2.0 * M_PI) + 1.0);

	m_s = 0;

	// diode stuff
	m_diode_xx = 0.025;

	m_diode_ax[7] = -3432.0 / pow(m_diode_xx, 15.0);
	m_diode_ax[6] = 25740.0 / pow(m_diode_xx, 14.0);
	m_diode_ax[5] = -83160.0 / pow(m_diode_xx, 13.0);
	m_diode_ax[4] = 150150 / pow(m_diode_xx, 12.0);
	m_diode_ax[3] = -163800.0 / pow(m_diode_xx, 11.0);
	m_diode_ax[2] = 108108.0 / pow(m_diode_xx, 10.0);
	m_diode_ax[1] = -40040.0 / pow(m_diode_xx, 9.0);
	m_diode_ax[0] = 6435.0 / pow(m_diode_xx, 8.0);
}

AREnvelope::ARFilter& AREnvelope::ARFilter::operator=(const AREnvelope::ARFilter &obj)
{
	m_diode_lpf = obj.m_diode_lpf;

	fa = obj.fa;
	fr = obj.fr;

	m_diode_xx = obj.m_diode_xx;
	memcpy(m_diode_ax, obj.m_diode_ax, sizeof(m_diode_ax));

	return *this;
}

double AREnvelope::ARFilter::processSample(double x)
{
	double y;

	y = ( m_s += fa * ( diode(x - m_s) ) + fr * (x - m_s) );

	// alternative way: use second diode for release fr * ( -diode(m_s - x) ) but don't needed

	y = m_diode_lpf.processSample(y);    // remove diode aliasing (VERY USEFUL!)

	return y;
}

double AREnvelope::ARFilter::diode(double x)
{
	/*
	* this approximation sounds very good
	*/
	double x2, x4, x8, x12, y;

	if (x < 0.0)
		return 0.0;
	if (x > m_diode_xx)
		return x;

	// spline 2 curves
	x2 = x * x;
	x4 = x2 * x2;
	x8 = x4 * x4;
	x12 = x8 * x4;

	// m_diode_ax[0] for x^9
	y = x8 * x * m_diode_ax[0] + x8 * x2 * m_diode_ax[1] + x8 * x2 * x * m_diode_ax[2] +
		x12 * m_diode_ax[3] + x12 * x * m_diode_ax[4] + x12 * x2 * m_diode_ax[5] +
		x12 * x2 * x * m_diode_ax[6] + x12 * x4 * m_diode_ax[7];

	return y;
}

void AREnvelope::ARFilter::reset(bool reset_to_1)
{
	m_diode_lpf.reset(reset_to_1);
	m_s = 0;
}






//===================================================
// EnvelopeK
//===================================================

EnvelopeClip::EnvelopeClip(double thresh, double knee, double ratio)
{
	double delta, x2, y2, dy2;

	assert(thresh > 0.0 && thresh <= 1.0);
	assert(knee >= 0.0 && knee <= 1.0);
	assert(ratio > 1.0);

	m_thresh = thresh;
	m_hx_min = 1e-6;    // GR(m_hx_min) = 1

	m_clip = false;

	// calculate h2 info (compression part)
	if (ratio <= 10.9)
	{
		// logarithmic ratio (only 1.1..10.9 supported)
		ratio = floor(ratio * 10.0) / 10.0;
		m_ratio_index = (int)(ratio * 10.0 - 11.0);

		assert(m_ratio_index >= 0.0 && m_ratio_index < 99);

		m_ratio = 1.0 / ratio;

		if (m_ratio_index < 0) m_ratio_index = 0;
		if (m_ratio_index >= 99) m_ratio_index = 98;

		m_thresh_ratio = pow(1.0 / m_thresh, m_ratio) * m_thresh;

	}
	else
	{
		// limiter mode
		m_ratio = 0;
	}

	delta = (1.0 - knee * 0.95) * m_thresh;    // m_thresh - soft knee

	m_thresh2 = m_thresh + delta;

	// calculate h1 (soft knee part)
	x2 = m_thresh2;
	y2 = h2(x2);
	dy2 = dh2(y2);

	// calculate m_A[]  (m_A[0] = 0, m_A[1] = 1, m_A[2] = 0)
	m_A3 = -(3.0 * x2 - 4.0 * y2 + x2 * dy2) / (x2 * x2 * x2);
	m_A4 = 1.0 / (x2 * x2 * x2 * x2) * (2.0 * x2 - 3.0 * y2 + x2 * dy2);
}

EnvelopeClip& EnvelopeClip::operator=(const EnvelopeClip &obj)
{
	m_thresh = obj.m_thresh;
	m_ratio = obj.m_ratio;
	m_thresh_ratio = obj.m_thresh_ratio;
	m_ratio_index = obj.m_ratio_index;

	m_hx_min = obj.m_hx_min;
	m_thresh2 = obj.m_thresh2;

	m_A3 = obj.m_A3;
	m_A4 = obj.m_A4;

	return *this;
}

double EnvelopeClip::processSample(double x)
{
	// check bypass
	if (m_ratio == 1.0) return 1.0;

	// we don't try to remove getGR() aliasing because the next A/R envelope call will remove unneccessary harmonics
	return getGR(x);
}

// transfer function
double EnvelopeClip::h(double x)
{
	double y;

	if (x < m_thresh2)
		y = h1(x);
	else
		y = h2(x);

	return y;
}

// knee part of transfer function
double EnvelopeClip::h1(double x)
{
	double x2, x3, x4;

	x2 = x * x;
	x3 = x2 * x;
	x4 = x2 * x2;

	return x + x3 * m_A3 + x4 * m_A4;
}

double   EnvelopeClip::m_sqrt_k[99][10] = {
    // 8th order polynomial x^(1.1..10.9) interpolation and max x
# include "sqrt.dat"
};

// compression part of transfer function
double EnvelopeClip::h2(double x)
{
	double y, pow_x_ratio, x2, x4;

	if (m_ratio > 0.0)
	{
		// anialiased: y = pow(x / m_thresh, m_ratio) * m_thresh;

		if (x > m_sqrt_k[m_ratio_index][9])
		{
			x = m_sqrt_k[m_ratio_index][9];

			m_clip = true;
		}

		// x ^ m_ratio

		x2 = x * x;
		x4 = x2 * x2;

		pow_x_ratio = m_sqrt_k[m_ratio_index][0] +
			x * m_sqrt_k[m_ratio_index][1] + x2 * m_sqrt_k[m_ratio_index][2] + x2 * x * m_sqrt_k[m_ratio_index][3] +
			x4 * m_sqrt_k[m_ratio_index][4] + x4 * x * m_sqrt_k[m_ratio_index][5] + x4 * x2 * m_sqrt_k[m_ratio_index][6] +
			x4 * x2 * x * m_sqrt_k[m_ratio_index][7] + x4 * x4 * m_sqrt_k[m_ratio_index][8];

		y = pow_x_ratio * m_thresh_ratio;
	}
	else
	{
		// hardlimit compression part
		y = m_thresh;
	}

	assert(y >= 0.0);

	return y;
}

// dh2(x)/dx  (used for h1 smoothing)
double EnvelopeClip::dh2(double x)
{
	double y;

	if (m_ratio > 0.0)
	{
		// logarithmic compression part
		if (x > 0.0) 
		{
			// y = pow(x / m_thresh, m_ratio) * m_thresh * m_ratio / x;
			y = h2(x) * m_ratio / x;
		}
		else
			y = 0.0;
	}
	else
	{
		// hardlimit compression part
		y = 0.0;
	}

	return y;
}

double EnvelopeClip::getGR(double x)
{
    double y;

    if (x >= m_hx_min)
        y = h(x) / x;
    else
        y = 1.0;

    return y;
}

bool EnvelopeClip::getClip(bool reset)
{
    bool rc = m_clip;

    if (reset)
        m_clip = false;

    return rc;
}







//===================================================
// ChannelCompressor
//===================================================

ChannelCompressor::ChannelCompressor()
:   m_sidechain_flt(100.0, 44100),
    m_sidechain_flt2(
        SIDECHAIN_SHELF_FREQ / 44100,
        LSFilter2::S_from_Q(SIDECHAIN_SHELF_Q, SIDECHAIN_SHELF_CUT),
        DB_TO_K(SIDECHAIN_SHELF_CUT)),
    m_env(
        1000.0 / 44100,
        100.0 / 44100,
        0.49,
        false),
    m_env_ar(
        AREnvelope::sec_to_freq_k(0.01, 44100),
        AREnvelope::sec_to_freq_k(0.05, 44100),
        0.49,
        false),
    m_envelope_k(1.0, 1.0, 2.0)
{
    m_detector_fix = 1.0;
    m_use_sidechain_flt = true;

    m_max_zero_samples = 44100;

    m_zero_samples = 0;
    m_is_on = false;    // auto off

    m_gain = 1.0;
    m_dry_mix_k = 0.0;
    m_in_gain = 1.0;

    // set it default to 1
    m_env_ar.reset(true);
}

bool ChannelCompressor::preprocessSample(double x, State *state)
{
	double	x_sc;

	// check auto on/off condition
	if (!m_is_on && x != 0.0)
		m_is_on = true;         // auto_on

	if (!m_is_on)
	{
		// true bypass
pass:	memset(state, 0, sizeof(*state));
		return false;
	}

	if (x == 0.0)
	{
		m_zero_samples++;
		if (m_zero_samples >= m_max_zero_samples)
		{
			m_is_on = false;    // auto off
			goto pass;
		}
	}
	else
		m_zero_samples = 0;

	// ==================================
	// start processing
	// ==================================

	state->over = x * m_in_gain;

	// generate envelope

	if (m_use_sidechain_flt)

		// use filter#1 (user-defined high-pass)
		x_sc = m_sidechain_flt.processSample(state->over);

	else
		// use filter#2 (hardcoded shelf)
		x_sc = m_sidechain_flt2.processSample(state->over);

	state->env = m_env.processSampleAC(x_sc) * m_detector_fix;

	return true;
}

double ChannelCompressor::postprocessSample(State *state)
{
	double	x_dry, env_gr, env_ar, x_wet, y;

	x_dry = state->over;

	// generate GR envelope by threshold & ratio
	env_gr = m_envelope_k.processSample(state->env);

	// apply attack/release to env_gr
	// reverse because GR curve is reverse: rotate around 1.0
	env_ar = 2.0 - m_env_ar.processSampleDC(2.0 - env_gr);

	// apply gain reduction to dry input
	x_wet = x_dry * env_ar;

	// gain & dry mix
	y = x_wet * m_gain + x_dry * m_dry_mix_k;

	return y;
}

double ChannelCompressor::processSample(double x)
{
	State		state;

	return !preprocessSample(x, &state) ? x : postprocessSample(&state);
}

bool ChannelCompressor::getClip(bool reset)
{
    // clipping can occur envelope shaper
    return m_envelope_k.getClip(reset);
}

void ChannelCompressor::reset()
{
	m_env.reset();
	m_sidechain_flt.reset();
	m_sidechain_flt2.reset();
	m_env_ar.reset(true);
}

















#ifdef MOLOT_LITE_STEREO

// ============================================================
// StereoCompressor
// ============================================================

StereoCompressor::StereoCompressor()
{
    m_stereo_mode = SL_STEREO;
}

StereoCompressor::~StereoCompressor()
{
}

void StereoCompressor::setEnvelope(double sample_rate, double attack_s, double release_s, double sidechain_freq, bool sharp_mode)
{
	int chn;
	double env_attack, env_release;

	assert(attack_s >= 0.0001 && attack_s < 1.0);
	assert(release_s >= 0.005 && release_s < 10.0);

	for (chn = 0; chn < 2; chn++)
	{
		if (!sharp_mode)
		{
			if (attack_s <= 0.0095)
			{
				// use fast detector
				env_attack = 0.001;     // 1ms
				env_release = 0.050;    // 50ms

				m_comp[chn].m_detector_fix = DB_TO_K(0.547);    // calibrated by 1kHz

			}
			else if (attack_s <= 0.0245)
			{
				// use slow detector
				env_attack = 0.010;     // 10ms
				env_release = 0.050;    // 50ms

				m_comp[chn].m_detector_fix = DB_TO_K(1.536);    // calibrated by 1kHz

			}
			else
			{
				// use the slowest detector
				env_attack = 0.025;     // 25ms
				env_release = 0.050;    // 50ms

				m_comp[chn].m_detector_fix = DB_TO_K(2.237);    // calibrated by 1kHz

			}
		}
		else
		{
			// use the fastest detector
			env_attack = 0.00005;       // 0.1ms
			env_release = 0.001;        // 1ms

			m_comp[chn].m_detector_fix = DB_TO_K(0.878);        // calibrated by 1kHz
		}

		m_comp[chn].m_env = AREnvelope(
					AREnvelope::sec_to_freq_k(env_attack, sample_rate),
					AREnvelope::sec_to_freq_k(env_release, sample_rate),
					0.49,
					false);

		// sidechain highpass
		if (sidechain_freq > 0.0)
		{
			m_comp[chn].m_sidechain_flt = LowShelfPhon80(sidechain_freq, sample_rate);
			m_comp[chn].m_use_sidechain_flt = true;
		}
		else
		{
            // use hardcoded lowshelf
            m_comp[chn].m_sidechain_flt2 = LSFilter2(
                SIDECHAIN_SHELF_FREQ / sample_rate,
                LSFilter2::S_from_Q(SIDECHAIN_SHELF_Q, DB_TO_K(SIDECHAIN_SHELF_CUT)),
                DB_TO_K(SIDECHAIN_SHELF_CUT));

            m_comp[chn].m_use_sidechain_flt = false;
        }

        // setup A/R envelopes
        m_comp[chn].m_env_ar = AREnvelope(
            AREnvelope::sec_to_freq_k(attack_s, sample_rate),
            AREnvelope::sec_to_freq_k(release_s, sample_rate),
            0.49,
            sharp_mode);   // if (!sharp_mode) got S-curve; else exp-curve;

        // setup auto-off time
        m_comp[chn].m_max_zero_samples = (size_t)(2.0 * release_s * sample_rate);    // use 2*release
    }
}

void StereoCompressor::setEnvelopeK(double threshold, double knee, double ratio)
{
	int chn;

	assert(threshold >= 0.0 && threshold <= 1.0);
	assert(knee >= 0.0 && knee <= 1.0);

	for (chn = 0; chn < 2; chn++)
	{
		// setup gain clipper
		m_comp[chn].m_envelope_k = EnvelopeClip(threshold, knee, ratio);
	}
}

void StereoCompressor::setGain(double gain, double dry_mix, double in_gain)
{
	int chn;

	for (chn = 0; chn < 2; chn++)
	{
		m_comp[chn].m_gain = gain * (1.0 - dry_mix);
		m_comp[chn].m_dry_mix_k = dry_mix;
		m_comp[chn].m_in_gain = in_gain;
	}
}

void StereoCompressor::setStereoMode(stereo_mode_t sl)
{
	m_stereo_mode = sl;
}

void StereoCompressor::processSample(double x1, double x2, double *y1, double *y2)
{
	double							xm, xs, ym, ys, env;
	ChannelCompressor::State	state[2];
	bool								rc1, rc2;

	switch (m_stereo_mode)
	{
		case SL_2MONO:
			*y1 = m_comp[0].processSample(x1);
			*y2 = m_comp[1].processSample(x2);

			break;

		case SL_R_SCHN:
			// use R as sidechain signal
			rc1 = m_comp[0].preprocessSample(x2, &state[0]);

			if (rc1)
			{
				// save L in state
				state[0].over = x1 * m_comp[0].m_in_gain;

				// now process it
				*y1 = m_comp[0].postprocessSample(&state[0]);
				*y2 = *y1;

			}
			else
			{
				// true bypass
				*y1 = x1;
				*y2 = x1;
			}

			break;
    
		case SL_M:
			xm = (x1 + x2) / 2.0;
			xs = (x1 - x2) / 2.0;

			ym = m_comp[0].processSample(xm);
			ys = xs * m_comp[0].m_in_gain;

			*y1 = ym + ys;
			*y2 = ym - ys;
			break;
    
		case SL_S:
        xm = (x1 + x2) / 2.0;
        xs = (x1 - x2) / 2.0;

        ys = m_comp[0].processSample(xs);
        ym = xm * m_comp[0].m_in_gain;

        *y1 = ym + ys;
        *y2 = ym - ys;
        break;

		case SL_STEREO:
			rc1 = m_comp[0].preprocessSample(x1, &state[0]);
			rc2 = m_comp[1].preprocessSample(x2, &state[1]);

			if (rc1 || rc2)
			{
				env = (state[0].env + state[1].env) / 2.0;
				//env = MAX(state[0].env, state[1].env);

				state[0].env = env;
				state[1].env = env;

				*y1 = m_comp[0].postprocessSample(&state[0]);
				*y2 = m_comp[1].postprocessSample(&state[1]);
			}
			else
			{
				// true bypass
				*y1 = x1;
				*y2 = x2;
			}

			break;

		case SL_MS:
			xm = (x1 + x2) / 2.0;
			xs = (x1 - x2) / 2.0;

			rc1 = m_comp[0].preprocessSample(xm, &state[0]);
			rc2 = m_comp[1].preprocessSample(xs, &state[1]);

			if (rc1 || rc2)
			{
				env = MAX(state[0].env, state[1].env);

				state[0].env = env;
				state[1].env = env;

				ym = m_comp[0].postprocessSample(&state[0]);
				ys = m_comp[1].postprocessSample(&state[1]);
			}
			else
			{
				// true bypass
				ym = xm;
				ys = xs;
			}

			*y1 = ym + ys;
			*y2 = ym - ys;

//			break;
	}
}

#ifdef MOLOT_LITE_QUAD

void StereoCompressor::processSampleSC(double x1, double x2, double sc1, double sc2, double *y1, double *y2)
{
	double xm, xs, ym, ys, env, sc_m, sc_s;
	ChannelCompressor::State state[2];
	bool rc1, rc2;

	switch (m_stereo_mode)
	{
		case SL_STEREO:
			rc1 = m_comp[0].preprocessSample(sc1, &state[0]);
			rc2 = m_comp[1].preprocessSample(sc2, &state[1]);

			if (rc1 || rc2)
			{
				// make average envelope
				env = (state[0].env + state[1].env) / 2.0;

				state[0].env = env;
				state[1].env = env;

				// save x1 & x2 in state
				state[0].over = x1 * m_comp[0].m_in_gain;
				state[1].over = x2 * m_comp[1].m_in_gain;

				// now process it!
				*y1 = m_comp[0].postprocessSample(&state[0]);
				*y2 = m_comp[1].postprocessSample(&state[1]);
			}
			else
			{
				// true bypass
				*y1 = x1;
				*y2 = x2;
			}

			break;

		case SL_2MONO:
			// unlinked stereo
			rc1 = m_comp[0].preprocessSample(sc1, &state[0]);
			rc2 = m_comp[1].preprocessSample(sc2, &state[1]);

			if (rc1 || rc2)
			{
				// save x1 & x2 in state
				state[0].over = x1 * m_comp[0].m_in_gain;
				state[1].over = x2 * m_comp[1].m_in_gain;

				// now process it!
				*y1 = m_comp[0].postprocessSample(&state[0]);
				*y2 = m_comp[1].postprocessSample(&state[1]);
			}
			else
			{
				// true bypass
				*y1 = x1;
				*y2 = x1;
			}

			break;

		case SL_R_SCHN:
			// use sc2 as sidechain signal
			rc1 = m_comp[0].preprocessSample(sc2, &state[0]);

			if (rc1)
			{
				// save L in state
				state[0].over = x1 * m_comp[0].m_in_gain;

				// now process it
				*y1 = m_comp[0].postprocessSample(&state[0]);
				*y2 = *y1;
			}
			else
			{
				// true bypass
				*y1 = x1;
				*y2 = x1;
			}

			break;
    
		case SL_M:
			xm = (x1 + x2) / 2.0;
			xs = (x1 - x2) / 2.0;

			sc_m = (sc1 + sc2) / 2.0;

			rc1 = m_comp[0].preprocessSample(sc_m, &state[0]);

			if (rc1)
			{
				// save xm in state
				state[0].over = xm * m_comp[0].m_in_gain;

				// now process it
				ym = m_comp[0].postprocessSample(&state[0]);
				ys = xs * m_comp[1].m_in_gain;
			}
			else
			{
				// true bypass
				ym = xm;
				ys = xs;
			}

			*y1 = ym + ys;
			*y2 = ym - ys;
			break;
    
		case SL_S:
			xm = (x1 + x2) / 2.0;
			xs = (x1 - x2) / 2.0;

			sc_s = (sc1 - sc2) / 2.0;

			rc1 = m_comp[0].preprocessSample(sc_s, &state[0]);

			if (rc1)
			{
				// save xs in state
				state[0].over = xs * m_comp[0].m_in_gain;

				// now process it
				ys = m_comp[0].postprocessSample(&state[0]);
				ym = xm * m_comp[1].m_in_gain;
			}
			else
			{
				// true bypass
				ym = xm;
				ys = xs;
			}

			*y1 = ym + ys;
			*y2 = ym - ys;
			break;

		case SL_MS:
			xm = (x1 + x2) / 2.0;
			xs = (x1 - x2) / 2.0;

			sc_m = (x1 + x2) / 2.0;
			sc_s = (x1 - x2) / 2.0;

			rc1 = m_comp[0].preprocessSample(sc_m, &state[0]);
			rc2 = m_comp[1].preprocessSample(sc_s, &state[1]);

			if (rc1 || rc2)	
			{
				// make MAX envelope
				env = MAX(state[0].env, state[1].env);

				state[0].env = env;
				state[1].env = env;

				// save x1 & x2 in state
				state[0].over = xm * m_comp[0].m_in_gain;
				state[1].over = xs * m_comp[1].m_in_gain;

				// now process it!
				ym = m_comp[0].postprocessSample(&state[0]);
				ys = m_comp[1].postprocessSample(&state[1]);
			}
			else
			{
				// true bypass
				ym = xm;
				ys = xs;
			}

			*y1 = ym + ys;
			*y2 = ym - ys;

//			break;
	}
}

#endif

void StereoCompressor::reset()
{
	m_comp[0].reset();
	m_comp[1].reset();
}

bool StereoCompressor::isStereo() const
{
	return !(m_stereo_mode == SL_M || m_stereo_mode == SL_S);
}

bool StereoCompressor::getClip(bool reset)
{
	bool rc;

	rc = m_comp[0].getClip(reset);

	if (isStereo()) rc |= m_comp[1].getClip(reset);

	return rc;
}

#else

// ============================================================
// MonoCompressor
// ============================================================

MonoCompressor::MonoCompressor()
{
}

MonoCompressor::~MonoCompressor()
{
}

void MonoCompressor::setEnvelope(double sample_rate, double attack_s, double release_s, double sidechain_freq, bool sharp_mode)
{
	double env_attack, env_release;

	assert(attack_s >= 0.0001 && attack_s < 1.0);
	assert(release_s >= 0.005 && release_s < 10.0);

	if (!sharp_mode)
	{
		if (attack_s <= 0.0095)
		{
			// use fast detector
			env_attack = 0.001;     // 1ms
			env_release = 0.050;    // 50ms

			m_comp.m_detector_fix = DB_TO_K(0.547);    // calibrated by 1kHz

		}
		else if (attack_s <= 0.0245)
		{
			// use slow detector
			env_attack = 0.010;     // 10ms
			env_release = 0.050;    // 50ms

			m_comp.m_detector_fix = DB_TO_K(1.536);    // calibrated by 1kHz

		}
		else
		{
			// use the slowest detector
			env_attack = 0.025;     // 25ms
			env_release = 0.050;    // 50ms

			m_comp.m_detector_fix = DB_TO_K(2.237);    // calibrated by 1kHz
		}
	}
	else
	{
		// use the fastest detector
		env_attack = 0.00005;       // 0.1ms
		env_release = 0.001;        // 1ms

		m_comp.m_detector_fix = DB_TO_K(0.878);        // calibrated by 1kHz
	}

	m_comp.m_env = AREnvelope(
				AREnvelope::sec_to_freq_k(env_attack, sample_rate),
				AREnvelope::sec_to_freq_k(env_release, sample_rate),
				0.49,
				false);

	// sidechain highpass
	if (sidechain_freq > 0.0)
	{
		m_comp.m_sidechain_flt = LowShelfPhon80(sidechain_freq, sample_rate);
		m_comp.m_use_sidechain_flt = true;
	}
	else
	{
         // use hardcoded lowshelf
         m_comp.m_sidechain_flt2 = LSFilter2(
             SIDECHAIN_SHELF_FREQ / sample_rate,
             LSFilter2::S_from_Q(SIDECHAIN_SHELF_Q, DB_TO_K(SIDECHAIN_SHELF_CUT)),
             DB_TO_K(SIDECHAIN_SHELF_CUT));

         m_comp.m_use_sidechain_flt = false;
     }

     // setup A/R envelopes
     m_comp.m_env_ar = AREnvelope(
         AREnvelope::sec_to_freq_k(attack_s, sample_rate),
         AREnvelope::sec_to_freq_k(release_s, sample_rate),
         0.49,
         sharp_mode);   // if (!sharp_mode) got S-curve; else exp-curve;

     // setup auto-off time
     m_comp.m_max_zero_samples = (size_t)(2.0 * release_s * sample_rate);    // use 2*release
}

void MonoCompressor::setEnvelopeK(double threshold, double knee, double ratio)
{
	assert(threshold >= 0.0 && threshold <= 1.0);
	assert(knee >= 0.0 && knee <= 1.0);

	// setup gain clipper
	m_comp.m_envelope_k = EnvelopeClip(threshold, knee, ratio);
}

void MonoCompressor::setGain(double gain, double dry_mix, double in_gain)
{
	m_comp.m_gain = gain * (1.0 - dry_mix);
	m_comp.m_dry_mix_k = dry_mix;
	m_comp.m_in_gain = in_gain;
}

double MonoCompressor::processSample(double x1)
{
	return m_comp.processSample(x1);
}

void MonoCompressor::reset()
{
	m_comp.reset();
}

bool MonoCompressor::getClip(bool reset)
{
	return m_comp.getClip(reset);
}

#endif
