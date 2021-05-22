/*
 *  Molot compressor engine declarations.
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

#ifndef _moloteng_h_
#define _moloteng_h_

#ifdef MOLOT_LITE_QUAD
#define MOLOT_LITE_STEREO 1
#endif

#define DB_TO_K(db)		(pow(10.0, (double)(db) / 20.0))
#define K_TO_DB(k)		(20.0 * log10(k))

// 2nd order filter
class Filter2 {
	public:
		Filter2(double a0, double a1, double a2, double b1, double b2);
		Filter2();
		Filter2& operator=(const Filter2& obj);

		double	processSample(double x);
		void		reset(bool reset_to_1 = false);

	protected:
		double	m_lp_a[3], m_lp_b[3];	// filter K

	private:
		double	m_xv[3], m_yv[3];			// filter state
};

// 4nd order filter
class Filter4 {
	public:
		Filter4(const Filter2 &flt0, const Filter2 &flt1);
		Filter4& operator=(const Filter4 &obj);

		double	processSample(double x);
		void		reset();

	protected:
		Filter2		m_flt0, m_flt1;
};

// 2nd order low-pass filter helper
class LPFilter2 : public Filter2 {
	public:
		LPFilter2(double freq_k, double Q = M_SQRT1_2);		// freq_k = freq / sample_rate
		LPFilter2& operator=(const LPFilter2& obj) { Filter2::operator=(obj); return *this; }
};

// 2nd order high-pass filter helper
class HPFilter2 : public Filter2 {
	public:
		HPFilter2(double freq_k, double Q);					// freq_k = freq / sample_rate;
		HPFilter2& operator=(const HPFilter2& obj) { Filter2::operator=(obj); return *this; }
};

// 2nd order low-shelf filter helper
class LSFilter2 : public Filter2 {
	public:
		LSFilter2(double freq_k, double S, double A);		// freq_k = freq / sample_rate; S is slope; A is gain
		LSFilter2& operator=(const LSFilter2& obj) { Filter2::operator=(obj); return *this; }

		static double	S_from_Q(double Q, double A);
};

// low-shelf by ISO226:2003 phon80 curve < 1000 Hz
class LowShelfPhon80 : public Filter4 {
	public:
		LowShelfPhon80(double freq_hz, double sample_rate);	// >=23.75 Hz supported; >=43.81 Hz without riple
		LowShelfPhon80& operator=(const LowShelfPhon80& obj) { Filter4::operator=(obj); return *this; }

	protected:
		static double	get_Q_from_hz(double freq_hz);
};

// attack/release envelope follower
class AREnvelope {
	public:
		AREnvelope(double attack_freq_k, double release_freq_k, double signal_freq_k, bool sharp_mode);

		AREnvelope& operator=(const AREnvelope &obj);

		static double	sec_to_freq_k(double sec, double sample_rate);
		static double	tau_to_freq_k(double sec, double sample_rate);

		double	processSampleAC(double x);
		double	processSampleDC(double x);

		void		reset(bool reset_to_1 = false);

	private:
		class ARFilter {
		public:
			ARFilter(double attack_freq_k, double release_freq_k, double signal_freq_k, bool sharp_mode);

			ARFilter& operator=(const ARFilter &obj);

			double	processSample(double x);
			void		reset(bool reset_to_1 = false);

			double	diode(double x);

		private:
			double		fa, fr;
			double		m_s;
			LPFilter2	m_diode_lpf;

			double		m_diode_xx;
			double		m_diode_ax[8];
		};

		LPFilter2	m_fabs_lpf;		// remove fabs aliasing
		ARFilter		m_filter;
};

// clip envelope with threshold/ratio given and get gain reduction K
class EnvelopeClip {
	public:
		EnvelopeClip(double thresh_k, double knee, double ratio);

		EnvelopeClip& operator=(const EnvelopeClip &obj);

		double	processSample(double x);

		bool		getClip(bool reset);

	private:
		double	m_thresh;
		double	m_ratio;
		double	m_thresh_ratio;
		int		m_ratio_index;
		double	m_hx_min;
		double	m_thresh2;

		double	m_A3, m_A4;

		double	h(double x);
		double	h1(double x);
		double	h2(double x);
		double	dh2(double x);

		double	getGR(double x);

		bool		m_clip;

		static double	m_sqrt_k[99][10];
};

// mono compressor with sidechain
class ChannelCompressor
{
	public:
		ChannelCompressor();

		// compressor objects
		LowShelfPhon80		m_sidechain_flt;
		LSFilter2			m_sidechain_flt2;
		AREnvelope			m_env;
		AREnvelope			m_env_ar;
		EnvelopeClip		m_envelope_k;

		double				m_detector_fix;

		size_t				m_max_zero_samples;

		struct State {
			double	over;
			double	env;
		};

		bool		preprocessSample(double x, struct State *);
		double	postprocessSample(struct State *);
		double	processSample(double x);

		void		reset();

		bool		getClip(bool reset);

		double				m_gain;
		double				m_dry_mix_k;
		double				m_in_gain;

		bool					m_use_sidechain_flt;

		double				m_gr;

	private:
		size_t				m_zero_samples;
		bool					m_is_on;
};

#ifdef MOLOT_LITE_STEREO

/*
 * StereoCompressor
 */

class StereoCompressor
{
	public:
		StereoCompressor();
		~StereoCompressor();

		enum stereo_mode_t {
			SL_STEREO,
			SL_2MONO,
			SL_R_SCHN,
			SL_M,
			SL_S,
			SL_MS
		};

		void		setEnvelope(double sample_rate, double attack_s, double release_s, double sidechain_freq, bool sharp_mode);
		void		setEnvelopeK(double threshold, double knee, double ratio);
		void		setGain(double gain, double dry_mix, double in_gain = 1.0);
		void		setStereoMode(stereo_mode_t sl);
		void		getGainReduction(double *g1, double *g2) const;

		void		processSample(double x1, double x2, double *y1, double *y2);
#ifdef MOLOT_LITE_QUAD
		void		processSampleSC(double x1, double x2, double sc1, double sc2, double *y1, double *y2);
#endif
		void		reset();

		// levels
		bool		getClip(bool reset);

	private:
		ChannelCompressor		m_comp[2];
		stereo_mode_t			m_stereo_mode;

		bool		isStereo() const;
};

#else

/*
 * MonoCompressor
 */

class MonoCompressor
{
	public:
		MonoCompressor();
		~MonoCompressor();
		void		setEnvelope(double sample_rate, double attack_s, double release_s, double sidechain_freq, bool sharp_mode);
		void		setEnvelopeK(double threshold, double knee, double ratio);
		void		setGain(double gain, double dry_mix, double in_gain = 1.0);
		double	getGainReduction() const;
		double	processSample(double);
		void		reset();
		bool		getClip(bool reset);

	private:
		ChannelCompressor		m_comp;
};

#endif

#endif
