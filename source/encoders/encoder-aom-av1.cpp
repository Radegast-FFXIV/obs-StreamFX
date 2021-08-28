// Copyright (c) 2021 Michael Fabian Dirks <info@xaymar.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "encoder-aom-av1.hpp"
#include <filesystem>
#include <thread>
#include "util/util-logging.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<encoder::aom::av1> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

#if defined(D_PLATFORM_WINDOWS)
#define D_LIB_SUFFIX ".dll"
#else
#define D_LIB_SUFFIX ".so"
#endif

#define ST_I18N "Encoder.AOM.AV1"

// AOM
#define ST_I18N_PRESET ST_I18N ".Preset"
#define ST_I18N_PRESET_USAGE ST_I18N_PRESET ".Usage"
#define ST_I18N_PRESET_USAGE_GOODQUALITY ST_I18N_PRESET_USAGE ".GoodQuality"
#define ST_I18N_PRESET_USAGE_REALTIME ST_I18N_PRESET_USAGE ".RealTime"
#define ST_I18N_PRESET_USAGE_ALLINTRA ST_I18N_PRESET_USAGE ".AllIntra"
#define ST_KEY_PRESET_USAGE "Preset.Usage"
#define ST_I18N_PRESET_CPUUSAGE ST_I18N_PRESET ".CPUUsage"
#define ST_KEY_PRESET_CPUUSAGE "Preset.CPUUsage"

// Rate Control
#define ST_I18N_RATECONTROL ST_I18N ".RateControl"
#define ST_I18N_RATECONTROL_MODE ST_I18N_RATECONTROL ".Mode"
#define ST_I18N_RATECONTROL_MODE_CBR ST_I18N_RATECONTROL_MODE ".CBR"
#define ST_I18N_RATECONTROL_MODE_VBR ST_I18N_RATECONTROL_MODE ".VBR"
#define ST_I18N_RATECONTROL_MODE_CQ ST_I18N_RATECONTROL_MODE ".CQ"
#define ST_I18N_RATECONTROL_MODE_Q ST_I18N_RATECONTROL_MODE ".Q"
#define ST_KEY_RATECONTROL_MODE "RateControl.Mode"
#define ST_I18N_RATECONTROL_LOOKAHEAD ST_I18N_RATECONTROL ".LookAhead"
#define ST_KEY_RATECONTROL_LOOKAHEAD "RateControl.LookAhead"
#define ST_I18N_RATECONTROL_LIMITS ST_I18N_RATECONTROL ".Limits"
#define ST_I18N_RATECONTROL_LIMITS_BITRATE ST_I18N_RATECONTROL_LIMITS ".Bitrate"
#define ST_KEY_RATECONTROL_LIMITS_BITRATE "RateControl.Limits.Bitrate"
#define ST_I18N_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT ST_I18N_RATECONTROL_LIMITS_BITRATE ".Undershoot"
#define ST_KEY_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT "RateControl.Limits.Bitrate.Undershoot"
#define ST_I18N_RATECONTROL_LIMITS_BITRATE_OVERSHOOT ST_I18N_RATECONTROL_LIMITS_BITRATE ".Overshoot"
#define ST_KEY_RATECONTROL_LIMITS_BITRATE_OVERSHOOT "RateControl.Limits.Bitrate.Overshoot"
#define ST_I18N_RATECONTROL_LIMITS_QUALITY ST_I18N_RATECONTROL_LIMITS ".Quality"
#define ST_KEY_RATECONTROL_LIMITS_QUALITY "RateControl.Limits.Quality"
#define ST_I18N_RATECONTROL_LIMITS_QUANTIZER_MINIMUM ST_I18N_RATECONTROL_LIMITS ".Quantizer.Minimum"
#define ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MINIMUM "RateControl.Limits.Quantizer.Minimum"
#define ST_I18N_RATECONTROL_LIMITS_QUANTIZER_MAXIMUM ST_I18N_RATECONTROL_LIMITS ".Quantizer.Maximum"
#define ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MAXIMUM "RateControl.Limits.Quantizer.Maximum"
#define ST_I18N_RATECONTROL_BUFFER ST_I18N_RATECONTROL ".Buffer"
#define ST_I18N_RATECONTROL_BUFFER_SIZE ST_I18N_RATECONTROL_BUFFER ".Size"
#define ST_KEY_RATECONTROL_BUFFER_SIZE "RateControl.Buffer.Size"
#define ST_I18N_RATECONTROL_BUFFER_SIZE_INITIAL ST_I18N_RATECONTROL_BUFFER_SIZE ".Initial"
#define ST_KEY_RATECONTROL_BUFFER_SIZE_INITIAL "RateControl.Buffer.Size.Initial"
#define ST_I18N_RATECONTROL_BUFFER_SIZE_OPTIMAL ST_I18N_RATECONTROL_BUFFER_SIZE ".Optimal"
#define ST_KEY_RATECONTROL_BUFFER_SIZE_OPTIMAL "RateControl.Buffer.Size.Optimal"

// Key-Frames
#define ST_I18N_KEYFRAMES ST_I18N ".KeyFrames"
#define ST_I18N_KEYFRAMES_INTERVALTYPE ST_I18N_KEYFRAMES ".IntervalType"
#define ST_I18N_KEYFRAMES_INTERVALTYPE_SECONDS ST_I18N_KEYFRAMES_INTERVALTYPE ".Seconds"
#define ST_I18N_KEYFRAMES_INTERVALTYPE_FRAMES ST_I18N_KEYFRAMES_INTERVALTYPE ".Frames"
#define ST_KEY_KEYFRAMES_INTERVALTYPE "KeyFrames.IntervalType"
#define ST_I18N_KEYFRAMES_INTERVAL ST_I18N_KEYFRAMES ".Interval"
#define ST_KEY_KEYFRAMES_INTERVAL_SECONDS "KeyFrames.Interval.Seconds"
#define ST_KEY_KEYFRAMES_INTERVAL_FRAMES "KeyFrames.Interval.Frames"

