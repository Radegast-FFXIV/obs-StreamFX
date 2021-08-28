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

#pragma once
#include "common.hpp"
#include <memory>
#include <queue>
#include "obs/obs-encoder-factory.hpp"
#include "util/util-library.hpp"

//namespace libaom {
#include <aom/aomcx.h>
//}

namespace streamfx::encoder::aom::av1 {
	class aom_av1_factory;

	class aom_av1_instance : public obs::encoder_instance {
		std::shared_ptr<aom_av1_factory> _factory;

		aom_codec_iface_t*  _iface;
		aom_codec_ctx_t     _ctx;
		aom_codec_enc_cfg_t _cfg;

		aom_img_fmt                              _image_format;
		std::queue<std::shared_ptr<aom_image_t>> _images;

		aom_fixed_buf_t* _global_headers;

		public:
		aom_av1_instance(obs_data_t* settings, obs_encoder_t* self, bool is_hw);
		virtual ~aom_av1_instance();

		virtual void migrate(obs_data_t* settings, uint64_t version);

		virtual bool update(obs_data_t* settings);

		virtual bool get_extra_data(uint8_t** extra_data, size_t* size);

		virtual bool get_sei_data(uint8_t** sei_data, size_t* size);

		virtual void get_video_info(struct video_scale_info* info);

		virtual bool encode_video(encoder_frame* frame, encoder_packet* packet, bool* received_packet);
	};

	class aom_av1_factory : public obs::encoder_factory<aom_av1_factory, aom_av1_instance> {
		std::shared_ptr<::streamfx::util::library> _library;

		public:
		aom_av1_factory();
		~aom_av1_factory();

		const char* get_name() override;

		void* create(obs_data_t* settings, obs_encoder_t* encoder, bool is_hw) override;

		void get_defaults2(obs_data_t* data) override;

		obs_properties_t* get_properties2(instance_t* data) override;

#ifdef ENABLE_FRONTEND
		static bool on_manual_open(obs_properties_t* props, obs_property_t* property, void* data);
#endif

		public:
		// aom_codec.h
		decltype(&aom_codec_version)           libaom_codec_version;
		decltype(&aom_codec_version_str)       libaom_codec_version_str;
		decltype(&aom_codec_version_extra_str) libaom_codec_version_extra_str;
		decltype(&aom_codec_build_config)      libaom_codec_build_config;
		decltype(&aom_codec_iface_name)        libaom_codec_iface_name;
		decltype(&aom_codec_err_to_string)     libaom_codec_err_to_string;
		decltype(&aom_codec_error)             libaom_codec_error;
		decltype(&aom_codec_error_detail)      libaom_codec_error_detail;
		decltype(&aom_codec_destroy)           libaom_codec_destroy;
		decltype(&aom_codec_get_caps)          libaom_codec_get_caps;
		decltype(&aom_codec_control) libaom_codec_control;
		decltype(&aom_codec_set_option)   libaom_codec_set_option;
		decltype(&aom_obu_type_to_string) libaom_obu_type_to_string;

		// aom_integer.h
		decltype(&aom_uleb_size_in_bytes)     libaom_uleb_size_in_bytes;
		decltype(&aom_uleb_decode)            libaom_uleb_decode;
		decltype(&aom_uleb_encode)            libaom_uleb_encode;
		decltype(&aom_uleb_encode_fixed_size) libaom_uleb_encode_fixed_size;

		// aom_image.h
		decltype(&aom_img_alloc)             libaom_img_alloc;
		decltype(&aom_img_wrap)              libaom_img_wrap;
		decltype(&aom_img_alloc_with_border) libaom_img_alloc_with_border;
		decltype(&aom_img_set_rect)          libaom_img_set_rect;
		decltype(&aom_img_flip)              libaom_img_flip;
		decltype(&aom_img_free)              libaom_img_free;
		decltype(&aom_img_plane_width)       libaom_img_plane_width;
		decltype(&aom_img_plane_height)      libaom_img_plane_height;
		decltype(&aom_img_add_metadata)      libaom_img_add_metadata;
		decltype(&aom_img_get_metadata)      libaom_img_get_metadata;
		decltype(&aom_img_num_metadata)      libaom_img_num_metadata;
		decltype(&aom_img_remove_metadata)   libaom_img_remove_metadata;
		decltype(&aom_img_metadata_alloc)    libaom_img_metadata_alloc;
		decltype(&aom_img_metadata_free)     libaom_img_metadata_free;

		// aom_encoder.h
		decltype(&aom_codec_enc_init_ver)       libaom_codec_enc_init_ver;
		decltype(&aom_codec_enc_config_default) libaom_codec_enc_config_default;
		decltype(&aom_codec_enc_config_set)     libaom_codec_enc_config_set;
		decltype(&aom_codec_get_global_headers) libaom_codec_get_global_headers;
		decltype(&aom_codec_encode)             libaom_codec_encode;
		decltype(&aom_codec_set_cx_data_buf)    libaom_codec_set_cx_data_buf;
		decltype(&aom_codec_get_cx_data)        libaom_codec_get_cx_data;
		decltype(&aom_codec_get_preview_frame)  libaom_codec_get_preview_frame;

		// aomcx.h
		decltype(&aom_codec_av1_cx) libaom_codec_av1_cx;

		public: // Singleton
		static void initialize();

		static void finalize();

		static std::shared_ptr<aom_av1_factory> get();
	};
} // namespace streamfx::encoder::aom::av1
