
/*
* File Name: ismd_audio_defs.h
*/

/* 

  This file is provided under a dual BSD/GPLv2 license.  When using or 
  redistributing this file, you may do so under either license.

  GPL LICENSE SUMMARY

  Copyright(c) 2006-2011 Intel Corporation. All rights reserved.

  This program is free software; you can redistribute it and/or modify 
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
  The full GNU General Public License is included in this distribution 
  in the file called LICENSE.GPL.

  Contact Information:

  Intel Corporation
  2200 Mission College Blvd.
  Santa Clara, CA  97052

  BSD LICENSE 

  Copyright(c) 2006-2011 Intel Corporation. All rights reserved.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without 
  modification, are permitted provided that the following conditions 
  are met:

    * Redistributions of source code must retain the above copyright 
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright 
      notice, this list of conditions and the following disclaimer in 
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Intel Corporation nor the names of its 
      contributors may be used to endorse or promote products derived 
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


/** \addtogroup ismd_audio 
@{ */

#ifndef _ISMD_AUDIO_DEFS_H_
#define _ISMD_AUDIO_DEFS_H_

#include "ismd_audio_type_defs.h"
#include "ismd_audio_srs.h"
#include "ismd_audio_dts_neural.h"

#ifdef __cplusplus
extern "C" {
#endif

/**Maximum number of Audio Processors supported*/
#define AUDIO_MAX_PROCESSORS 2

/** AUDIO_MAX_INPUTS:
  * 1 - primary stream - (decode)
  * 1 - secondary stream - (decode)
  * 1 - capture input stream (PCM)
  * 8 - Seperate effects streams. (PCM) */
#define AUDIO_MAX_INPUTS 11
/** The maximum number of outputs supported*/
#define AUDIO_MAX_OUTPUTS 6

/** The maximum amount of input channels supported by the Audio subsystem.*/
#define AUDIO_MAX_INPUT_CHANNELS 8
/** The maximum amount of output channels supported by the Audio subsystem.*/
#define AUDIO_MAX_OUTPUT_CHANNELS 8

/** The maximum delay in milliseconds supported by the \ref ismd_audio_output_set_delay function.*/
#define AUDIO_MAX_OUTPUT_STREAM_DELAY 256

/** The maximum per channel delay allowed in units of 0.1ms. See \ref ismd_audio_set_per_channel_delay. */
#define ISMD_AUDIO_PER_CHANNEL_MAX_DELAY_MS 830
/** The minimum per channel delay allowed in units of 0.1ms. See \ref ismd_audio_set_per_channel_delay. */
#define ISMD_AUDIO_PER_CHANNEL_MIN_DELAY_MS 0

#define ISMD_AUDIO_INVALID_LEVEL_DB -10000

/**Used to specify an invalid handle used in the Audio module.*/
#define AUDIO_INVALID_HANDLE -1

/** Invalid channel configuration. No channels are present.*/
#define AUDIO_CHAN_CONFIG_INVALID   0xFFFFFFFF
/** Eample of a common mono channel config. This define specifies Left only. See \ref ismd_audio_channel_t*/
#define AUDIO_CHAN_CONFIG_1_CH      0xFFFFFFF0 
/** Eample of a common stereo channel config. See \ref ismd_audio_channel_t*/
#define AUDIO_CHAN_CONFIG_2_CH      0xFFFFFF20       
/** Eample of a common 5.1 channel config. See \ref ismd_audio_channel_t*/
#define AUDIO_CHAN_CONFIG_6_CH      0xFF743210
/** Eample of a common 7.1 channel config. See \ref ismd_audio_channel_t*/
#define AUDIO_CHAN_CONFIG_8_CH      0x76543210

/** Handle used to control the default mixer.*/
#define ISMD_AUDIO_DEFAULT_MIXER_HANDLE 0

/** Maximum number of mixers supported*/
#define ISMD_AUDIO_MAX_MIXERS 2


/** Use these values to specify a channel in channel configurations.
   4bits of a 32bit integer are used to specify a channel configuration.
   See \ref AUDIO_CHAN_CONFIG_8_CH for an example channel configuration.
   These values are used while specifying a channel configuration of an PCM input
   while calling the \ref ismd_audio_input_set_pcm_format or while filling out the 
   ch_map member of \ref ismd_audio_output_config_t.
*/
typedef enum { //Do NOT change the ordering of this enum.
   ISMD_AUDIO_CHANNEL_LEFT = 0,
   ISMD_AUDIO_CHANNEL_CENTER , // 1
   ISMD_AUDIO_CHANNEL_RIGHT , // 2
   ISMD_AUDIO_CHANNEL_LEFT_SUR , // 3
   ISMD_AUDIO_CHANNEL_RIGHT_SUR , // 4
   ISMD_AUDIO_CHANNEL_LEFT_REAR_SUR , // 5
   ISMD_AUDIO_CHANNEL_RIGHT_REAR_SUR, // 6   
   ISMD_AUDIO_CHANNEL_LFE , // 7
   ISMD_AUDIO_CHANNEL_INVALID = 0xF,
} ismd_audio_channel_t;

/** Value to specify for hardware output for I2S0 (stereo output)*/
#define ISMD_AUDIO_HW_OUTPUT_I2S0 0x3D1
/** Value to specify for hardware output for I2S1 (multichannel output) */
#define ISMD_AUDIO_HW_OUTPUT_I2S1 0x3D2
/** Value to specify for hardware output for SPDIF */
#define ISMD_AUDIO_HW_OUTPUT_SPDIF 0x3D3
/** Value to specify for hardware output for HDMI */
#define ISMD_AUDIO_HW_OUTPUT_HDMI 0x3D4
/** Value to specify for hardware output for I2S0 internal ADAC */
#define ISMD_AUDIO_HW_OUTPUT_ADAC0 0x3D5


/** Value to specify for hardware input device I2S0. (Stereo Capture) */
#define ISMD_AUDIO_HW_INPUT_I2S0 0x4D1
/** Value to specify for hardware input device I2S1. (Multichannel Capture) */
#define ISMD_AUDIO_HW_INPUT_I2S1 0x4D2
/** Value to specify for hardware input device SPDIF */
#define ISMD_AUDIO_HW_INPUT_SPDIF 0x4D3
/** Value to specify for hardware input device HDMI */
#define ISMD_AUDIO_HW_INPUT_HDMI 0x4D4


/** Specify this sample rate value for on an audio output port (file out) to
 * indicate no sample rate conversion is to be applied to the input streams.
 * This is useful if you have a file output port in which you want no sample
 * rate conversion to happen. Not reccomended for hardware outputs.
 */
#define ISMD_AUDIO_OUTPUT_SAMPLE_RATE_NO_CONVERT 0xABCD


/** Handle used to access/configure an audio processor.*/
typedef int ismd_audio_processor_t;

/** Output handle used to access/configure an audio output.*/
typedef int ismd_audio_output_t;

/** Mixer handle type to access/ configure an audio mixer.  */
typedef int ismd_audio_mixer_t; 

/** The list of possible audio stream formats supported. Ex. use these values in \ref ismd_audio_set_data_format.*/
typedef enum {
   ISMD_AUDIO_MEDIA_FMT_INVALID,  /**< - Invalid or Unknown algorithm*/
   ISMD_AUDIO_MEDIA_FMT_PCM , /**< - stream is raw linear PCM data*/
   ISMD_AUDIO_MEDIA_FMT_DVD_PCM , /**< - stream is linear PCM data coming from a DVD */
   ISMD_AUDIO_MEDIA_FMT_BLURAY_PCM , /**< - stream is linear PCM data coming from a BD (HDMV Audio) */
   ISMD_AUDIO_MEDIA_FMT_MPEG , /**< - stream uses MPEG-1 or MPEG-2 BC algorithm*/
   ISMD_AUDIO_MEDIA_FMT_AAC , /**< - stream uses MPEG-2 or MPEG-4 AAC algorithm*/
   ISMD_AUDIO_MEDIA_FMT_AAC_LOAS , /**< - stream uses MPEG-2 or MPEG-4 AAC algorithm with LOAS header format*/
   ISMD_AUDIO_MEDIA_FMT_DD , /**< - stream uses Dolby Digital (AC3) algorithm*/
   ISMD_AUDIO_MEDIA_FMT_DD_PLUS , /**< - stream uses Dolby Digital Plus (E-AC3) algorithm*/
   ISMD_AUDIO_MEDIA_FMT_TRUE_HD , /**< - stream uses Dolby TrueHD algorithm*/
   ISMD_AUDIO_MEDIA_FMT_DTS_HD , /**< - stream uses DTS High Definition audio algorithm */
   ISMD_AUDIO_MEDIA_FMT_DTS_HD_HRA , /**< - DTS-HD High Resolution Audio */
   ISMD_AUDIO_MEDIA_FMT_DTS_HD_MA , /**< - DTS-HD Master Audio */
   ISMD_AUDIO_MEDIA_FMT_DTS , /**< - stream uses DTS  algorithm*/
   ISMD_AUDIO_MEDIA_FMT_DTS_LBR , /**< - stream uses DTS Low BitRate decode algorithm*/
   ISMD_AUDIO_MEDIA_FMT_DTS_BC ,  /**< - stream uses DTS broadcast decode algorithm*/
   ISMD_AUDIO_MEDIA_FMT_WM9 , /**< - stream uses Windows Media 9 algorithm*/
   ISMD_AUDIO_MEDIA_FMT_DDC,  /**< - stream uses DD/DDP algorithm and decoded by DDC decoder*/
   ISMD_AUDIO_MEDIA_FMT_DDT,/**< - stream uses AAC(up to AAC-HE V2)algorithm and decoded by DDT decoder*/
   ISMD_AUDIO_MEDIA_FMT_DRA ,  /**< - stream uses Digital Rise Audio algorithm*/
   ISMD_AUDIO_MEDIA_FMT_COUNT /**< - Count of enum members.*/
} ismd_audio_format_t;
typedef enum {
   ISMD_AUDIO_ENCODE_FMT_AC3 = ISMD_AUDIO_MEDIA_FMT_COUNT,
   ISMD_AUDIO_ENCODE_FMT_DTS,
   ISMD_AUDIO_ENCODE_FMT_TRUEHD_MAT,
   ISMD_AUDIO_ENCODE_FMT_AAC,
   ISMD_AUDIO_ENCODE_FMT_END
} ismd_audio_encode_format_t;

/**  Defines a list of possible sub formats, this is not limited for the aac, can add dts later */
typedef enum {
    ISMD_AUDIO_SUB_FMT_DEFAULT = 0,
	ISMD_AUDIO_SUB_FMT_HE_AAC =  1
} ismd_audio_sub_format_t;

typedef enum {
   ISMD_AUDIO_MATRIX_DECODER_DTS_NEO6 = ISMD_AUDIO_ENCODE_FMT_END,
   ISMD_AUDIO_MATRIX_DECODER_END
} ismd_audio_matrix_decoder_t;

/** Use these values to specify the type of specialized post 
    processing to be applied to the audio output. See the 
    output specific function \ref ismd_audio_output_enable_specialized_processing */
typedef enum {
   ISMD_AUDIO_SPECIALIZED_PROCESSING_SRS = ISMD_AUDIO_MATRIX_DECODER_END,/** SRS specific audio post processing */
   ISMD_AUDIO_SPECIALIZED_PROCESSING_DTS_NEURAL,                         /** DTS Neural up-mix processing.*/
   ISMD_AUDIO_SPECIALIZED_PROCESSING_END
}ismd_audio_specialized_processing_t;

#define ISMD_AUDIO_WM_TYPE_INVALID -1 /** Invalid watermark*/
/** Enumeration of all supported watermark technologies in audio subsystem.*/
typedef enum {
   ISMD_AUDIO_WM_TYPE_CINAVIA = ISMD_AUDIO_SPECIALIZED_PROCESSING_END, /**  Cinavia watermark technology*/
   ISMD_AUDIO_WM_TYPE_MOONHILL,     /**  Moonhill watermark technology*/
   ISMD_AUDIO_WM_TYPE_END
} ismd_audio_wm_type_t;

/** The max number of possible processing libraries installed in the audio processor.*/
#define ISMD_AUDIO_MAX_LIBS ISMD_AUDIO_WM_TYPE_END

/** Use this to specify configuration parameters for specialized
    audio post processing algorithms.  See \ref ismd_audio_output_enable_specialized_processing */
typedef union {
   ismd_audio_srs_pipe_params_t srs;
   ismd_audio_dts_neural_params_t dts_neural;
} ismd_audio_specialized_processing_params_t;

/** Use these values when calling /ref ismd_audio_set_latency_mode */
typedef enum {
   ISMD_AUDIO_LATENCY_MODE_DEFAULT,                   /** Default latency mode. All use cases supported.*/
   ISMD_AUDIO_LATENCY_MODE_LOW_WITH_POST_PROCESSING   /** Low latency mode with post processing enabled, no encode or HDMI output support.*/
}ismd_audio_latency_mode_t;


/** Define to list all supported formats of the user's specific implementation. 
   This could be useful if you did not want to track this information in \ref ismd_audio_input_pass_through_config_t 
*/
typedef ismd_audio_format_t ismd_audio_suported_formats_t[ISMD_AUDIO_MEDIA_FMT_COUNT];

/** \deprecated Defines a list of possible aac header formats. */
typedef enum {
    ISMD_AUDIO_AAC_FMT_UNKNOWN = 0,
    ISMD_AUDIO_AAC_FMT_M4MUX = 1,
    ISMD_AUDIO_AAC_FMT_LATM = 2,
    ISMD_AUDIO_AAC_FMT_ADIF = 3,
    ISMD_AUDIO_AAC_FMT_MP4FF = 4,
    ISMD_AUDIO_AAC_FMT_ADTS = 5,
    ISMD_AUDIO_AAC_FMT_LOAS = 6
} ismd_audio_aac_format_t;

/**  \deprecated Defines a list of possible MPEG audio format.*/
typedef enum {
    ISMD_AUDIO_MPEG_LAYER_UNKNOWN = 0,
    ISMD_AUDIO_MPEG1_LAYER_LII = 1,
    ISMD_AUDIO_MPEG1_LAYER_LIII = 2,
    ISMD_AUDIO_MPEG2_LAYER_LII = 3,
} ismd_audio_mpeg_layer_t;

/**
Use these values to specify the channel configuration of the output. See \ref ismd_audio_output_set_channel_config.
Note: This does not specify the actual ordering of the output channels. 
*/
typedef enum {
   ISMD_AUDIO_CHAN_CONFIG_INVALID,
   ISMD_AUDIO_STEREO , /**< Right and Left channel data*/
   ISMD_AUDIO_DUAL_MONO , /**< Right channel data*/
   ISMD_AUDIO_5_1 , /**< 6 channel data*/
   ISMD_AUDIO_7_1 , /**< 8 channel data*/
} ismd_audio_channel_config_t;

/** Defines the bass management setting applicable to each channel in a stream. */
typedef enum {
   ISMD_AUDIO_SPEAKER_INVALID , /**< Invalid value*/
   ISMD_AUDIO_SPEAKER_LARGE , /**< Speaker is large (contains all frequencies)*/
   ISMD_AUDIO_SPEAKER_SMALL   /**< Speaker is small (contains no frequencies below the specified crossover freq.) */
} ismd_audio_speaker_setting_t;


/** List of possible crossover frequencies used to specify in the function \ref ismd_audio_set_bass_management. */
typedef enum {
   ISMD_AUDIO_CROSSOVER_FREQ_INVALID,
   ISMD_AUDIO_CROSSOVER_FREQ_40HZ,
   ISMD_AUDIO_CROSSOVER_FREQ_60HZ,
   ISMD_AUDIO_CROSSOVER_FREQ_80HZ,
   ISMD_AUDIO_CROSSOVER_FREQ_90HZ,
   ISMD_AUDIO_CROSSOVER_FREQ_100HZ,
   ISMD_AUDIO_CROSSOVER_FREQ_110HZ,
   ISMD_AUDIO_CROSSOVER_FREQ_120HZ,
   ISMD_AUDIO_CROSSOVER_FREQ_160HZ,
   ISMD_AUDIO_CROSSOVER_FREQ_200HZ,
} ismd_audio_crossover_freq_t;

/** \deprecated */
typedef enum {
   ISMD_AUDIO_HW_INPUT,
   ISMD_AUDIO_SW_INPUT,
   ISMD_AUDIO_NULL_INPUT
} ismd_audio_input_type_t;

/** This enumeration specifies the mode the output will operate in. Used in \ref ismd_audio_output_set_mode. */
typedef enum {
   ISMD_AUDIO_OUTPUT_INVALID,
   ISMD_AUDIO_OUTPUT_PCM,                   /**< uncompressed, PCM samples */
   ISMD_AUDIO_OUTPUT_PASSTHROUGH,           /**< output without mixing, minimal modification */
   ISMD_AUDIO_OUTPUT_ENCODED_DOLBY_DIGITAL, /**< encoded DD output  */
   ISMD_AUDIO_OUTPUT_ENCODED_DTS,           /**< encoded DTS output */
   ISMD_AUDIO_OUTPUT_ENCODED_AAC            /**< encoded AAC output */
} ismd_audio_output_mode_t;


/** Specifies the use of a internal or external audio clock used to drive the hardware audio outputs. 
  Used in \ref ismd_audio_configure_master_clock. */
typedef enum {
   ISMD_AUDIO_CLK_SRC_INTERNAL,
   ISMD_AUDIO_CLK_SRC_EXTERNAL
}ismd_audio_clk_src_t;

/** Specifies the use of a A/V locked or independent audio clock used to drive the hardware audio outputs. 
  Used in \ref ismd_audio_set_clock_mode. */
typedef enum {
   ISMD_AUDIO_CLK_LOCKED,
   ISMD_AUDIO_CLK_INDEPENDENT
}ismd_audio_clk_mode_t;

/** Mixer setup information.*/
typedef struct{
   /** Mixer instance handle, AUDIO_INVALID_HANDLE specified if not valid.*/
   ismd_audio_mixer_t mixer_handle;
   /** List of output handles associated to this mixer, if AUDIO_INVALID_HANDLE is
       is specified, it is not a valid output. */
   ismd_audio_output_t associated_outputs[AUDIO_MAX_OUTPUTS];
}ismd_audio_mixer_setup_info_t;

/** Information on the how mixers are configured for a given processor.*/
typedef struct{
   ismd_audio_mixer_setup_info_t mixers[ISMD_AUDIO_MAX_MIXERS];
}ismd_audio_processor_mixer_setup_info_t;

/** Structure to contain stream position information when calling \ref ismd_audio_input_get_stream_position. */
typedef struct {

   // Base time currently set in the renderer.
   audio_uint64_t base_time;

   // Current time when stream position is queried.
   audio_uint64_t current_time;

   // The current segment time
   audio_uint64_t segment_time;
   
   // The current unscaled local time.
   audio_uint64_t linear_time;
   
   // The current scaled local time.
   audio_uint64_t scaled_time;

   // The current number of samples sent to render
   audio_uint64_t sample_count;
   
   // Current stream position between zero and the duration of the clip
   // ISMD_NO_PTS will be returned if the driver does not have enough information
   // to calculate this field 
   audio_uint64_t stream_position;
   
} ismd_audio_stream_position_info_t;

/** Used as a max limit in \ref ismd_audio_decoder_param_t*/
#define MAX_CODEC_PARAM_SIZE 144

/** Decoder parameter container.*/
typedef struct {
   uint8_t pad[MAX_CODEC_PARAM_SIZE];
} ismd_audio_decoder_param_t;

/** Use this structure to specify a delay value for each audio channel. */
typedef struct {
   int front_left;
   int front_right;
   int center;
   int lfe;
   int surround_left;
   int surround_right;
   int rear_surround_left;
   int rear_surround_right;
} ismd_audio_per_channel_delay_t;


/** \deprecated Do not use! */
#define AUDIO_GAIN_MINUS_INFINITY -1450 

/** Maximum gain value of 18.0dB */
#define ISMD_AUDIO_GAIN_MAX   180
/* for per channel output vol, master per output vol, 
  both of these in the quality pipeline, max gain is 30.0dB */
#define ISMD_AUDIO_VOL_CTL_GAIN_MAX 300
/** No gain 0.0dB */
#define ISMD_AUDIO_GAIN_0_DB  0
/** Minimum gain of -145.0dB */
#define ISMD_AUDIO_GAIN_MUTE  (-1450) 

/** Min ramp value in ms used when ramping up/down from a certain level.*/
#define ISMD_AUDIO_RAMP_MS_MIN 0
/** MAX ramp value in ms used when ramping up/down from a certain level.*/
#define ISMD_AUDIO_RAMP_MS_MAX 20000


/** Use this structure to specify volume levels of each audio channel.*/
typedef struct {
   ismd_audio_gain_value_t front_left;
   ismd_audio_gain_value_t front_right;
   ismd_audio_gain_value_t center;
   ismd_audio_gain_value_t lfe;
   ismd_audio_gain_value_t surround_left;
   ismd_audio_gain_value_t surround_right;
   ismd_audio_gain_value_t rear_surround_left;
   ismd_audio_gain_value_t rear_surround_right;
} ismd_audio_per_channel_volume_t;

/**
Use this structure to define the speaker setting (Small or Large)
of each individual channel/speaker of the audio stream. LFE not included.
*/
typedef struct {
   ismd_audio_speaker_setting_t front_left;
   ismd_audio_speaker_setting_t front_right;
   ismd_audio_speaker_setting_t center;
   ismd_audio_speaker_setting_t surround_left;
   ismd_audio_speaker_setting_t surround_right;
   ismd_audio_speaker_setting_t rear_surround_left;
   ismd_audio_speaker_setting_t rear_surround_right;
}ismd_audio_per_speaker_setting_t;

/**
Pass through mode configuration structure. Use this to indicate that the input is in 
pass through mode and use supported_algos to list the supported encoding algorithms
to accept as pass through streams. 
*/
typedef struct {
   bool is_pass_through; /**< Flag to indicate whether or not the input is in pass through mode, if false, all other members in this struct will be ignored.*/
   bool dts_or_dolby_convert; /**< Flag to tell if conversion is needed for DD+ -> AC3-5.1. Will support DTS-HD -> DTS-5.1 in the future.*/
   ismd_audio_format_t supported_formats[ISMD_AUDIO_MEDIA_FMT_COUNT]; /**< List of supported audio formats allowed to passthrough at this input.*/
   int supported_format_count;  /**< Number of elements in the supported_fmts array member in this structure.*/
}ismd_audio_input_pass_through_config_t;

/**
Use these values to specify how the passthrough stream will be 
prepared when it is output. See \ref ismd_audio_input_set_pass_through_mode
*/
typedef enum{
   ISMD_AUDIO_PASS_THROUGH_MODE_INVALID,   /** Invalid passthrough mode.*/
   ISMD_AUDIO_PASS_THROUGH_MODE_DIRECT,    /** Pass through input will be output as is, no formatting.*/
   ISMD_AUDIO_PASS_THROUGH_MODE_IEC61937   /** Pass through input will be output formatted to the IEC61937 specification. (default)*/
}ismd_audio_pass_through_mode_t;

/**
   Specifies how an input channel will be mixed into the various output channels.
*/
typedef struct {
	int input_channel_id; /**< ID of the input channel being mapped */
	ismd_audio_gain_value_t output_channels_gain[AUDIO_MAX_OUTPUT_CHANNELS]; /**< Array indicating the relative gain to be included in each output channel. */
} ismd_audio_channel_mapping_t;

/** Specifies a complete mapping of input channels to output channels. */
typedef struct{
   ismd_audio_channel_mapping_t input_channels_map[AUDIO_MAX_INPUT_CHANNELS]; /**< Each entry in the array controls the mixing of a single input channel. */
} ismd_audio_channel_mix_config_t;

/**
   Output configuration parameters.
*/
typedef struct{
   int stream_delay; /**< Set the delay in miliseconds of output stream. Range 0-255 ms (1 ms step).*/
   int sample_size; /**< Set the output audio sample size, i.e. the bitwidth.  Valid values are 16, 20, 24, and 32.  
                        32-bit samples are equivalent to unpacked 24-bit and 20-bit are supported only for HDMI outputs. */
   ismd_audio_channel_config_t ch_config; /**< Set the output channel configuration*/
   ismd_audio_output_mode_t out_mode; /**< Set the output mode. (i.e. ISMD_AUDIO_OUTPUT_PCM or ISMD_AUDIO_OUTPUT_PASSTHROUGH) */
   int sample_rate; /**< Set the output stream's sample rate.*/
   int ch_map; /**<Set the output channel mapping (applicable in SW output only), 0 = use default driver channel mapping (disabled)*/
}ismd_audio_output_config_t;


/** This struct defines the stream information */
typedef struct {
   int bitrate;
   int sample_rate;
   int sample_size;
   int channel_config;
   int channel_count;
   ismd_audio_format_t algo;
}ismd_audio_stream_info_t;

/**
Use this structure to notify the WMA decoder (if avaialble), 
of the stream information given from an ASF parser. See
\ref ismd_audio_input_set_wma_format
*/
typedef struct {
   int sample_rate;  /**< Sample rate of the PCM data encoded in this WMA stream */
   int num_channels;  /**< Number of channels in the encoded PCM data in this WMA stream */
   int format_tag;  /**< Specifies which flavor of WMA the input stream contains.  Currently only WMA Standard v2 (0x161) is supported. */  
   int sample_size; /**< Sample size of the encoded PCM data in this WMA stream */
   int block_align; /**< Set the block size to be used by the WMA decoder.  Valid range is 1 to 65535.  */
   int encode_option; /**< Set the encode options that were used when encoding the stream.  Valid range is 0 to 63. */
   int bitrate; /**< Average bytes per second of the input stream.  Valid values are zero and positive values.  Zero configures the decoder for a low-bit rate input stream. */
}ismd_audio_wma_format_t;


/**
 This is a list if the supported asynchronous notifications that an 
 application may receive from the audio subsystem.
 */
typedef enum {
 //ISMD_AUDIO_NOTIFY_INVALID               =  0, /**< Invalid notification type, don't use. */
   ISMD_AUDIO_NOTIFY_STREAM_BEGIN          =  1, /**< Begin of stream detected. */
   ISMD_AUDIO_NOTIFY_STREAM_END            =  2, /**< End of stream detected. */
   ISMD_AUDIO_NOTIFY_RENDER_UNDERRUN       =  3, /**< Renderer underrun detected. */
   ISMD_AUDIO_NOTIFY_WATERMARK             =  4, /**< Watermark detectect in stream. */
   ISMD_AUDIO_NOTIFY_TAG_RECEIVED          =  5, /**< A buffer was received with an application-specific tag. */
   ISMD_AUDIO_NOTIFY_SAMPLE_RATE_CHANGE    =  6, /**< Sample rate changed in-band at the input. */
   ISMD_AUDIO_NOTIFY_CODEC_CHANGE          =  7, /**< Codec in-band at the input. */
   ISMD_AUDIO_NOTIFY_SAMPLE_SIZE_CHANGE    =  8, /**< Sample size changed in-band at the input. */
   ISMD_AUDIO_NOTIFY_CHANNEL_CONFIG_CHANGE =  9, /**< Channel configuration changed in-band at the input. */
   ISMD_AUDIO_NOTIFY_PTS_VALUE_EARLY       = 10, /**< Received packet very early. */
   ISMD_AUDIO_NOTIFY_PTS_VALUE_LATE        = 11, /**< Received packet late. */
   ISMD_AUDIO_NOTIFY_PTS_VALUE_RECOVERED   = 12, /**< Received packet renderring normal again after PTS_LATE */
   ISMD_AUDIO_NOTIFY_CORRUPT_FRAME         = 13, /**< Received corrupted packet at input. */
   ISMD_AUDIO_CAPTURE_OVERRUN              = 14, /**< Audio capture input overrun. */
   ISMD_AUDIO_NOTIFY_CLIENT_ID             = 15, /**< CLIENT ID TAG */
   ISMD_AUDIO_NOTIFY_SEGMENT_END           = 16, /**< Reveived a PTS value beyond the stop value of a segment. */
   ISMD_AUDIO_NOTIFY_SRC_STATUS_CHANGE     = 17, /**< Notifies when the sample rate converter status changes from 
                                                      supporting the rate conversion to not supporting the conversion 
                                                      and the opposite. Use \ref ismd_audio_input_get_status to get 
                                                      the status of the sample rate converter.*/
   ISMD_AUDIO_NOTIFY_DECODE_ERROR          = 18, /**< Codec failure or CRC error */
   ISMD_AUDIO_NOTIFY_DECODE_SYNC_FOUND     = 19, /**< Codec found SYNCWORD */
   ISMD_AUDIO_NOTIFY_DECODE_SYNC_LOST      = 20, /**< Codec lost SYNCWORD */
   ISMD_AUDIO_NOTIFY_INPUT_FULL            = 21, /**< Input port full */
   ISMD_AUDIO_NOTIFY_INPUT_EMPTY           = 22, /**< Input port empty */
   ISMD_AUDIO_NOTIFY_INPUT_RECOVERED       = 23, /**< Input receive buffer again after EMPTY */
   ISMD_AUDIO_NOTIFY_SEGMENT_START         = 24, /**< Received the first PTS value fitting into the start and stop values of a segment. */
   ISMD_AUDIO_NOTIFY_DATA_RENDERED         = 25, /**< Notify that actual audio data for the given input is being rendered. This
                                                      event will be signaled at the rate of the processing time quantam internal to 
                                                      the audio subsystem, which is normally no lower than 10ms.*/
   ISMD_AUDIO_NOTIFY_PARSER_STATUS_CHANGE  = 26, /**< Notify that the input parser has detected new stream information or has changed status.*/
   ISMD_AUDIO_NOTIFY_TIMING_INFO_UNAVAILABLE  = 27, /**< Notify that the input stream has detected timing information no longer available */
   ISMD_AUDIO_NOTIFY_COUNT,                      /**< Event count.*/
   ISMD_AUDIO_NOTIFY_MULTIPLE_EVENTS /**< Special event type. It is only used for firing 
                                          multiple notification events from DSP to host.
                                          Please refer to \ref audio_dsp_pvt_interrupt_handler. */
} ismd_audio_notification_t;

typedef enum {
   ISMD_AUDIO_PROCESSOR_NOTIFY_INVALID         =  -1,/**< Invalid notification type. */
   ISMD_AUDIO_PROCESSOR_NOTIFY_POST_PROCESSING_STARTED =   0,/**< Post processing pipe starts. */   
   ISMD_AUDIO_PROCESSOR_NOTIFY_COUNT,                /**< The number of global events we support now.*/
} ismd_audio_processor_notification_t;

/** Describes the processing needed on an audio buffer with a discontinuity. */
typedef enum {
   AUDIO_DISCONTINUITY_ACTION_NONE = 0,                           /** No action required.*/
   AUDIO_DISCONTINUITY_ACTION_RAMP_UP,                            /** To soften the discontinuity, ramp the audio up.*/
   AUDIO_DISCONTINUITY_ACTION_RAMP_DOWN,                          /** To soften the discontinuity, ramp the audio down.*/
   AUDIO_DISCONTINUITY_ACTION_RAMP_DOWN_PREV_UP_AT_OFFSET         /** Indicates the preceding buffer needs ramp down, 
                                                                      but also a ramp up is needed at the offset specified.*/
}audio_discontinuity_action_t;

/** Structure that describes a known discontinuity in a buffer.*/
typedef struct {
   uint16_t action;     /** The action that needs to happen at the discontinuity in 
                            the buffer, if it exits. See \ref audio_discontinuity_action_t */
   uint16_t offset;     /** The offset in bytes of where the discontinuity starts.*/
}audio_discont_desc_t;

/** Tags association metadata*/
typedef struct {
   int tag_buffer_id_start;/*First buffer that has tags assciated to this buffer*/
   int tag_buffer_id_end;/*Last buffer that has tags associated to this buffer.*/
} audio_tags_desc_t;

/**
   This structure is used to specify in-band buffer metadata attributes.  This data will 
   be in the "attributes" field of ismd_buffer_descriptor_t structures associated with 
   buffers read from an audio output port.
*/
typedef struct {
   audio_uint64_t       local_pts;           /** Scaled PTS value */
   audio_uint64_t       linear_pts;          /** Linear PTS value */
   audio_uint64_t       original_pts;        /** Original PTS value */
   audio_uint64_t       render_time_stamp;   /** Time stamp to let render know when to accept the buffer.*/
   ismd_audio_format_t  audio_format;        /** The audio format of the buffer payload. */
   unsigned int         sample_size;         /** Sample size (bit width) of the audio samples */
   unsigned int         sample_rate;         /** Sample rate (frequency) of the audio samples */
   unsigned int         channel_count;       /** Number of channels in each audio sample */
   unsigned int         channel_config;      /** Describes the order the channels are orgainzed in memory in the buffer. */
   unsigned int         bitrate;             /** Bitrate of the audio data. */
   unsigned int         discontinuity;       /** If true, this buffer is the first buffer after a discontinuity has been detected. */
   audio_discont_desc_t discontinuity_desc;  /** Description of a known discontinuity contained in the buffer. */
   int                  opt_metadata_offset; /** Offset of optional metadata in the buffer payload, or OPT_METADATA_INVALID when not present */
   audio_tags_desc_t    tags;                /** Information for SMD buffers that have tags for this buffer. */
   int                  tag_input_h;         /** Input handle ID so events can be fired when tags are attached to this buffer. */
   unsigned char        ad_valid;            /** Audio description metadata valid flag.*/
   unsigned char        ad_fade_byte;        /** Audio description specific fade information. */
   unsigned char        ad_pan_byte;         /** Audio description specific pan information. */
   uint8_t              acmod;               /** Audio coding mode of the audio data if originated from an encoded stream. */
   unsigned short       is_encoded_data;     /** Set to true if the buffer ONLY has encoded data.*/
   unsigned short       aac_downmix_metadata;/** AAC specific downmix metadata flag. TODO: Remove this when no longer needed.*/
} audio_buffer_attr_t;

/**
  Message generated by decoder.
  */
typedef enum {
   ISMD_AUDIO_MESSAGE_STREAM_INFO_CHANGE = 0x0, /**< channel number or sample rate... */
} ismd_audio_message_t;

/** Dolby specific downmix structure */
typedef struct{
   audio_int16_t cmixlev;
   audio_int16_t surmixlev;
}dolby_dmixlevs_t;

/** Codec specific stream information.*/
typedef struct {
   /** This is the channel configuration (acmod) of the input bit stream. 
    * Valid values are 0 through 7. 
    * 0 indicates 2 full bandwidth channels (1/0 + 1/0), 
    * 1 � 1/0 (C)
    * 2 � 2/0 (L, R)
    * 3 � 3/0 (L, C, R)
    * 4 � 2/1 (L, R, l)
    * 5 � 3/1 (L, C, R, l)
    * 6 � 2/2 (L, R, l, r)
    * 7 � 3/2 (L, C, R, l, r) (default) */
   int acmod;

   /** 0 = not present 1= present*/
   int lfe_present;

   /** This the byte-order of the input bit stream. Any non zero value means
    * the application must byte swap the AC3 stream before feeding it to the driver.*/
   int input_byte_swap;

   /* extended bitstream information 1 */
   audio_int16_t bsid;                       /** Bitstream identification */
   audio_int16_t bsmod;                      /** Bitstream mode */
   audio_int16_t lfeon;                      /** Low freq effects chan flag */
   audio_int16_t xbsi1e;                     /** Extra BSI1 info exists */
   audio_int16_t copyright;                  /** Copyright bit information */

   /* downmix coefficients */
   dolby_dmixlevs_t legacy_dmixlevs;         /** Legacy downmix levels */
   dolby_dmixlevs_t ltrt_dmixlevs;           /** Lt/Rt downmix levels  */
   dolby_dmixlevs_t loro_dmixlevs;           /** Lo/Ro downmix levels */
}ismd_audio_stream_info_ac3_t;

/** Codec specific stream information.*/
typedef struct {
   /** The transport format for the encoded input bit stream: 0 -Unknown, 1-adif, 2-adts, 6-raw */
   int aac_format;

   /** Information about the audio coding mode of the input bit stream describing the encoded channel configuration.
    * 0 - Undefined, 
    * 1 - mono (1/0), 
    * 2 -parametric stereo, 
    * 3 - dual mono, 
    * 4 -stereo(2/0), 
    * 5 -L,C,R(3/0)
    * 6 � L, R, l (2/1),
    * 7 � L, R, l, r (2/2),
    * 8 � L, C, R, Cs (3/0/1)
    * 9 � L, C, R, l, r (3/2)
    * 10 � L, C, R, l, r, Cs (3/2/1)
    * 11 � L, C, R, l, r, Sbl, Sbr (3/2/2)
    * 12 � L, R, LFE (2/0.1)
    * 13 � L, C, R, LFE (3/0.1)
    * 14 � L, R, Cs, LFE (2/0/1.1)
    * 15 � L, R, Ls, Rs, LFE (2/2.1)
    * 16 � L, C, R, Cs, LFE (3/0/1.1)
    * 17 � L, C, R, l, r, LFE (5.1 mode)
    * 18 � L, C, R, l, r, Cs, LFE (3/2/1.1)
    * 19 � L, C, R, l, r, Sbl, Sbr, LFE (7.1 mode)*/
   int acmod;

   /** The decoded Audio Object Type.
    * 0 - plain AAC-LC (low complexity) object type
    * 1 - aacPlus object type containing SBR element.
    * 2 - aacPlusv2 object type containing PS object type*/
   int sbr_type;
}ismd_audio_stream_info_aac_t;

/** Codec specific stream information.*/
typedef struct {
   /** This indicates if an extension substream is present in the stream. 0 = not present 1= present*/
   int extn_substream_present; 

   /** 0 = not present 1= present*/
   int lfe_present;

   /** This is the number of extra full-bandwidth channels in addition to the main audio channels. 
    * This number does not include the LFE channel.
    * Valid values: 0 through 3*/
   int num_xchan;

   /** This is the channel mode information
    * bits 1:0 � 0 (stereo), 
    * 1 (joint stereo), 
    * 2 (dual mono), 
    * 3 (mono),
    * bit 4: intensity stereo, if set to 1
    * bit 5: mid-side stereo, if set to 1*/
   int ch_mode_info;

   /** Channel configuration information
    * 1 = L/C/R/Ls/Rs
    * 2 = L/C/R/Cs
    * 3 = L/C/R
    * 4 = L/C/R/L2/R2
    * 5 = L/R/Ls/Rs
    * 6 = L/R/Cs
    * 7 = L/R
    * 8 = L/R/L2/R2
    * 9 = C
    * 10 = C/L2/R2
    */
   int chan_config;

}ismd_audio_stream_info_mpeg_t;

/** Codec specific stream information.*/
typedef struct {
   audio_int16_t strmtyp;                   /**  Stream type */
   audio_int16_t substreamid;               /**  Sub-stream identification */
   audio_int16_t frmsiz;                    /**  Frame size (in 16-bit words) */
   audio_int16_t fscod2;                    /**  Sample rate code 2 (halfrate) */
   audio_int16_t chanmape;                  /**  Channel map exists flag */
   audio_int16_t chanmap;                   /**  Channel map data */
   audio_int16_t mixmdate;                  /**  Mixing metadata exists flag */
   audio_int16_t lfemixlevcode;             /**  LFE Mix Level Code exists flag */
   audio_int16_t lfemixlevcod;              /**  LFE Mix Level Code */
   audio_int16_t pgmscle[2];                /**  Program scale factor exists flags */
   audio_int16_t pgmscl[2];                 /**  Program scale factor */
   audio_int16_t extpgmscle;                /**  External program scale factor exists flags */
   audio_int16_t extpgmscl;                 /**  External program scale factor exists */
   audio_int16_t mixdef;                    /**  Mix control type */
   audio_int16_t mixdeflen;                 /**  Length of mixing parameter data field */
   audio_int16_t mixdata2e;                 /**  Mixing data 2 exists */
   audio_int16_t premixcmpsel;              /**  Premix compression word select */    
   audio_int16_t drcsrc;                    /**  Dynamic range control word source (external or current) */
   audio_int16_t premixcmpscl;              /**  Premix compression word scale factor */
   audio_int16_t extpgmlscle;               /**  External program left scale factor exists */
   audio_int16_t extpgmcscle;               /**  External program center scale factor exists */
   audio_int16_t extpgmrscle;               /**  External program right scale factor exists */
   audio_int16_t extpgmlsscle;              /**  External program left surround scale factor exists */
   audio_int16_t extpgmrsscle;              /**  External program right surround scale factor exists */
   audio_int16_t extpgmlfescle;             /**  External program LFE scale factor exists */
   audio_int16_t extpgmlscl;                /**  External program left scale factor */
   audio_int16_t extpgmcscl;                /**  External program center scale factor */      
   audio_int16_t extpgmrscl;                /**  External program right scale factor */
   audio_int16_t extpgmlsscl;               /**  External program left surround scale factor */
   audio_int16_t extpgmrsscl;               /**  External program right surround scale factor */
   audio_int16_t extpgmlfescl;              /**  External program LFE scale factor */
   audio_int16_t dmixscle;                  /**  Downmix scale factor exists */
   audio_int16_t dmixscl;                   /**  Downmix scale factor */
   audio_int16_t addche;                    /**  Additional scale factors exist */
   audio_int16_t extpgmaux1scle;            /**  External program 1st auxiliary channel scale factor exists */
   audio_int16_t extpgmaux1scl;             /**  External program 1st auxiliary channel scale factor */
   audio_int16_t extpgmaux2scle;            /**  External program 2nd auxiliary channel scale factor exists */
   audio_int16_t extpgmaux2scl;             /**  External program 2nd auxiliary channel scale factor */
   audio_int16_t frmmixcfginfoe;            /**  Frame mixing configuration information exists flag */
   audio_int16_t blkmixcfginfoe;            /**  Block mixing configuration information exists flag */
   audio_int16_t blkmixcfginfo[6];          /**  Block mixing configuration information */
   audio_int16_t paninfoe[2];               /**  Pan information exists flag */
   audio_int16_t panmean[2];                /**  Pan mean angle data */
   audio_int16_t paninfo[2];                /**  Pan information */
   audio_int16_t infomdate;                 /**  Information Meta-Data exists flag */
   audio_int16_t sourcefscod;               /**  Source sample rate code */
   audio_int16_t convsync;                  /**  Converter synchronization flag */
   audio_int16_t blkid;                     /**  Block identification */

   /* extended bitstream information 1 */
   audio_int16_t bsid;                       /** Bitstream identification */
   audio_int16_t bsmod;                      /** Bitstream mode */
   audio_int16_t acmod;                      /** Audio coding mode */
   audio_int16_t lfeon;                      /** Low freq effects chan flag */
   audio_int16_t nfchans;                    /** Derived # of full bw chans */
   audio_int16_t nchans;                     /** Derived # of channels */
   audio_int16_t niprgms;                    /** Derived # of independent programs */
   audio_int16_t xbsi1e;                     /** Extra BSI1 info exists */
   audio_int16_t copyright;                  /** Copyright bit information */

   /* downmix coefficients */
   dolby_dmixlevs_t legacy_dmixlevs;         /** Legacy downmix levels */
   dolby_dmixlevs_t ltrt_dmixlevs;           /** Lt/Rt downmix levels  */
   dolby_dmixlevs_t loro_dmixlevs;           /** Lo/Ro downmix levels */
}ismd_audio_stream_info_ddplus_t;

/** Codec specific stream information. Dolby Digital Plus mixing data fields */
typedef struct
{
   audio_int16_t  dmixmod;                   /** Preferred downmix mode */
   audio_int16_t  cmixlev;                   /** Center downmix level */
   audio_int16_t  surmixlev;                 /** Surround downmix level */
   audio_int16_t  ltrtcmixlev;               /** Lt/Rt center downmix level */
   audio_int16_t  ltrtsurmixlev;             /** Lt/Rt surround downmix level */
   audio_int16_t  lorocmixlev;               /** Lo/Ro center downmix level */
   audio_int16_t  lorosurmixlev;             /** Lo/Ro surround downmix level */
   audio_int16_t  pgmscl[2];                 /** Program scale factor */
   audio_int16_t  extpgmscl;                 /** External program scale factor  */
   audio_int16_t  paninfo[2];                /** Panning information */
   audio_int16_t  panmean[2];                /** Pan mean angle data */
   audio_int16_t  lfemixlevcod;              /** LFE mix level code */
   audio_int16_t  premixcmpsel;              /** Premix compression word select */
   audio_int16_t  drcsrc;                    /** Dynamic range control word source (external or current) */
   audio_int16_t  premixcmpscl;              /** Premix compression word scale factor */
   audio_int16_t  extpgmlscl;                /** External program left scale factor */
   audio_int16_t  extpgmcscl;                /** External program center scale factor */
   audio_int16_t  extpgmrscl;                /** External program right scale factor */
   audio_int16_t  extpgmlsscl;               /** External program left surround scale factor */
   audio_int16_t  extpgmrsscl;               /** External program right surround scale factor */
   audio_int16_t  extpgmlfescl;              /** External program LFE scale factor */
   audio_int16_t  dmixscl;                   /** Downmix scale factor */
   audio_int16_t  extpgmaux1scl;             /** External program 1st auxiliary channel scale factor */
   audio_int16_t  extpgmaux2scl;             /** External program 2nd auxiliary channel scale factor */
   audio_int16_t  mixdatavalid;              /** introduced for valid mixdata */
} ismd_audio_stream_info_ddc_t;


/** Codec specific stream information.*/
typedef struct {
   int channel_mask;
   int mix_metadata_enabled;
   int mix_metadata_adj_level;
   int bits_for_mixout_mask;
   int num_mixout_configs;
   int mixout_chmask[4];
   int mix_metadata_present;
   int external_mixflag;
   int postmix_gainadj_code;
   int control_mixer_drc;
   int limit_embeded_drc;
   int custom_drc_code;
   int enbl_perch_mainaudio_scale;
   int onetoone_mixflag;
   int mix_out_config;
   //unsigned char ui8MainAudioScaleCode[4][16];
   unsigned char ui8MainAudioScaleCode[16];
   //unsigned short ui16MixMapMask[4][3][6];
   unsigned short ui16MixMapMask[6];
   //unsigned char ui8MixCoeffs[4][3][6][16];
   unsigned char ui8MixCoeffs[6][16];
} ismd_audio_stream_info_dts_lbr_t;

/** Codec specific stream information.*/
typedef struct {
   int audio_emphasis_flag;
   int audio_mute_flag;
   int dynamic_range_control;
}ismd_audio_stream_info_dvd_pcm_t;

/** Codec specific stream information.*/
typedef struct {
   int      active_channel_count;

   int      gain_required;
   int      valid_metadata;
   int      stream_type;
   
   uint8_t  ch_meaning_fba_reserved1                       ; /** Reserved :  6 */
   bool     ch_meaning_fba_control_enabled_2ch             ; /** DRC applied to 2ch :  1 */
   bool     ch_meaning_fba_control_enabled_6ch             ; /** DRC applied to 6ch :  1 */
   bool     ch_meaning_fba_control_enabled_8ch             ; /** DRC applied to 8ch :  1 */
   bool     ch_meaning_fba_reserved2                       ; /** Reserved :  1 */
   int8_t   ch_meaning_fba_start_up_gain                   ; /** Initialize gain of DRC :  7 */
   uint8_t  ch_meaning_fba_dialogue_norm_2ch               ; /** Average Dialogue level below full scale dB, default 0 means -31dB :  6 */
   uint8_t  ch_meaning_fba_mix_level_2ch                   ; /** Absolute SPL of presentation. Add to 70 :  6 */
   uint8_t  ch_meaning_fba_dialogue_norm_6ch               ; /** Average Dialogue level below full scale dB, default 0 means -31dB :  5 */
   uint8_t  ch_meaning_fba_mix_level_6ch                   ; /** Absolute SPL of presentation. Add to 70 :  6 */
   uint8_t  ch_meaning_fba_source_format_6ch               ; /** Describe hierarchial source information. Default 0 :  5 */
   uint8_t  ch_meaning_fba_dialogue_norm_8ch               ; /** Average Dialogue level below full scale dB, default 0 means -31dB :  5 */
   uint8_t  ch_meaning_fba_mix_level_8ch                   ; /** Absolute SPL of presentation. Add to 70 :  6 */
   uint8_t  ch_meaning_fba_source_format_8ch               ; /** Describe hierarchial source information. Default 0 :  6 */
   bool     ch_meaning_fba_reserved3                       ; /** Reserved :  1 */
   bool     ch_meaning_fba_extra_channel_meaning_present   ; /** Set if further metadata given after channel meaning :  1 */  

   uint8_t  ch_meaning_fbb_summary_info                    ; /** Compact encoding of FBB fields below:  5 */
   uint8_t  ch_meaning_fbb_source_format                   ; /** Multichannel system being used :  4 */
   uint8_t  ch_meaning_fbb_reserved                        ; /** Reserved :  7 */
   uint16_t ch_meaning_fbb_level_control                   ; /** Level control: 16 */
   uint8_t  ch_meaning_fbb_copy_protection                 ; /** Copy protection bit for stream:  3 */
   uint16_t ch_meaning_fbb_speaker_layout                  ; /** Describes horizonatal and 3D speaker layouts : 10 */
   uint8_t  ch_meaning_fbb_multi_channel_type              ; /** Describes channel purpose :  3 */
   uint8_t  ch_meaning_fbb_channel_occupancy               ; /** Channel 5-0 are active or not :  6 */
   uint8_t  ch_meaning_fbb_wordwidth                       ; /** Max width of sample values :  5 */
   uint8_t  ch_meaning_fbb_fs                              ; /** Enumeration of standard sampling frequencies :  5 */   

   uint16_t format_info_fba_decoder_channel_assignment_8ch  ; /** FBA Channel assignment for 8ch presentation : 13 */
   uint8_t  format_info_fba_decoder_channel_modifier_8ch    ; /** FBA Channel modifier for 8ch presentation :  2 */
   uint8_t  format_info_fba_decoder_channel_assignment_6ch  ; /** FBA Channel assignment for 6ch presentation :  5 */
   uint8_t  format_info_fba_decoder_channel_modifier_6ch    ; /** FBA Channel modifier for 6ch presentation  :  2 */
   uint8_t  format_info_fba_decoder_channel_modifier_2ch    ; /** FBA Channel modifier for 2ch presentation  :  2 */
   uint8_t  format_info_fba_reserved                        ; /** FBA Reserved :  2 */
   bool     format_info_fba_multichannel_type_8ch           ; /** FBA Multichannel type for 8ch presentation :  1 */
   bool     format_info_fba_multichannel_type_6ch           ; /** FBA Multichannel type for 6ch presentation :  1 */
   uint8_t  format_info_fba_audio_sampling_frequency        ; /** FBA Stream sampling frequency :  4 */   

   uint8_t  format_info_fbb_channel_assignment              ; /** FBB Channel Assignment information : 5 */
   uint8_t  format_info_fbb_reserved3                       ; /** FBB reserved : 3 */
   uint8_t  format_info_fbb_multi_channel_type              ; /** FBA Multichannel type : 4 */
   uint8_t  format_info_fbb_reserved4                       ; /** FBB reserved : 4 */
   uint8_t  format_info_fbb_audio_sampling_frequency_2      ; /** FBB Stream sampling frequency 2 : 4 */
   uint8_t  format_info_fbb_audio_sampling_frequency_1      ; /** FBB Stream sampling frequency 1 : 4 */
   uint8_t  format_info_fbb_quantization_word_length_2      ; /** FBB Word length 2 : 4 */
   uint8_t  format_info_fbb_quantization_word_length_1      ; /** FBB Word length 1 : 4 */   
}ismd_audio_stream_info_truehd_t;

/** Union used in \ref ismd_audio_format_specific_stream_info_t.*/
typedef union
{
   ismd_audio_stream_info_ddplus_t ddplus;
   ismd_audio_stream_info_ac3_t ac3;
   ismd_audio_stream_info_mpeg_t mpeg;
   ismd_audio_stream_info_aac_t aac;
   ismd_audio_stream_info_dvd_pcm_t dvd_pcm;
   ismd_audio_stream_info_truehd_t truehd;
   ismd_audio_stream_info_dts_lbr_t dts_lbr;
   ismd_audio_stream_info_ddc_t ddc;
}ismd_audio_format_stream_info_t;

/** Use this structure to get detailed codec stream information when calling
 \ref ismd_audio_input_get_format_specific_stream_info.*/
typedef struct
{
   ismd_audio_stream_info_t basic_stream_info;
   ismd_audio_format_stream_info_t format;
} ismd_audio_format_specific_stream_info_t;

/** \deprecated.
This struct defines catagor codes to be placed in SPDIF
channel status bits. (Has been deprecated)
*/
typedef enum{
   SPDIF_CATEGORY_GENERAL = 0x00, //   000 00000 General. Used temporarily
   SPDIF_CATEGORY_LASER_OPTICAL = 0x01, //** 100 XXXXL Laser optical products
   SPDIF_CATEGORY_DIG_CONVERTERS_OR_SIGNAL_PROCESSING = 0x02, //   010 XXXXL Digital/digital converters and signal processing products
   SPDIF_CATEGORY_MAGNETIC_TAPE_OR_DISC = 0x03,//   110 XXXXL Magnetic tape or disc based products
   SPDIF_CATEGORY_DIGITAL_BROADCAST = 0x04, //** 001 XXXXL Broadcast reception of digitally encoded audio signals with or without video signals
   SPDIF_CATEGORY_DIGITAL_BROADCAST1 = 0x0E, //** 011 1XXXL Broadcast reception of digitally encoded audio signals with or without video signals
   SPDIF_CATEGORY_MUSICAL_RECORDING = 0x05, //   101 XXXXL Musical instruments, microphones and other sources without copyright information
   SPDIF_CATEGORY_ANALOG_DIG_CONVERTERS_NO_COPYRIGHT = 0x06, //  011 00XXL Analogue/digital converters for analogue signals without copyright information
   SPDIF_CATEGORY_ANALOG_DIG_CONVERTERS_COPYRIGHT = 0x16, //  011 01XXL Analogue/digital converters for analogue signals which include copyright information
   SPDIF_CATEGORY_SOLID_STATE_MEMORY = 0x08, //   000 1XXXL Solid state memory based products
   SPDIF_CATEGORY_EXPERIMENTAL = 0x40, //   000 0001L Experimental products not for commercial sale
   SPDIF_CATEGORY_RESERVED = 0x07, // 111 XXXXL Not defined. Reserved
}spdif_chan_stat_category_code_t;

/**
Use this struct to set certain fields in the SPDIF channel 
status bits. (Has been deprecated)
*/
typedef struct
{
   spdif_chan_stat_category_code_t cat_code;
   bool copyright;
   bool gen_status;//Set this to true to set the L-bit to 1. False to clear to 0.
} ismd_audio_spdif_channel_status_t;

/**
Use this struct to set certain fields in the IEC60958 channel 
status bits. 
*/
typedef struct {
   /** The category_code indicates the kind of equipment that generates the digital audio interface signal
    *  0x00:  000 00000 General. Used temporarily
    *  0x01:  100 XXXXL Laser optical products
    *  0x02:  010 XXXXL Digital/digital converters and signal processing products
    *  0x03:  110 XXXXL Magnetic tape or disc based products
    *  0x04:  001 XXXXL Broadcast reception of digitally encoded audio signals with or without video signals         
    *  0x0E:  011 1XXXL Broadcast reception of digitally encoded audio signals with or without video signals         
    *  0x05:  101 XXXXL Musical instruments, microphones and other sources without copyright information
    *  0x06:  011 00XXL Analogue/digital converters for analogue signals without copyright information
    *  0x16:  011 01XXL Analogue/digital converters for analogue signals which include copyright information
    *  0x08:  000 1XXXL Solid state memory based products
    *  0x40:  000 0001L Experimental products not for commercial sale
    *  0x07:  111 XXXXL Not defined. Reserved */
   uint8_t category_code;
      
   /** Linear PCM audio mode
    *  0x00:  2 audio channels without pre-emphasis; 
    *  0x01:  2 audio channels with 50us/15us pre-emphasis. 
    *  others: reversed for future */
   uint8_t pcm_audio_mode;       

   /** 0x00:  copyright is asserted
    *  0x01:  no copyright is asserted
    *  others: invalid */
   uint8_t copyright;       

   /** IEC60958 Generation status/L-bit. 
    *
    *  Generally the L-bit is specified as:
    *  0x00:  No indication
    *  0x01:  Commercially released pre-recorded software 
    *  
    *  For category_code as 0x01, 0x04 and 0x0E, the reverse situation is valid
    *  0x00:  Commercially released pre-recorded software
    *  0x01:  No indication */
   uint8_t gen_status;       
} ismd_audio_iec60958_channel_status_t;


/**
 * Define input status type
*/
typedef enum {
   ISMD_AUDIO_INPUT_STATUS_CURRENT_STATE  = 0x1,
   ISMD_AUDIO_INPUT_STATUS_ASRC_MODE  = 0x2,
} ismd_audio_input_status_type_t;
 
/**
 *  Define input status
 */
typedef enum {
   ISMD_AUDIO_STATUS_OKAY                    =      0,
   ISMD_AUDIO_STATUS_NO_DATA_AVAIL           = 1 << 0,
   ISMD_AUDIO_STATUS_DECODE_ERROR            = 1 << 1,
   ISMD_AUDIO_STATUS_UNSUPPORTED_FORMAT      = 1 << 2,
   ISMD_AUDIO_STATUS_UNSUPPORTED_CONVERSION  = 1 << 3,
} ismd_audio_status_t;

/**
 *  Define input asrc mode
 */
typedef enum {
   ISMD_AUDIO_ASRC_DISABLE                    =      0,
   ISMD_AUDIO_ASRC_ENABLE					  = 	 1,
   ISMD_AUDIO_ASRC_AUTO					      = 	 2,
} ismd_audio_asrc_status_t;


/**
 * This union includes information details that application requests.
 */
typedef union {
   ismd_audio_status_t  status;
   ismd_audio_asrc_status_t asrc_mode;
} ismd_audio_input_status_t;


/**
Use these values when calling \ref ismd_audio_output_set_downmix_mode
*/
typedef enum {
   ISMD_AUDIO_DOWNMIX_INVALID,                  
   ISMD_AUDIO_DOWNMIX_DEFAULT,                 
   ISMD_AUDIO_DOWNMIX_1_0,                     
   ISMD_AUDIO_DOWNMIX_1_0_LFE,                  
   ISMD_AUDIO_DOWNMIX_2_0,
   ISMD_AUDIO_DOWNMIX_2_0_NO_SCALE,
   ISMD_AUDIO_DOWNMIX_2_0_LFE,                  
   ISMD_AUDIO_DOWNMIX_2_0_LTRT,
   ISMD_AUDIO_DOWNMIX_2_0_LTRT_NO_SCALE,
   ISMD_AUDIO_DOWNMIX_2_0_DOLBY_PRO_LOGIC_II,
   ISMD_AUDIO_DOWNMIX_2_0_DOLBY_PRO_LOGIC_II_NO_SCALE,
   ISMD_AUDIO_DOWNMIX_2_0_DVB_AAC,
   ISMD_AUDIO_DOWNMIX_2_1,
   ISMD_AUDIO_DOWNMIX_2_1_LFE,
   ISMD_AUDIO_DOWNMIX_3_0,
   ISMD_AUDIO_DOWNMIX_3_0_LFE,
   ISMD_AUDIO_DOWNMIX_3_1,
   ISMD_AUDIO_DOWNMIX_3_1_LFE,
   ISMD_AUDIO_DOWNMIX_2_2,
   ISMD_AUDIO_DOWNMIX_2_2_LFE,
   ISMD_AUDIO_DOWNMIX_3_2,
   ISMD_AUDIO_DOWNMIX_3_2_LFE,
   ISMD_AUDIO_DOWNMIX_3_0_1,
   ISMD_AUDIO_DOWNMIX_3_0_1_LFE,
   ISMD_AUDIO_DOWNMIX_2_2_1,
   ISMD_AUDIO_DOWNMIX_2_2_1_LFE,
   ISMD_AUDIO_DOWNMIX_3_2_1,
   ISMD_AUDIO_DOWNMIX_3_2_1_LFE,   
   ISMD_AUDIO_DOWNMIX_3_0_2,
   ISMD_AUDIO_DOWNMIX_3_0_2_LFE,
   ISMD_AUDIO_DOWNMIX_2_2_2,
   ISMD_AUDIO_DOWNMIX_2_2_2_LFE,
   ISMD_AUDIO_DOWNMIX_3_2_2,
   ISMD_AUDIO_DOWNMIX_3_2_2_LFE
} ismd_audio_downmix_mode_t;

/** Use these values to specify mixer output channel config mode in \ref ismd_audio_set_mixing_channel_config*/
typedef enum {
   ISMD_AUDIO_MIX_CH_CFG_MODE_INVALID,
   ISMD_AUDIO_MIX_CH_CFG_MODE_AUTO,
   ISMD_AUDIO_MIX_CH_CFG_MODE_FIXED,
   ISMD_AUDIO_MIX_CH_CFG_MODE_PRIMARY
}ismd_audio_mix_ch_cfg_mode_t;

/** Use these values to specify mixer sample rate mode in \ref ismd_audio_set_mixing_sample_rate*/
typedef enum {
   ISMD_AUDIO_MIX_SAMPLE_RATE_MODE_INVALID,
   ISMD_AUDIO_MIX_SAMPLE_RATE_MODE_AUTO,
   ISMD_AUDIO_MIX_SAMPLE_RATE_MODE_FIXED,
   ISMD_AUDIO_MIX_SAMPLE_RATE_MODE_PRIMARY
}ismd_audio_mix_sample_rate_mode_t;


/** Use these defines in the function \ref ismd_audio_set_sampling_rate_conversion_quality*/
typedef enum {
   ISMD_AUDIO_SRC_QUALITY_DEFAULT,  /** Default SRC quality setting.*/
   ISMD_AUDIO_SRC_QUALITY_HIGH      /** Higher quality sample rate conversion.*/
} ismd_audio_src_quality_t;


/**************************************************************************/
/*! @Audio Quality Pipeline Defines */

/** Data type for an audio quality filter handle. See \ref ismd_audio_output_quality_filter_alloc*/
typedef int ismd_audio_quality_filter_t;
/** Data type for an audio quality control handle. See \ref ismd_audio_output_quality_control_alloc*/
typedef int ismd_audio_quality_control_t;

/** Maximum audio quality filters available. */
#define AUDIO_QUALITY_MAX_FILTERS 40
/** Maximum audio quality controls available. */
#define AUDIO_QUALITY_MAX_CONTROLS 16
/** Maximum audio quality stages available. */
#define AUDIO_QUALITY_MAX_STAGES 22

/** Min for AVL attack time parameter, in milliseconds */
#define AUDIO_QUALITY_ATTACK_TIME_MIN 0
/** Max for AVL attack time parameter, in milliseconds */
#define AUDIO_QUALITY_ATTACK_TIME_MAX 255

/** Min for AVL release time parameter, in milliseconds */
#define AUDIO_QUALITY_REL_TIME_MIN 0
/** Max for AVL release time parameter, in milliseconds */
#define AUDIO_QUALITY_REL_TIME_MAX 7650

/** Min value for Fc filter parameter */
#define AUDIO_QUALITY_FILTER_FC_MIN 0
/** Max value  for Fc filter parameter */
#define AUDIO_QUALITY_FILTER_FC_MAX 96000

/** The maximum channels supported by the audio quality mixer.*/
#define AUDIO_QUALITY_MIXER_MAX_CHANNELS 4

/** Use this enum to specify a type of filter.*/
typedef enum {
   ISMD_AUDIO_FILTER_INVALID, 
   ISMD_AUDIO_FILTER_HPF2,
   ISMD_AUDIO_FILTER_LPF2,
   ISMD_AUDIO_FILTER_HPF1,
   ISMD_AUDIO_FILTER_LPF1,
   ISMD_AUDIO_FILTER_SHLF_L2,
   ISMD_AUDIO_FILTER_SHLF_H2,
   ISMD_AUDIO_FILTER_SHLF_L1,
   ISMD_AUDIO_FILTER_SHLF_H1,
   ISMD_AUDIO_FILTER_PEQ2, 
   ISMD_AUDIO_FILTER_COUNT
}ismd_audio_quality_filter_type_t;

/** Use this struct to specify filter parameters. */
typedef struct{
   int fc;
   int q;
   ismd_audio_gain_value_t gain;
}ismd_audio_quality_filter_params_t;

/** Use this struct to specify duplication block parameters. */
typedef struct{
   int DupFromChan;
   int DupBeforeStage;
} ismd_audio_quality_dup_params_t;

/** Use this enum to specify a type of control. */
typedef enum {
   ISMD_AUDIO_CONTROL_INVALID,
   ISMD_AUDIO_CONTROL_PC_VOLUME,
   ISMD_AUDIO_CONTROL_AVL,
   ISMD_AUDIO_CONTROL_VOLUME,
   ISMD_AUDIO_CONTROL_MUTE,
   ISMD_AUDIO_CONTROL_DUP,
   ISMD_AUDIO_CONTROL_MINIMIX,
   ISMD_AUDIO_CONTROL_CLAMP,
   ISMD_AUDIO_CONTROL_BASS_LIMITER,
   ISMD_AUDIO_CONTROL_TREBLE_LIMITER,
   ISMD_AUDIO_CONTROL_LEVEL_METER,
   ISMD_AUDIO_CONTROL_UPROG_FIR,       /** < User programmable, up to 512 tap FIR*/
   ISMD_AUDIO_CONTROL_UPROG_BQIIR,     /** < User programmable, bi-quad IIR*/
   ISMD_AUDIO_CONTROL_COUNT
}ismd_audio_quality_control_type_t;


/** Pameters required by the AVL control. */
typedef struct {
   ismd_audio_gain_value_t t1;         /** < Changing point 1 (dB)	(0.5dB/step)	*/
   ismd_audio_gain_value_t t2;         /** < Changing point 2 (dB)	(0.5dB/step)*/	
   ismd_audio_gain_value_t t1_gain;    /** < Gain at changing point 1 (dB)	(0.5dB/step)*/	
   ismd_audio_gain_value_t t2_gain;    /** < Gain at changing point 2 (dB)	(0.5dB/step)*/	
   ismd_audio_gain_value_t k0;         /** < Gradient less than T1 area	(0.1/step)	*/
   ismd_audio_gain_value_t k2;         /** < Gradient more than T2 area	(0.1/step)	*/
   int attack_time;                    /** < Attack Time (mSec)	(1mSec/step)	0~255mSec*/
   int release_time;                   /** < Release Time (mSec)	(30mSec/step)	0~7650mSec*/
   ismd_audio_gain_value_t offset_gain;/** < Offset gain (dB)	(0.1dB/step)	*/
} ismd_audio_quality_avl_params_t;

/** Pameters required by the volume control.*/
typedef struct {
   int gain; /** < Gain in 0.1dB steps*/
   int ramp; /** < Time in milliseconds for one naperian time constant. */
}ismd_audio_quality_pc_volume_params_t;

/** Parameters required by level meter */
typedef struct {
   int window; /** windows size in 100mSec units, valid values 1 to 20*/
} ismd_audio_quality_level_params_t;

/** Pameters required by the volume control.*/
typedef struct {
   ismd_audio_gain_value_t gain; /** < Gain in 0.1dB steps*/
   int ramp;                     /** < Time in milliseconds for one naperian time constant. */
}ismd_audio_quality_volume_params_t;

/** Pameters required by the mute control.*/
typedef struct {
   int mute;   /** < If non-zero, mute is on, otherwise mute is off */
   int ramp;   /** < Time in milliseconds for one naperian time constant.	*/
}ismd_audio_quality_mute_params_t;

/** Pameters required by the mixer control.*/
typedef struct {
   /** Channel number of input channel to feed into mixer.
       If set to -1 then this input to the mixer is unused*/
   int mix_src[AUDIO_QUALITY_MIXER_MAX_CHANNELS];
   /** Gain in units of 0.1 dB = i.e. -6dB would be -60*/
   int gain[AUDIO_QUALITY_MIXER_MAX_CHANNELS];
   /** Ramp time to adjust to a new gain, this is a 1st 
       order RC time constant, in 0.1mSec - so a value of 
       1000 causes the mixer to ramp 0.633 of the difference 
       between the old gain and the new gain in 100mSec */
   int ramp[AUDIO_QUALITY_MIXER_MAX_CHANNELS];
   /** Flag for whether the input is phase inverted, 0 
       means do not phase invert, non-zero is phase invert. */
   char pi[AUDIO_QUALITY_MIXER_MAX_CHANNELS];
} ismd_audio_quality_minimix_params_t;

/** Pameters required by bass/treble limiter controls.*/
typedef struct {
   ismd_audio_gain_value_t volume;    /** < Volume Control Set point (dB) (0.5dB/step) */
   ismd_audio_gain_value_t set_p;     /** < Changing point 1 (dB)	(0.5dB/step)*/
   ismd_audio_gain_value_t set_l;     /** < Changing point 2 (dB)	(0.5dB/step)*/
   ismd_audio_gain_value_t set_h;     /** < Changing point 3 (dB)	(0.5dB/step)*/
   ismd_audio_gain_value_t k0;        /** < Gradient less than set_p area (dB)	(0.01dB/step)*/
   ismd_audio_gain_value_t k1;        /** < Gradient more than set_p and less than set_l area (dB)	(0.01dB/step)*/
   ismd_audio_gain_value_t k2;        /** < Gradient more than set_h area (dB)	(0.01dB/step)*/
   int fc;                            /** < Cut off frequency (150-3200, 1Hz/step) */
}ismd_audio_quality_loudness_params_t;

typedef struct {
	int level;
} ismd_audio_quality_clamp_params_t;

/** Parameters required by pro fir controls.*/
typedef struct {
   int coef_buf_handle;             /** < Handle of FIR coef buf*/
   int coef_num;                    /** < FIR order, support up to 512 taps*/
} ismd_audio_quality_pro_fir_params_t;

/** Parameters required by pro bqiir controls.*/
typedef struct {
   int prescale_shift;              /** < Pre scale shift*/
   int b0;                          /** < Numerator z0 coefficient*/
   int b1;                          /** < Numerator z-1 coefficient */
   int b2;                          /** < Numerator z-2 coefficient*/
   int a1;                          /** < Denominator z-1 coefficient*/
   int a2;                          /** < Denominator z-2 coefficient*/
} ismd_audio_quality_pro_bqiir_params_t;

/** Used to specify parameters depending on which control/filter it references.*/
typedef union {
   ismd_audio_quality_avl_params_t avl;               /** < AVL control paramter*/
   ismd_audio_quality_dup_params_t dup;               /** < Dup control paramter*/
   ismd_audio_quality_loudness_params_t bass_loud;    /** < Base limiter control paramter*/
   ismd_audio_quality_loudness_params_t treble_loud;  /** < Treble limiter control paramter*/
   ismd_audio_quality_minimix_params_t minimix;       /** < Minimix control paramter*/
   ismd_audio_quality_clamp_params_t clamp;           /** < Clamp control paramter*/
   ismd_audio_quality_pc_volume_params_t pc_volume;   /** < Volume control paramter*/
   ismd_audio_quality_level_params_t level_meter;     /** < Level meter control paramter*/
   ismd_audio_quality_pro_fir_params_t pro_fir;       /** < Pro fir control parameter*/
   ismd_audio_quality_pro_bqiir_params_t pro_bqiir;   /** < Pro bqiir control parameter*/
} ismd_audio_quality_control_params_t;


/**************************************************************************/

/**Use to set the audio input stream parsing mode. See: \ref ismd_audio_input_set_stream_parsing_mode*/
typedef enum {
   ISMD_AUDIO_INPUT_PARSING_MODE_DISABLED,         /** Input parsing is disabled.*/
   ISMD_AUDIO_INPUT_PARSING_MODE_IEC60958,         /** Input will be parsed according to the IEC60958 spec.*/
   ISMD_AUDIO_INPUT_PARSING_MODE_IEC61937,         /** Input will be parsed according to the IEC61937 spec.*/
   ISMD_AUDIO_INPUT_PARSING_MODE_IEC60958_CUSTOM   /** Non-standard input parsing, then parsed according to the IEC60958 spec.*/
}ismd_audio_input_parsing_mode_t;

/** The status indicators of the audio stream parser. */
typedef enum {
   ISMD_AUDIO_PARSER_STATUS_SYNC_NOT_FOUND = 0,   /** Parser is in the process of finding sync but is not found yet.*/
   ISMD_AUDIO_PARSER_STATUS_SYNC_FOUND,           /** Parser is in a stable state, processing data.*/
   ISMD_AUDIO_PARSER_STATUS_SYNC_ERROR            /** Parser has made multiple attempts to find sync, but is not found. This 
                                                      status is an indication that the input stream cannot be parsed and is corrupt.*/
} ismd_audio_parser_status_t;

/** The possible events to register for when enabling input parsing. See \ref ismd_audio_input_set_stream_parsing_mode*/
typedef enum {
   ISMD_AUDIO_PARSER_EVENT_NONE                    = 0,       /** No events will be set by the parser.*/
   ISMD_AUDIO_PARSER_EVENT_CHANNEL_STATUS_CHANGED  = 1 << 0,  /** Notify when IEC60958 channel status info changes.*/
   ISMD_AUDIO_PARSER_EVENT_AUDIO_FORMAT_CHANGE     = 1 << 1,  /** Notify if new audio format is detected. */
   ISMD_AUDIO_PARSER_EVENT_VALIDITY_BIT_CHANGE     = 1 << 2,  /** Notify if the validity bit has changed. */
   ISMD_AUDIO_PARSER_EVENT_SYNC_ERROR              = 1 << 3,  /** Notify if the parser cannot find sync after multiple attempts. */
} ismd_audio_parser_event_t;

/** Current input parsing status and audio stream information. See \ref ismd_audio_input_get_detected_stream_info */
typedef struct{
   ismd_audio_parser_event_t triggered_event;         /** The last event signaled by the parser.*/
   ismd_audio_stream_info_t stream_info;              /** Detected audio stream attributes.*/
   ismd_audio_iec60958_channel_status_t chan_status;  /** Last detected IEC60958 channel status data.*/
   ismd_audio_parser_status_t curr_status;            /** The current status of the input parser. */
}ismd_audio_input_parsing_status_t;

/******************************************************************************/
/** \ref AUDIO_WM_CINAVIA_MAX_PARAMS_LEN, 
    \ref AUDIO_WM_CINAVIA_MAX_PARAM_BLOCK_SIZE and \ref audio_wm_params_block_t
    are for Cinavia watermark backward compatibility only. */

/** Used in \ref audio_wm_cnv_shared_buf_layout_deprecated_t as the max 
    parameter buffer length.*/
#define AUDIO_WM_CINAVIA_MAX_PARAMS_LEN 1472 // 64 x 23
/** Used in \ref audio_wm_params_block_t */
#define AUDIO_WM_CINAVIA_MAX_PARAM_BLOCK_SIZE 256

/** AUDIO_WM_MAX_PARAMS_LEN and AUDIO_WM_MAX_PARAM_BLOCK_SIZE are the original
    definition in Cinavia watermark. In order to keep the backward compatibility,
    these two definitions remain the same.*/
#define AUDIO_WM_MAX_PARAMS_LEN AUDIO_WM_CINAVIA_MAX_PARAMS_LEN
#define AUDIO_WM_MAX_PARAM_BLOCK_SIZE AUDIO_WM_CINAVIA_MAX_PARAM_BLOCK_SIZE

/** Use this structure to specify audio water mark parameters. See \ref ismd_audio_enable_watermark_detection.*/
typedef struct {
   unsigned short len;
   unsigned short offset;
   unsigned char params[AUDIO_WM_CINAVIA_MAX_PARAM_BLOCK_SIZE];
} audio_wm_params_block_t;

/** Audio watermark instance handle type to access or configure an audio
    watermark instance.*/
typedef int32_t ismd_audio_wm_instance_t;

/** @} */ // End of weakgroup ismd_audio_data_types

#ifdef __cplusplus
}
#endif
#endif