// Advanced
#define ST_I18N_ADVANCED ST_I18N ".Advanced"
#define ST_I18N_ADVANCED_THREADS ST_I18N_ADVANCED ".Threads"
#define ST_KEY_ADVANCED_THREADS "Advanced.Threads"
#define ST_I18N_ADVANCED_ROWMULTITHREADING ST_I18N_ADVANCED ".RowMultiThreading"
#define ST_KEY_ADVANCED_ROWMULTITHREADING "Advanced.RowMultiThreading"
#define ST_I18N_ADVANCED_TILE_COLUMNS ST_I18N_ADVANCED ".Tile.Columns"
#define ST_KEY_ADVANCED_TILE_COLUMNS "Advanced.Tile.Columns"
#define ST_I18N_ADVANCED_TILE_ROWS ST_I18N_ADVANCED ".Tile.Rows"
#define ST_KEY_ADVANCED_TILE_ROWS "Advanced.Tile.Rows"

using namespace streamfx::encoder::aom::av1;

static constexpr std::string_view HELP_URL = "https://github.com/Xaymar/obs-StreamFX/wiki/Encoder-AOM-AV1";

const char* obs_video_format_to_string(enum video_format format)
{
	switch (format) {
	case VIDEO_FORMAT_I420:
		return "I420";
	case VIDEO_FORMAT_NV12:
		return "NV12";
	case VIDEO_FORMAT_YVYU:
		return "YVYU";
	case VIDEO_FORMAT_YUY2:
		return "YUY";
	case VIDEO_FORMAT_UYVY:
		return "UYVY";
	case VIDEO_FORMAT_RGBA:
		return "RGBA";
	case VIDEO_FORMAT_BGRA:
		return "BGRA";
	case VIDEO_FORMAT_BGRX:
		return "BGRX";
	case VIDEO_FORMAT_Y800:
		return "Y800";
	case VIDEO_FORMAT_I444:
		return "I444";
	case VIDEO_FORMAT_BGR3:
		return "BGR3";
	case VIDEO_FORMAT_I422:
		return "I422";
	case VIDEO_FORMAT_I40A:
		return "I40A";
	case VIDEO_FORMAT_I42A:
		return "I42A";
	case VIDEO_FORMAT_YUVA:
		return "YUVA";
	case VIDEO_FORMAT_AYUV:
		return "AYUV";
	case VIDEO_FORMAT_NONE:
	default:
		return "Unknown";
	}
}

aom_av1_instance::aom_av1_instance(obs_data_t* settings, obs_encoder_t* self, bool is_hw)
	: obs::encoder_instance(settings, self, is_hw), _factory(aom_av1_factory::get()), _iface(nullptr), _ctx(), _cfg(),
	  _image_format(), _images(), _global_headers(nullptr)
{
	if (is_hw) {
		throw std::runtime_error("Hardware encoding isn't even registered, how did you get here?");
	}

	// Video Information
	video_t*                        video      = obs_encoder_video(_self);
	const struct video_output_info* video_info = video_output_get_info(video);
	uint32_t                        width      = obs_encoder_get_width(_self);
	uint32_t                        height     = obs_encoder_get_height(_self);
	uint32_t                        fps_num    = video_info->fps_num;
	uint32_t                        fps_den    = video_info->fps_den;
	bool                            monochrome = (video_info->format == VIDEO_FORMAT_Y800);
	video_scale_info                ovsi;
	aom_color_primaries_t           aom_primaries;
	aom_transfer_characteristics_t  aom_transfer;
	aom_matrix_coefficients_t       aom_matrix;
	aom_color_range_t               aom_range;

	{ // Figure out the correct color format and stuff.
		ovsi.colorspace = video_info->colorspace;
		ovsi.format     = video_info->format;
		ovsi.range      = video_info->range;
		get_video_info(&ovsi);

		switch (ovsi.format) {
		case VIDEO_FORMAT_I420:
			_image_format = AOM_IMG_FMT_I420;
			break;
		case VIDEO_FORMAT_I422:
			_image_format = AOM_IMG_FMT_I422;
			break;
		case VIDEO_FORMAT_I444:
			_image_format = AOM_IMG_FMT_I444;
			break;
		default:
			throw std::runtime_error("Something went wrong figuring out our color format.");
		}
	}

	// Retrieve encoder interface.
	_iface = _factory->libaom_codec_av1_cx();
	if (!_iface) {
		throw std::runtime_error("AOM library does not provide AV1 encoder.");
	}

	{ // Configuration

		{ // Usage and Defaults
			_cfg.g_usage = static_cast<unsigned int>(obs_data_get_int(settings, ST_KEY_PRESET_USAGE));
			_factory->libaom_codec_enc_config_default(_iface, &_cfg, _cfg.g_usage);
		}

		{ // Frame Information
			// Size
			_cfg.g_w = width;
			_cfg.g_h = height;

			// Time Base (Rate is inverted Time Base)
			_cfg.g_timebase.num = fps_den;
			_cfg.g_timebase.den = fps_num;

			// !INFO: Whenever OBS decides to support anything but 8-bits, let me know.
			_cfg.g_bit_depth       = AOM_BITS_8;
			_cfg.g_input_bit_depth = AOM_BITS_8;

			// AV1 Profile
			if (ovsi.format == VIDEO_FORMAT_I444) {
				_cfg.g_profile = 1;
			} else if (ovsi.format == VIDEO_FORMAT_I422) {
				_cfg.g_profile = 2;
			}
		}

		{ // Advanced Controls
			// Monochrome
			_cfg.monochrome = monochrome ? 1 : 0;

			// Passes
			_cfg.g_pass = AOM_RC_ONE_PASS;

			// Threads
			if (auto threads = obs_data_get_int(settings, ST_KEY_ADVANCED_THREADS); threads > 0) {
				_cfg.g_threads = static_cast<unsigned int>(threads);
			} else {
				_cfg.g_threads = std::thread::hardware_concurrency();
			}
		}

		// User-controlled
		update(settings);
	}

	// Initialize Encoder
	if (auto error = _factory->libaom_codec_enc_init_ver(&_ctx, _iface, &_cfg, 0, AOM_ENCODER_ABI_VERSION);
		error != AOM_CODEC_OK) {
		const char* errstr = _factory->libaom_codec_err_to_string(error);
		D_LOG_ERROR("Failed to initialize codec, unexpected error: %s (code %" PRIu32 ")", errstr, error);
		throw std::runtime_error(errstr);
	}

	{ // Control Settings

		{ // Color Information
			switch (ovsi.colorspace) {
			case VIDEO_CS_601:
				aom_primaries = AOM_CICP_CP_BT_601;
				aom_transfer  = AOM_CICP_TC_BT_601;
				aom_matrix    = AOM_CICP_MC_BT_601;
				break;
			case VIDEO_CS_709:
				aom_primaries = AOM_CICP_CP_BT_709;
				aom_transfer  = AOM_CICP_TC_BT_709;
				aom_matrix    = AOM_CICP_MC_BT_709;
				break;
			case VIDEO_CS_SRGB:
				aom_primaries = AOM_CICP_CP_BT_709;
				aom_transfer  = AOM_CICP_TC_BT_709;
				aom_matrix    = AOM_CICP_MC_BT_709;
				break;
			}

#ifdef AOM_CTRL_AV1E_SET_COLOR_PRIMARIES
			if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_COLOR_PRIMARIES, aom_primaries);
				error != AOM_CODEC_OK) {
				const char* errstr = _factory->libaom_codec_err_to_string(error);
				D_LOG_WARNING("Failed to change %s, error: %s (code %" PRIu32 ")", "Color Primaries", errstr, error);
			}
#endif
#ifdef AOM_CTRL_AV1E_SET_TRANSFER_CHARACTERISTICS
			if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_TRANSFER_CHARACTERISTICS, aom_transfer);
				error != AOM_CODEC_OK) {
				const char* errstr = _factory->libaom_codec_err_to_string(error);
				D_LOG_WARNING("Failed to change %s, error: %s (code %" PRIu32 ")", "Transfer Characteristics", errstr,
							  error);
			}
#endif
#ifdef AOM_CTRL_AV1E_SET_MATRIX_COEFFICIENTS
			if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_MATRIX_COEFFICIENTS, aom_matrix);
				error != AOM_CODEC_OK) {
				const char* errstr = _factory->libaom_codec_err_to_string(error);
				D_LOG_WARNING("Failed to change %s, error: %s (code %" PRIu32 ")", "Color Matrix", errstr, error);
			}
#endif

#ifdef AOM_CTRL_AV1E_SET_COLOR_RANGE
			switch (ovsi.range) {
			case VIDEO_RANGE_FULL:
				aom_range = AOM_CR_FULL_RANGE;
				break;
			case VIDEO_RANGE_PARTIAL:
				aom_range = AOM_CR_STUDIO_RANGE;
				break;
			}

			if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_COLOR_RANGE, aom_range);
				error != AOM_CODEC_OK) {
				const char* errstr = _factory->libaom_codec_err_to_string(error);
				D_LOG_WARNING("Failed to change %s, error: %s (code %" PRIu32 ")", "Color Range", errstr, error);
			}
#endif
#ifdef AOM_CTRL_AV1E_SET_CHROMA_SAMPLE_POSITION
			// !TODO: Consider making this user-controlled. At the moment, this follows the H.264 chroma standard.
			if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_CHROMA_SAMPLE_POSITION, AOM_CSP_VERTICAL);
				error != AOM_CODEC_OK) {
				const char* errstr = _factory->libaom_codec_err_to_string(error);
				D_LOG_WARNING("Failed to change %s, error: %s (code %" PRIu32 ")", "AV1E_SET_CHROMA_SAMPLE_POSITION",
							  errstr, error);
			}
#endif
		}

		// User-configured
		if (!update(settings)) {
			throw std::runtime_error("Unexpected error during configuration.");
		}
	}

	// Preallocate global headers.
	_global_headers = _factory->libaom_codec_get_global_headers(&_ctx);

	// Allocate frames.
	for (size_t idx = 0; idx < _cfg.g_threads; idx++) {
		// Allocate and queue frame.
		std::shared_ptr<aom_image_t> image = std::make_shared<aom_image_t>();
		_factory->libaom_img_alloc(image.get(), _image_format, _cfg.g_w, _cfg.g_h, 4);
		_images.push(image);

		// Set various parameters.
		image->fmt        = _image_format;
		image->cp         = aom_primaries;
		image->tc         = aom_transfer;
		image->mc         = aom_matrix;
		image->monochrome = monochrome ? 1 : 0;
		image->csp        = AOM_CSP_VERTICAL; // !TODO: Consider making this user-controlled.
		image->range      = aom_range;
		//image->r_w        = width;
		//image->r_h        = height;
	}
}

aom_av1_instance::~aom_av1_instance()
{
	// Deallocate global buffer.
	if (_global_headers) {
		/* Breaks heap
		if (_global_headers->buf) {
			free(_global_headers->buf);
			_global_headers->buf = nullptr;
		}
		free(_global_headers);
		_global_headers = nullptr;
		*/
	}

	// Deallocate frames.
	while (_images.size() > 0) {
		auto image = _images.front();
		_factory->libaom_img_free(image.get());
		_images.pop();
	}

	// Destroy encoder.
	_factory->libaom_codec_destroy(&_ctx);
}

void aom_av1_instance::migrate(obs_data_t* settings, uint64_t version) {}

bool aom_av1_instance::update(obs_data_t* settings)
{
	video_t*                        obsVideo      = obs_encoder_video(_self);
	const struct video_output_info* obsVideoInfo  = video_output_get_info(obsVideo);
	uint32_t                        obsFPSnum     = obsVideoInfo->fps_num;
	uint32_t                        obsFPSden     = obsVideoInfo->fps_den;
	bool                            obsMonochrome = (obsVideoInfo->format == VIDEO_FORMAT_Y800);

	{ // Configuration.

		//_cfg.g_forced_max_frame_width = ?;
		//_cfg.g_forced_max_frame_height = ?;
		//_cfg.g_error_resilient = ?;
		//_cfg.g_lag_in_frames = ?;
		//_cfg.sframe_dist = ?;
		//_cfg.sframe_mode = ?;
		//_cfg.large_scale_tile = ?;
		//_cfg.full_still_picture_hdr = ?;
		//_cfg.save_as_annexb = ?;
		//_cfg.encoder_cfg = ?;

		{ // Rate Control

			// Usage
			_cfg.rc_end_usage = static_cast<aom_rc_mode>(obs_data_get_int(settings, ST_KEY_RATECONTROL_MODE));

			// Look-ahead
			if (auto v = obs_data_get_int(settings, ST_KEY_RATECONTROL_LOOKAHEAD); v != -1) {
				_cfg.g_lag_in_frames = static_cast<unsigned int>(v);
			}

			// Limits
			if (auto v = obs_data_get_int(settings, ST_KEY_RATECONTROL_LIMITS_BITRATE); v != -1) {
				_cfg.rc_target_bitrate = static_cast<unsigned int>(v);
			}
			if (auto v = obs_data_get_int(settings, ST_KEY_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT); v != -1) {
				_cfg.rc_undershoot_pct = static_cast<unsigned int>(v);
			}
			if (auto v = obs_data_get_int(settings, ST_KEY_RATECONTROL_LIMITS_BITRATE_OVERSHOOT); v != -1) {
				_cfg.rc_overshoot_pct = static_cast<unsigned int>(v);
			}
			if (auto v = obs_data_get_int(settings, ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MINIMUM); v != -1) {
				_cfg.rc_min_quantizer = static_cast<unsigned int>(v);
			}
			if (auto v = obs_data_get_int(settings, ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MAXIMUM); v != -1) {
				_cfg.rc_max_quantizer = static_cast<unsigned int>(v);
			}

			// Buffer
			if (auto v = obs_data_get_int(settings, ST_KEY_RATECONTROL_BUFFER_SIZE); v != -1) {
				_cfg.rc_buf_sz = static_cast<unsigned int>(v);
			}
			if (auto v = obs_data_get_int(settings, ST_KEY_RATECONTROL_BUFFER_SIZE_INITIAL); v != -1) {
				_cfg.rc_buf_initial_sz = static_cast<unsigned int>(v);
			}
			if (auto v = obs_data_get_int(settings, ST_KEY_RATECONTROL_BUFFER_SIZE_OPTIMAL); v != -1) {
				_cfg.rc_buf_optimal_sz = static_cast<unsigned int>(v);
			}

			// Override certain settings if a specific mode is set.
			if (_cfg.rc_end_usage == AOM_CBR) {
				_cfg.rc_overshoot_pct  = 0;
				_cfg.rc_undershoot_pct = 0;
			} else if (_cfg.rc_end_usage == AOM_Q) {
				_cfg.rc_target_bitrate = 0;
				_cfg.rc_overshoot_pct  = 0;
				_cfg.rc_undershoot_pct = 0;

			} else if (_cfg.rc_end_usage == AOM_VBR) {
			} else if (_cfg.rc_end_usage == AOM_CQ) {
			}

			//_cfg.rc_dropframe_tresh = ?;
			//_cfg.rc_resize_mode = ?;
			//_cfg.rc_resize_denominator = ?;
			//_cfg.rc_resize_kf_denominator = ?;
			//_cfg.rc_superres_mode = ?;
			//_cfg.rc_superres_denominator = ?;
			//_cfg.rc_superres_kf_denominator = ?;
			//_cfg.rc_superres_qthresh = ?;
			//_cfg.rc_superres_kf_qtresh = ?;
		}

		{ // Key-Frames
			int64_t kf_type    = obs_data_get_int(settings, ST_KEY_KEYFRAMES_INTERVALTYPE);
			bool    is_seconds = (kf_type == 0);

			_cfg.kf_mode = AOM_KF_AUTO;
			if (is_seconds) {
				_cfg.kf_max_dist =
					static_cast<unsigned int>(obs_data_get_double(settings, ST_KEY_KEYFRAMES_INTERVAL_SECONDS)
											  * static_cast<double>(obsFPSnum) / static_cast<double>(obsFPSden));
			} else {
				_cfg.kf_max_dist =
					static_cast<unsigned int>(obs_data_get_int(settings, ST_KEY_KEYFRAMES_INTERVAL_FRAMES));
			}
			_cfg.kf_min_dist = _cfg.kf_max_dist;

			// ToDo: Variable Key-Frames, forward reference keyframes
			//_cfg.fwd_kf_enabled = ?;
		}

		// Apply configuration
		if (_ctx.iface) {
			if (auto error = _factory->libaom_codec_enc_config_set(&_ctx, &_cfg); error != AOM_CODEC_OK) {
				const char* errstr = _factory->libaom_codec_err_to_string(error);
				D_LOG_ERROR("Failed to apply configuration, error: %s (code %" PRIu32 ")\n%s\n%s", errstr, error,
							_factory->libaom_codec_error(&_ctx), _factory->libaom_codec_error_detail(&_ctx));
				return false;
			}
		}
	}

	if (_ctx.iface) { // Control

		{ // Presets
#ifdef AOM_CTRL_AOME_SET_CPUUSED
			if (auto v = obs_data_get_int(settings, ST_KEY_PRESET_CPUUSAGE); v != -1) {
				if (auto error = _factory->libaom_codec_control(&_ctx, AOME_SET_CPUUSED, v); error != AOM_CODEC_OK) {
					const char* errstr = _factory->libaom_codec_err_to_string(error);
					D_LOG_WARNING("Failed to change %s, error: %s (code %" PRIu32 ")\n%s\n%s",
								  D_TRANSLATE(ST_I18N_PRESET_CPUUSAGE), errstr, error,
								  _factory->libaom_codec_error(&_ctx), _factory->libaom_codec_error_detail(&_ctx));
				}
			}
#endif
		}

		{ // Rate Control
#ifdef AOM_CTRL_AOME_SET_CQ_LEVEL
			if (auto v = obs_data_get_int(settings, ST_KEY_RATECONTROL_LIMITS_QUALITY);
				(v != -1) && ((_cfg.rc_end_usage == AOM_CQ) || (_cfg.rc_end_usage == AOM_Q))) {
				if (auto error = _factory->libaom_codec_control(&_ctx, AOME_SET_CQ_LEVEL, v); error != AOM_CODEC_OK) {
					const char* errstr = _factory->libaom_codec_err_to_string(error);
					D_LOG_WARNING("Failed to change %s, error: %s (code %" PRIu32 ")\n%s\n%s",
								  D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS_QUALITY), errstr, error,
								  _factory->libaom_codec_error(&_ctx), _factory->libaom_codec_error_detail(&_ctx));
				}
			}
#endif
		}

		{ // Advanced
#ifdef AOM_CTRL_AV1E_SET_ROW_MT
			if (auto v = obs_data_get_int(settings, ST_KEY_ADVANCED_ROWMULTITHREADING); v != -1) {
				if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_ROW_MT, v); error != AOM_CODEC_OK) {
					const char* errstr = _factory->libaom_codec_err_to_string(error);
					D_LOG_WARNING("Failed to change %s, error: %s (code %" PRIu32 ")\n%s\n%s",
								  D_TRANSLATE(ST_I18N_ADVANCED_ROWMULTITHREADING), errstr, error,
								  _factory->libaom_codec_error(&_ctx), _factory->libaom_codec_error_detail(&_ctx));
				}
			}
#endif

#ifdef AOM_CTRL_AV1E_SET_TILE_COLUMNS
			if (auto v = obs_data_get_int(settings, ST_KEY_ADVANCED_TILE_COLUMNS); v != -1) {
				if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_TILE_COLUMNS, v);
					error != AOM_CODEC_OK) {
					const char* errstr = _factory->libaom_codec_err_to_string(error);
					D_LOG_WARNING("Failed to change %s, error: %s (code %" PRIu32 ")\n%s\n%s",
								  D_TRANSLATE(ST_I18N_ADVANCED_TILE_COLUMNS), errstr, error,
								  _factory->libaom_codec_error(&_ctx), _factory->libaom_codec_error_detail(&_ctx));
				}
			}
#endif

#ifdef AOM_CTRL_AV1E_SET_TILE_ROWS
			if (auto v = obs_data_get_int(settings, ST_KEY_ADVANCED_TILE_ROWS); v != -1) {
				if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_TILE_ROWS, v); error != AOM_CODEC_OK) {
					const char* errstr = _factory->libaom_codec_err_to_string(error);
					D_LOG_WARNING("Failed to change %s, error: %s (code %" PRIu32 ")\n%s\n%s",
								  D_TRANSLATE(ST_I18N_ADVANCED_TILE_ROWS), errstr, error,
								  _factory->libaom_codec_error(&_ctx), _factory->libaom_codec_error_detail(&_ctx));
				}
			}
#endif
		}
	}

	return true;
}

bool aom_av1_instance::get_extra_data(uint8_t** extra_data, size_t* size)
{
	if (!_global_headers) {
		return false;
	}

	*extra_data = static_cast<uint8_t*>(_global_headers->buf);
	*size       = _global_headers->sz;

	return true;
}

bool aom_av1_instance::get_sei_data(uint8_t** sei_data, size_t* size)
{
	return get_extra_data(sei_data, size);
}

void aom_av1_instance::get_video_info(struct video_scale_info* info)
{
	// Fix up color format.
	auto format = obs_encoder_get_preferred_video_format(_self);
	if (format == VIDEO_FORMAT_NONE) {
		format = info->format;
	}

	switch (format) {
		// Perfect matches.
	case VIDEO_FORMAT_I444: // AOM_IMG_I444.
	case VIDEO_FORMAT_I422: // AOM_IMG_I422.
	case VIDEO_FORMAT_I420: // AOM_IMG_I420.
		break;

		// 4:2:0 formats
	case VIDEO_FORMAT_NV12: // Y, UV Interleaved.
	case VIDEO_FORMAT_I40A:
		D_LOG_WARNING("Color-format '%s' is not supported, forcing 'I420'...", obs_video_format_to_string(format));
		info->format = VIDEO_FORMAT_I420;
		break;

		// 4:2:2-like formats
	case VIDEO_FORMAT_UYVY:
	case VIDEO_FORMAT_YUY2:
	case VIDEO_FORMAT_YVYU:
	case VIDEO_FORMAT_I42A:
		D_LOG_WARNING("Color-format '%s' is not supported, forcing 'I422'...", obs_video_format_to_string(format));
		info->format = VIDEO_FORMAT_I422;
		break;

		// 4:4:4
	case VIDEO_FORMAT_BGR3:
	case VIDEO_FORMAT_BGRA:
	case VIDEO_FORMAT_BGRX:
	case VIDEO_FORMAT_RGBA:
	case VIDEO_FORMAT_YUVA:
	case VIDEO_FORMAT_AYUV:
	case VIDEO_FORMAT_Y800: // Grayscale, no exact match.
		D_LOG_WARNING("Color-format '%s' is not supported, forcing 'I444'...", obs_video_format_to_string(format));
		info->format = VIDEO_FORMAT_I444;
		break;
	}

	// Fix up color space.
	if (info->colorspace == VIDEO_CS_DEFAULT) {
		info->colorspace = VIDEO_CS_SRGB;
	}

	// Fix up color range.
	if (info->range == VIDEO_RANGE_DEFAULT) {
		info->range = VIDEO_RANGE_PARTIAL;
	}
}

bool streamfx::encoder::aom::av1::aom_av1_instance::encode_video(encoder_frame* frame, encoder_packet* packet,
																 bool* received_packet)
{
	auto image = _images.front();

	if (_image_format == AOM_IMG_FMT_I444) {
		std::memcpy(image->planes[AOM_PLANE_Y], frame->data[0], frame->linesize[0] * image->h);
		std::memcpy(image->planes[AOM_PLANE_U], frame->data[1], frame->linesize[1] * image->h);
		std::memcpy(image->planes[AOM_PLANE_V], frame->data[2], frame->linesize[2] * image->h);
	} else if (_image_format == AOM_IMG_FMT_I422) {
		std::memcpy(image->planes[AOM_PLANE_Y], frame->data[0], frame->linesize[0] * image->h);
		std::memcpy(image->planes[AOM_PLANE_U], frame->data[1], frame->linesize[1] * image->h);
		std::memcpy(image->planes[AOM_PLANE_V], frame->data[2], frame->linesize[2] * image->h);
	} else if (_image_format == AOM_IMG_FMT_I420) {
		std::memcpy(image->planes[AOM_PLANE_Y], frame->data[0], frame->linesize[0] * image->h);
		std::memcpy(image->planes[AOM_PLANE_U], frame->data[1], frame->linesize[1] * image->h / 2);
		std::memcpy(image->planes[AOM_PLANE_V], frame->data[2], frame->linesize[2] * image->h / 2);
	}

	if (auto error = _factory->libaom_codec_encode(&_ctx, image.get(), frame->pts, 1, 0); error != AOM_CODEC_OK) {
		const char* errstr = _factory->libaom_codec_err_to_string(error);
		D_LOG_ERROR("Encoding frame failed with error: %s (code %" PRIu32 ")\n%s\n%s", errstr, error,
					_factory->libaom_codec_error(&_ctx), _factory->libaom_codec_error_detail(&_ctx));
		return false;
	} else {
		// Add to the back to the queue.
		_images.pop();
		_images.push(image);
	}

	// Get Packet
	aom_codec_iter_t iter = NULL;
	for (const aom_codec_cx_pkt_t* pkt = _factory->libaom_codec_get_cx_data(&_ctx, &iter); pkt != nullptr;
		 pkt                           = _factory->libaom_codec_get_cx_data(&_ctx, &iter)) {
		if (*received_packet == true)
			break;

		if (pkt->kind == AOM_CODEC_CX_FRAME_PKT) {
			// Status
			packet->type     = OBS_ENCODER_VIDEO;
			packet->keyframe = (pkt->data.frame.flags & AOM_FRAME_IS_KEY) == AOM_FRAME_IS_KEY;
			if (packet->keyframe) {
				//
				packet->priority      = 0;
				packet->drop_priority = packet->priority;
			} else if ((pkt->data.frame.flags & AOM_FRAME_IS_DROPPABLE) != AOM_FRAME_IS_DROPPABLE) {
				// Dropping this frame breaks the bitstream.
				packet->priority      = -1;
				packet->drop_priority = packet->priority;
			} else {
				// This frame can be dropped at will.
				packet->priority      = -2;
				packet->drop_priority = packet->priority;
			}

			// Data
			packet->data = static_cast<uint8_t*>(pkt->data.frame.buf);
			packet->size = pkt->data.frame.sz;

			// Time
			/*
			packet->timebase_num = _cfg.g_timebase.num;
			packet->timebase_den = _cfg.g_timebase.den;
			*/
			packet->pts = pkt->data.frame.pts + 1;
			packet->dts =
				pkt->data.frame.pts; // !TODO: Figure out how to properly get the decode timestamp from a packet.

			*received_packet = true;
		}
	}

	if (!*received_packet) {
		return false;
	}

	P_LOG_INFO("Packet PTS=%" PRId64 " DTS=%" PRId64 " Size=%" PRIuPTR "", packet->pts, packet->dts, packet->size);

	return true;
}

aom_av1_factory::aom_av1_factory()
{
	// Try and load the AOM library.
	std::vector<std::filesystem::path> libs;

#ifdef D_PLATFORM_WINDOWS
	libs.push_back(streamfx::data_file_path("aom" D_LIB_SUFFIX)); // MSVC (preferred)
	libs.push_back("aom" D_LIB_SUFFIX);
#endif
	libs.push_back(streamfx::data_file_path("libaom" D_LIB_SUFFIX)); // Cross-Compile
	libs.push_back("libaom" D_LIB_SUFFIX);

	for (auto lib : libs) {
		try {
			_library = streamfx::util::library::load(lib);
			if (_library)
				break;
		} catch (...) {
			D_LOG_WARNING("Loading of '%s' failed.", lib.generic_string().c_str());
		}
	}
	if (!_library) {
		throw std::runtime_error("Unable to load AOM library.");
	}

	// Load all necessary functions.
#define _LOAD_SYMBOL(X)                                                                      \
	{                                                                                        \
		lib##X = reinterpret_cast<decltype(lib##X)>(_library->load_symbol(std::string(#X))); \
	}
#define _LOAD_SYMBOL_(X, Y)                                                                         \
	{                                                                                            \
		lib##X = reinterpret_cast<decltype(lib##X)>(_library->load_symbol(std::string(Y))); \
	}
	_LOAD_SYMBOL(aom_codec_version);
	_LOAD_SYMBOL(aom_codec_version_str);
	_LOAD_SYMBOL(aom_codec_version_extra_str);
	_LOAD_SYMBOL(aom_codec_build_config);
	_LOAD_SYMBOL(aom_codec_iface_name);
	_LOAD_SYMBOL(aom_codec_err_to_string);
	_LOAD_SYMBOL(aom_codec_error);
	_LOAD_SYMBOL(aom_codec_error_detail);
	_LOAD_SYMBOL(aom_codec_destroy);
	_LOAD_SYMBOL(aom_codec_get_caps);
	_LOAD_SYMBOL(aom_codec_control);
	_LOAD_SYMBOL(aom_codec_set_option);
	_LOAD_SYMBOL(aom_obu_type_to_string);
	_LOAD_SYMBOL(aom_uleb_size_in_bytes);
	_LOAD_SYMBOL(aom_uleb_decode);
	_LOAD_SYMBOL(aom_uleb_encode);
	_LOAD_SYMBOL(aom_uleb_encode_fixed_size);
	_LOAD_SYMBOL(aom_img_alloc);
	_LOAD_SYMBOL(aom_img_wrap);
	_LOAD_SYMBOL(aom_img_alloc_with_border);
	_LOAD_SYMBOL(aom_img_set_rect);
	_LOAD_SYMBOL(aom_img_flip);
	_LOAD_SYMBOL(aom_img_free);
	_LOAD_SYMBOL(aom_img_plane_width);
	_LOAD_SYMBOL(aom_img_plane_height);
	_LOAD_SYMBOL(aom_img_add_metadata);
	_LOAD_SYMBOL(aom_img_get_metadata);
	_LOAD_SYMBOL(aom_img_num_metadata);
	_LOAD_SYMBOL(aom_img_remove_metadata);
	_LOAD_SYMBOL(aom_img_metadata_alloc);
	_LOAD_SYMBOL(aom_img_metadata_free);
	_LOAD_SYMBOL(aom_codec_enc_init_ver);
	_LOAD_SYMBOL(aom_codec_enc_config_default);
	_LOAD_SYMBOL(aom_codec_enc_config_set);
	_LOAD_SYMBOL(aom_codec_get_global_headers);
	_LOAD_SYMBOL(aom_codec_encode);
	_LOAD_SYMBOL(aom_codec_set_cx_data_buf);
	_LOAD_SYMBOL(aom_codec_get_cx_data);
	_LOAD_SYMBOL(aom_codec_get_preview_frame);
	_LOAD_SYMBOL(aom_codec_av1_cx);
#undef _LOAD_SYMBOL

	// Register encoder.
	_info.id    = S_PREFIX "aom-av1";
	_info.type  = obs_encoder_type::OBS_ENCODER_VIDEO;
	_info.codec = "av1";

	finish_setup();
}

aom_av1_factory::~aom_av1_factory() {}

std::shared_ptr<aom_av1_factory> _aom_av1_factory_instance = nullptr;

void aom_av1_factory::initialize()
try {
	if (!_aom_av1_factory_instance) {
		_aom_av1_factory_instance = std::make_shared<aom_av1_factory>();
	}
} catch (std::exception const& ex) {
	D_LOG_ERROR("Failed to initialize AOM AV1 encoder: %s", ex.what());
}

void aom_av1_factory::finalize()
{
	_aom_av1_factory_instance.reset();
}

std::shared_ptr<aom_av1_factory> aom_av1_factory::get()
{
	return _aom_av1_factory_instance;
}

const char* aom_av1_factory::get_name()
{
	return "AV1 (via AOM)";
}

void* aom_av1_factory::create(obs_data_t* settings, obs_encoder_t* encoder, bool is_hw)
{
	return new aom_av1_instance(settings, encoder, is_hw);
}

void aom_av1_factory::get_defaults2(obs_data_t* settings)
{
	{ // Presets
		obs_data_set_default_int(settings, ST_KEY_PRESET_USAGE, static_cast<long long>(AOM_USAGE_REALTIME));
		obs_data_set_default_int(settings, ST_KEY_PRESET_CPUUSAGE, -1);
	}

	{ // Rate-Control
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_MODE, static_cast<long long>(AOM_CBR));

		// Limits
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_LIMITS_BITRATE, 6000);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT, -1);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_LIMITS_BITRATE_OVERSHOOT, -1);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_LIMITS_QUALITY, -1);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MINIMUM, -1);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MAXIMUM, -1);

		// Buffer
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_BUFFER_SIZE, -1);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_BUFFER_SIZE_INITIAL, -1);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_BUFFER_SIZE_OPTIMAL, -1);
	}

	{ // Key-Frame Options
		obs_data_set_default_int(settings, ST_KEY_KEYFRAMES_INTERVALTYPE, 0);
		obs_data_set_default_double(settings, ST_KEY_KEYFRAMES_INTERVAL_SECONDS, 2.0);
		obs_data_set_default_int(settings, ST_KEY_KEYFRAMES_INTERVAL_FRAMES, 300);
	}

	{ // Advanced Options
		obs_data_set_default_int(settings, ST_KEY_ADVANCED_THREADS, 0);
		obs_data_set_default_int(settings, ST_KEY_ADVANCED_ROWMULTITHREADING, -1);
		obs_data_set_default_int(settings, ST_KEY_ADVANCED_TILE_COLUMNS, -1);
		obs_data_set_default_int(settings, ST_KEY_ADVANCED_TILE_ROWS, -1);
	}
}

static bool modified_keyframes(obs_properties_t* props, obs_property_t*, obs_data_t* settings) noexcept
try {
	bool is_seconds = obs_data_get_int(settings, ST_KEY_KEYFRAMES_INTERVALTYPE) == 0;
	obs_property_set_visible(obs_properties_get(props, ST_KEY_KEYFRAMES_INTERVAL_FRAMES), !is_seconds);
	obs_property_set_visible(obs_properties_get(props, ST_KEY_KEYFRAMES_INTERVAL_SECONDS), is_seconds);
	return true;
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
	return false;
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return false;
}

static bool modified_ratecontrol_mode(obs_properties_t* props, obs_property_t*, obs_data_t* settings) noexcept
try {
	bool is_bitrate_visible        = false;
	bool is_overundershoot_visible = false;
	bool is_quality_visible        = false;

	aom_rc_mode mode = static_cast<aom_rc_mode>(obs_data_get_int(settings, ST_KEY_RATECONTROL_MODE));
	if (mode == AOM_CBR) {
		is_bitrate_visible = true;
	} else if (mode == AOM_VBR) {
		is_bitrate_visible        = true;
		is_overundershoot_visible = true;
	} else if (mode == AOM_CQ) {
		is_bitrate_visible        = true;
		is_overundershoot_visible = true;
		is_quality_visible        = true;
	} else if (mode == AOM_Q) {
		is_quality_visible = true;
	}

	obs_property_set_visible(obs_properties_get(props, ST_KEY_RATECONTROL_LIMITS_BITRATE), is_bitrate_visible);
	obs_property_set_visible(obs_properties_get(props, ST_KEY_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT),
							 is_overundershoot_visible);
	obs_property_set_visible(obs_properties_get(props, ST_KEY_RATECONTROL_LIMITS_BITRATE_OVERSHOOT),
							 is_overundershoot_visible);
#ifdef AOM_CTRL_AOME_SET_CQ_LEVEL
	obs_property_set_visible(obs_properties_get(props, ST_KEY_RATECONTROL_LIMITS_QUALITY), is_quality_visible);
#endif
	return true;
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
	return false;
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return false;
}

obs_properties_t* aom_av1_factory::get_properties2(instance_t* data)
{
	obs_properties_t* props = obs_properties_create();

#ifdef ENABLE_FRONTEND
	{
		obs_properties_add_button2(props, S_MANUAL_OPEN, D_TRANSLATE(S_MANUAL_OPEN), aom_av1_factory::on_manual_open,
								   this);
	}
#endif

	{ // Presets
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(props, ST_I18N_PRESET, D_TRANSLATE(ST_I18N_PRESET), OBS_GROUP_NORMAL, grp);

		{ // Usage
			auto p = obs_properties_add_list(grp, ST_KEY_PRESET_USAGE, D_TRANSLATE(ST_I18N_PRESET_USAGE),
											 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_PRESET_USAGE_GOODQUALITY),
									  static_cast<long long>(AOM_USAGE_GOOD_QUALITY));
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_PRESET_USAGE_REALTIME),
									  static_cast<long long>(AOM_USAGE_REALTIME));
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_PRESET_USAGE_ALLINTRA),
									  static_cast<long long>(AOM_USAGE_ALL_INTRA));
		}

		{ // CPU Usage
			auto p = obs_properties_add_int_slider(grp, ST_KEY_PRESET_CPUUSAGE, D_TRANSLATE(ST_I18N_PRESET_CPUUSAGE),
												   -1, 9, 1);
		}
	}

	{ // Rate Control Options
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(props, ST_I18N_RATECONTROL, D_TRANSLATE(ST_I18N_RATECONTROL), OBS_GROUP_NORMAL, grp);

		{ // Mode
			auto p = obs_properties_add_list(grp, ST_KEY_RATECONTROL_MODE, D_TRANSLATE(ST_I18N_RATECONTROL_MODE),
											 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_set_modified_callback(p, modified_ratecontrol_mode);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_RATECONTROL_MODE_CBR), static_cast<long long>(AOM_CBR));
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_RATECONTROL_MODE_VBR), static_cast<long long>(AOM_VBR));
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_RATECONTROL_MODE_CQ), static_cast<long long>(AOM_CQ));
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_RATECONTROL_MODE_Q), static_cast<long long>(AOM_Q));
		}

		{ // Look-Ahead
			auto p =
				obs_properties_add_int(grp, ST_KEY_RATECONTROL_LOOKAHEAD, D_TRANSLATE(ST_I18N_RATECONTROL_LOOKAHEAD),
									   -1, std::numeric_limits<int32_t>::max(), 1);
			obs_property_int_set_suffix(p, " frames");
		}

		{ // Limits
			obs_properties_t* grp2 = obs_properties_create();
			obs_properties_add_group(grp, ST_I18N_RATECONTROL_LIMITS, D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS),
									 OBS_GROUP_NORMAL, grp2);

			{ // Bitrate
				auto p = obs_properties_add_int(grp2, ST_KEY_RATECONTROL_LIMITS_BITRATE,
												D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS_BITRATE), 0,
												std::numeric_limits<int32_t>::max(), 1);
				obs_property_int_set_suffix(p, " kbit/s");
			}

			{ // Bitrate Under/Overshoot
				auto p1 = obs_properties_add_int_slider(grp2, ST_KEY_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT,
														D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT), -1,
														100, 1);
				auto p2 = obs_properties_add_int_slider(grp2, ST_KEY_RATECONTROL_LIMITS_BITRATE_OVERSHOOT,
														D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS_BITRATE_OVERSHOOT), -1,
														1000, 1);
				obs_property_float_set_suffix(p1, " %");
				obs_property_float_set_suffix(p2, " %");
			}

#ifdef AOM_CTRL_AOME_SET_CQ_LEVEL
			{ // Quality
				auto p = obs_properties_add_int_slider(grp2, ST_KEY_RATECONTROL_LIMITS_QUALITY,
													   D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS_QUALITY), -1, 63, 1);
			}
#endif

			{ // Quantizer
				auto p1 =
					obs_properties_add_int_slider(grp2, ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MINIMUM,
												  D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS_QUANTIZER_MINIMUM), -1, 63, 1);
				auto p2 =
					obs_properties_add_int_slider(grp2, ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MAXIMUM,
												  D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS_QUANTIZER_MAXIMUM), -1, 63, 1);
			}
		}

		{ // Buffer
			obs_properties_t* grp2 = obs_properties_create();
			obs_properties_add_group(grp, ST_I18N_RATECONTROL_BUFFER, D_TRANSLATE(ST_I18N_RATECONTROL_BUFFER),
									 OBS_GROUP_NORMAL, grp2);

			{ // Buffer Size
				auto p = obs_properties_add_int(grp2, ST_KEY_RATECONTROL_BUFFER_SIZE,
												D_TRANSLATE(ST_I18N_RATECONTROL_BUFFER_SIZE), -1,
												std::numeric_limits<int32_t>::max(), 1);
				obs_property_int_set_suffix(p, " ms");
			}

			{ // Initial Buffer Size
				auto p = obs_properties_add_int(grp2, ST_KEY_RATECONTROL_BUFFER_SIZE_INITIAL,
												D_TRANSLATE(ST_I18N_RATECONTROL_BUFFER_SIZE_INITIAL), -1,
												std::numeric_limits<int32_t>::max(), 1);
				obs_property_int_set_suffix(p, " ms");
			}

			{ // Optimal Buffer Size
				auto p = obs_properties_add_int(grp2, ST_KEY_RATECONTROL_BUFFER_SIZE_OPTIMAL,
												D_TRANSLATE(ST_I18N_RATECONTROL_BUFFER_SIZE_OPTIMAL), -1,
												std::numeric_limits<int32_t>::max(), 1);
				obs_property_int_set_suffix(p, " ms");
			}
		}
	}

	{ // Key-Frame Options
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(props, ST_I18N_KEYFRAMES, D_TRANSLATE(ST_I18N_KEYFRAMES), OBS_GROUP_NORMAL, grp);

		{ // Key-Frame Interval Type
			auto p =
				obs_properties_add_list(grp, ST_KEY_KEYFRAMES_INTERVALTYPE, D_TRANSLATE(ST_I18N_KEYFRAMES_INTERVALTYPE),
										OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_set_modified_callback(p, modified_keyframes);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_KEYFRAMES_INTERVALTYPE_SECONDS), 0);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_KEYFRAMES_INTERVALTYPE_FRAMES), 1);
		}

		{ // Key-Frame Interval Seconds
			auto p = obs_properties_add_float(grp, ST_KEY_KEYFRAMES_INTERVAL_SECONDS,
											  D_TRANSLATE(ST_I18N_KEYFRAMES_INTERVAL), 0.00,
											  std::numeric_limits<uint16_t>::max(), 0.01);
			obs_property_float_set_suffix(p, " seconds");
		}

		{ // Key-Frame Interval Frames
			auto p =
				obs_properties_add_int(grp, ST_KEY_KEYFRAMES_INTERVAL_FRAMES, D_TRANSLATE(ST_I18N_KEYFRAMES_INTERVAL),
									   0, std::numeric_limits<int32_t>::max(), 1);
			obs_property_int_set_suffix(p, " frames");
		}
	}

	{ // Advanced Options
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(props, ST_I18N_ADVANCED, D_TRANSLATE(ST_I18N_ADVANCED), OBS_GROUP_NORMAL, grp);

		{ // Threads
			auto p = obs_properties_add_int(grp, ST_KEY_ADVANCED_THREADS, D_TRANSLATE(ST_I18N_ADVANCED_THREADS), 0,
											std::numeric_limits<int32_t>::max(), 1);
		}

#ifdef AOM_CTRL_AV1E_SET_ROW_MT
		{ // Row-MT
			auto p = streamfx::util::obs_properties_add_tristate(grp, ST_KEY_ADVANCED_ROWMULTITHREADING,
																 D_TRANSLATE(ST_I18N_ADVANCED_ROWMULTITHREADING));
		}
#endif

#ifdef AOM_CTRL_AV1E_SET_TILE_COLUMNS
		{ // Tile Columns
			auto p = obs_properties_add_int_slider(grp, ST_KEY_ADVANCED_TILE_COLUMNS,
												   D_TRANSLATE(ST_I18N_ADVANCED_TILE_COLUMNS), -1, 6, 1);
		}
#endif

#ifdef AOM_CTRL_AV1E_SET_TILE_ROWS
		{ // Tile Rows
			auto p = obs_properties_add_int_slider(grp, ST_KEY_ADVANCED_TILE_ROWS,
												   D_TRANSLATE(ST_I18N_ADVANCED_TILE_ROWS), -1, 6, 1);
		}
#endif
	}

	return props;
}

#ifdef ENABLE_FRONTEND
bool aom_av1_factory::on_manual_open(obs_properties_t* props, obs_property_t* property, void* data)
{
	streamfx::open_url(HELP_URL);
	return false;
}
#endif
