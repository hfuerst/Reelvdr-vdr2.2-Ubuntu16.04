/*
    This file is provided under a dual BSD/LGPLv2.1 license.  When using
    or redistributing this file, you may do so under either license.

    LGPL LICENSE SUMMARY

    Copyright(c) 2012. Intel Corporation. All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation; either version 2.1 of the
    License.

    This library is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
    USA. The full GNU Lesser General Public License is included in this
    distribution in the file called LICENSE.LGPL.

    Contact Information for Intel:
        Intel Corporation
        2200 Mission College Blvd.
        Santa Clara, CA  97052

    BSD LICENSE

    Copyright (c) 2012. Intel Corporation. All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

      - Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      - Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in
        the documentation and/or other materials provided with the
        distribution.
      - Neither the name of Intel Corporation nor the names of its
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

#include <gst/gstmarshal.h>
#include "ismd_core.h"
#include <ismd_audio.h>
#include <ismd_audio_defs.h>
#include <ismd_core_protected.h>
#include "ismd_gst_element.h"
#include "ismd_gst_audio_dec.h"

#define AAC_FREQUENCY_INDEX     12
#define AAC_CHANNEL_INDEX       7

enum
{
  UNKNOWN,
  PROP_AUDIO_GLOBAL_PROC,
  PROP_AUDIO_CH_CONFIG,
  PROP_AUDIO_OUT_MODE,
  PROP_AUDIO_SAMPLE_SIZE,
  PROP_AUDIO_SAMPLE_RATE,
};

enum
{
  OUT_PCM = ISMD_AUDIO_OUTPUT_PCM,
  OUT_AC3 = ISMD_AUDIO_OUTPUT_ENCODED_DOLBY_DIGITAL,
  OUT_DTS = ISMD_AUDIO_OUTPUT_ENCODED_DTS
};

enum
{
  OUT_SAMPLE_SIZE_16 = 16,
  OUT_SAMPLE_SIZE_20 = 20,
  OUT_SAMPLE_SIZE_24 = 24,
  OUT_SAMPLE_SIZE_32 = 32
};

enum
{
  OUT_SAMPLE_RATE_32K = 32000,
  OUT_SAMPLE_RATE_48K = 48000,
  OUT_SAMPLE_RATE_44100 = 44100,
  OUT_SAMPLE_RATE_88200 = 88200,
  OUT_SAMPLE_RATE_96K = 96000,
  OUT_SAMPLE_RATE_176400 = 176400,
  OUT_SAMPLE_RATE_192K = 192000
};

enum
{
    OUT_STEREO = ISMD_AUDIO_STEREO,
    OUT_DUAL_MONO = ISMD_AUDIO_DUAL_MONO,
    OUT_5_1 = ISMD_AUDIO_5_1,
    OUT_7_1 = ISMD_AUDIO_7_1,
};

typedef struct
{
  unsigned int data;
  unsigned char index;
} aac_audio_index;


#define ISMD_AUDIO_SRC_CAPS \
                            "audio/x-raw-int;" 

static GstStaticCaps srccap =
{ {0, 0, (GstCapsFlags) 0}, ISMD_AUDIO_SRC_CAPS};


/* AAC Audio Sample Frequency Index Table as per 13818-7 */
static aac_audio_index audio_frequency[AAC_FREQUENCY_INDEX] = {
  {96000, 0x00}, {88200, 0x01}, {64000, 0x02}, {48000, 0x03}, {44100, 0x04},
  {32000, 0x05}, {24000, 0x06}, {22050, 0x07}, {16000, 0x08}, {12000, 0x09},
  {11025, 0x0a}, {8000, 0x0b}
};

#define freq_index_max_value (sizeof(audio_frequency)/(sizeof(*audio_frequency)))

/* AAC Audio Channels Index Table as per 13818-7 */
static aac_audio_index audio_channels[AAC_CHANNEL_INDEX] = {
  {1, 0x01}, {2, 0x02}, {3, 0x03}, {4, 0x04}, {5, 0x05}, {6, 0x06}, {8, 0x7}
};

#define DEFAULT_ISMD_AUDIO_DECODER_GLOBAL_PROC FALSE 
#define DEFAULT_ISMD_AUDIO_DECODER_SYNC       FALSE
#define DEFAULT_ISMD_AUDIO_DECODER_DELAY       0   /* in nanoseconds */
#define DEFAULT_ISMD_AUDIO_DECODER_SAMPLE_RATE       OUT_SAMPLE_RATE_48K
#define DEFAULT_ISMD_AUDIO_DECODER_SAMPLE_SIZE       OUT_SAMPLE_SIZE_24 
#define DEFAULT_ISMD_AUDIO_DECODER_OUT_MODE         OUT_PCM
#define DEFAULT_ISMD_AUDIO_DECODER_CHANNEL_CFG      OUT_STEREO 


#define PCM_CAPS \
  "audio/x-raw-int, " \
    "endianness = (int) { LITTLE_ENDIAN }, " \
    "signed = (boolean) { true }, " \
    "width = (int) { 16, 24, 32 }, " \
    "depth = (int) { 16, 24, 32 }, " \
    "rate = (int) { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 96000, 192000 }, " \
    "channels = (int) [ 1, 8 ]"

#define XPRIV_LPCM_CAPS     "audio/x-private1-lpcm"
#define MPEG12_CAPS         "audio/mpeg, mpegversion = (int) { 1 }"
#define XMPEG12_CAPS        "audio/x-mpeg, mpegversion = (int) { 1 }"
#define MPEG4_CAPS          "audio/mpeg, mpegversion = (int) { 2, 4 }"
#define XMPEG4_CAPS         "audio/x-mpeg, mpegversion = (int) { 2, 4 }"
#define XAAC_CAPS           "audio/x-aac"
#define XAC3_CAPS           "audio/x-ac3"
#define XPRIV_AC3_CAPS      "audio/x-private1-ac3"
#define XDD_CAPS            "audio/x-dd"
#define XEAC3_CAPS          "audio/x-eac3"
#define XDDPLUS_CAPS        "audio/x-ddplus"
#define XDTS_CAPS           "audio/x-dts"
#define XPRIV_DTS_CAPS      "audio/x-private1-dts"
#define XWMA_CAPS           "audio/x-wma, wmaversion = (int) { 2 }, "\
  "channels = (int) { 1, 2 }," \
  "rate = (int) { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 96000, 192000 }"


#define GST_TYPE_ISMD_AUDIO_DECODER_OUT_MODE_COMPLEX \
  (ismd_gst_audio_decoder_out_mode_complex_get_type())
static GType
ismd_gst_audio_decoder_out_mode_complex_get_type (void)
{
  static GType ismd_gst_audio_decoder_out_mode_complex_type = 0;
  static const GEnumValue out_mode_types[] = {
    {OUT_PCM, "PCM", "pcm"},
    {OUT_AC3, "Dolby Digital", "ac3"},
    {OUT_DTS, "DTS", "dts"},
    {0, NULL, NULL}
  };

  if (!ismd_gst_audio_decoder_out_mode_complex_type) {
    ismd_gst_audio_decoder_out_mode_complex_type =
        g_enum_register_static ("GstIsmdAudioDecoderOutputModeComplex",
        out_mode_types);
  }
  return ismd_gst_audio_decoder_out_mode_complex_type;
}

#define GST_TYPE_ISMD_AUDIO_DECODER_OUT_SAMPLERATE_COMPLEX \
  (ismd_gst_audio_decoder_out_samplerate_complex_get_type())
static GType
ismd_gst_audio_decoder_out_samplerate_complex_get_type (void)
{
  static GType ismd_gst_audio_decoder_out_samplerate_complex_type = 0;
  static const GEnumValue out_samplerate_types[] = {
      {OUT_SAMPLE_RATE_32K, "32K", "32K"},
      {OUT_SAMPLE_RATE_48K, "48K", "48K"},
      {OUT_SAMPLE_RATE_44100, "44100", "44100"},
      {OUT_SAMPLE_RATE_88200 ,"88200","88200"},
      {OUT_SAMPLE_RATE_96K ,"96000","96000"},
      {OUT_SAMPLE_RATE_176400 ,"176400","176400"},
      {OUT_SAMPLE_RATE_192K ,"192000","192000"},
      {0, NULL, NULL}
  };

  if (!ismd_gst_audio_decoder_out_samplerate_complex_type) {
    ismd_gst_audio_decoder_out_samplerate_complex_type =
        g_enum_register_static ("GstIsmdAudioDecoderOutputSampleRateComplex",
        out_samplerate_types);
  }
  return ismd_gst_audio_decoder_out_samplerate_complex_type;
}

#define GST_TYPE_ISMD_AUDIO_DECODER_OUT_SAMPLESIZE_COMPLEX \
  (ismd_gst_audio_decoder_out_samplesize_complex_get_type())
static GType
ismd_gst_audio_decoder_out_samplesize_complex_get_type (void)
{
  static GType ismd_gst_audio_decoder_out_samplesize_complex_type = 0;
  static const GEnumValue out_samplesize_types[] = {
    {OUT_SAMPLE_SIZE_16, "16", "16"},
    {OUT_SAMPLE_SIZE_20, "20", "20"},
    {OUT_SAMPLE_SIZE_24, "24", "24"},
    {OUT_SAMPLE_SIZE_32, "32", "32"},
    {0, NULL, NULL}
  };

  if (!ismd_gst_audio_decoder_out_samplesize_complex_type) {
    ismd_gst_audio_decoder_out_samplesize_complex_type =
        g_enum_register_static ("GstIsmdAudioDecoderOutputSampleSizeComplex",
        out_samplesize_types);
  }
  return ismd_gst_audio_decoder_out_samplesize_complex_type;
}

#define GST_TYPE_ISMD_AUDIO_DECODER_CHANNEL_CFG_COMPLEX \
    (ismd_gst_audio_decoder_out_channel_cfg_complex_get_type())

static GType
ismd_gst_audio_decoder_out_channel_cfg_complex_get_type(void)
{
    static GType ismd_gst_audio_decoder_out_channel_cfg_complex_type = 0;
    static const GEnumValue channel_cfg_types[] = {
        {OUT_STEREO,"STEREO", "stereo"},
        {OUT_DUAL_MONO, "MONO", "mono"},
        {OUT_5_1, "CH_5.1", "ch_5.1"},
        {OUT_7_1,"CH_7.1", "ch_7.1"},
        {0, NULL, NULL}
    };

    if (!ismd_gst_audio_decoder_out_channel_cfg_complex_type) {
        ismd_gst_audio_decoder_out_channel_cfg_complex_type=
            g_enum_register_static ("GstIsmdAudioDecoderChannelCfgComplex",
                    channel_cfg_types);
    }
    return ismd_gst_audio_decoder_out_channel_cfg_complex_type;
}


/* Functions that get params from GStreamer elements
 * to pass to functions in audio decoder */
static gboolean ismd_start (ISmdGstElement* element);
static gboolean ismd_pause (ISmdGstElement* element);
static inline gboolean ismd_open (GstElement * element);
static inline gboolean ismd_stop (GstElement * element);
static inline gboolean ismd_close (GstElement * element);

static bool  ismd_configure_srcpad_caps(ISmdGstAudioDecoder * audiodec);

GST_DEBUG_CATEGORY_STATIC (ismd_gst_audio_decoder_debug);
#define GST_CAT_DEFAULT ismd_gst_audio_decoder_debug

static void
_do_init (GType ismd_gst_debug_type)
{
  GST_DEBUG_CATEGORY_INIT (ismd_gst_audio_decoder_debug, "ISMD_AUDIO_DECODER",
      (GST_DEBUG_BOLD | GST_DEBUG_FG_GREEN), "ismdgstaudiodecoder element");
}

/* Call Boilerplate to declare the base_init, class_init and init functions
 * and to define the get_type function */
GST_BOILERPLATE_FULL (ISmdGstAudioDecoder, ismd_gst_audio_decoder, ISmdGstElement,
    ISMD_GST_TYPE_ELEMENT, _do_init);

/* This function creates the AAC ADTS Header information to be added for all
 * AAC Audio Buffers */
static inline gboolean
create_aac_header (ISmdGstAudioDecoder * audiodec, GstStructure * structure)
{
  gboolean ret = FALSE;
  gint rate = 0, channels = 0, freq_index = 0x0F;
  gint channel_index = 2, loop, profile = 1;
  guint8 *aac_header_data = &audiodec->aac_header_data[0];
  const GValue *codec_data;

  GST_DEBUG_OBJECT (audiodec, "create_aac_header");

  if (!gst_structure_get_int (structure, "rate", &rate)) {
    codec_data = gst_structure_get_value (structure, "codec_data");
    if (codec_data) {
      GstBuffer *buffer;
      guint8 *data;
      guint16 AudioSpecificConfig;

      buffer = gst_value_get_buffer (codec_data);
      data = GST_BUFFER_DATA (buffer);
      AudioSpecificConfig = (data[0] << 8) | data[1];
      freq_index = (AudioSpecificConfig & 0x0780) >> 7;
      if ( freq_index >= freq_index_max_value ) {
        GST_WARNING_OBJECT (audiodec, "freq_index out of range: %d, max %d",
            freq_index, freq_index_max_value );
        goto beach;
      } else {
        rate = audio_frequency[freq_index].data;
        GST_DEBUG_OBJECT (audiodec,"rate %d", rate);
      }
    } else {
      GST_WARNING_OBJECT (audiodec, "Failed to get rate of the audio packets");
      goto beach;
    }
  }

  if (!gst_structure_get_int (structure, "channels", &channels)) {
    GST_WARNING_OBJECT (audiodec, "Failed to get number of the audio channels");
    goto beach;
  }

  /* AAC - ADTS header */
  aac_header_data[0] = 0xFF;    /* Sync Word */
  aac_header_data[1] = 0xF1;    /* Sync Word=FF, MPEG ID=0, Layer=0 & Protection_Absent=1 */
  aac_header_data[2] = 0x00;    /* Profile, Sample Frequency Index, Channel Configuration */
  aac_header_data[3] = 0x00;    /* Channel Configuration & Frame Length */
  aac_header_data[4] = 0x00;    /* Frame Length */
  aac_header_data[5] = 0x1F;    /* Frame Length & ADTS Buffer Fullness */
  aac_header_data[6] = 0xFC;    /* ADTS Buffer Fullness & Number of Raw data blocks */

  /* Sampling Frequency index */
  for (loop = 0; loop < AAC_FREQUENCY_INDEX; loop++) {
    if (rate == audio_frequency[loop].data) {
      freq_index = audio_frequency[loop].index;
      break;
    }
  }

  /* Audio Channels index */
  for (loop = 0; loop < AAC_CHANNEL_INDEX; loop++) {
    if (channels == audio_channels[loop].data) {
      channel_index = audio_channels[loop].index;
      break;
    }
  }

  /* Update the profile, frequency and audio channels index */
  aac_header_data[2] |= ((profile & 0x03) << 6);
  aac_header_data[2] |= (freq_index << 2);
  aac_header_data[2] |= (channel_index >> 2);
  aac_header_data[3] |= ((channel_index & 0x03) << 6);

  ret = TRUE;
beach:
  return ret;
}


static inline gboolean
ismd_unconfigure_output (ISmdGstAudioDecoder * audiodec)
{
  ismd_result_t result;

  if (audiodec->processor != ISMD_DEV_HANDLE_INVALID && audiodec->output_handle != ISMD_DEV_HANDLE_INVALID) {
    result = ismd_audio_remove_output (audiodec->processor, audiodec->output_handle);
    if (result != ISMD_SUCCESS) {
      GST_ERROR_OBJECT (audiodec, "ismd_audio_remove_output failed (%d)", result);
    }
    audiodec->output_handle = ISMD_DEV_HANDLE_INVALID;
  }
  return TRUE;
}


static inline gboolean
ismd_unconfigure_input (ISmdGstAudioDecoder * audiodec)
{
  ISmdGstElement *smd_element = ISMD_GST_ELEMENT (audiodec);
  ismd_result_t result;

  if (smd_element->dev_handle != ISMD_DEV_HANDLE_INVALID && audiodec->processor != ISMD_DEV_HANDLE_INVALID) {
    result = ismd_audio_remove_input (audiodec->processor, smd_element->dev_handle);
    if (result != ISMD_SUCCESS) {
      GST_ERROR_OBJECT (audiodec, "ismd_audio_remove_input failed (%d)", result);
    }
    smd_element->dev_handle = ISMD_DEV_HANDLE_INVALID;
  }

  return TRUE;
}


static inline int ismd_map_cfg_chl_to_chl_numer(int cfg_chl)
{
    int rvalue = 0;
    switch (cfg_chl){
        case OUT_DUAL_MONO:
            rvalue = 1;
            break;
        case OUT_STEREO: 
            rvalue = 2;
            break;
        case OUT_5_1:
            rvalue = 6;
            break;
        case OUT_7_1:
            rvalue = 8;
            break;
        default:
            rvalue = 2;
            break;
    }
    return rvalue;
}


static inline gboolean
ismd_configure_input (ISmdGstAudioDecoder * audiodec)
{
  gboolean ret = FALSE;
  ISmdGstElement *smd_element = ISMD_GST_ELEMENT (audiodec);
  ismd_result_t result;
  gint pcm_channels;

  GST_DEBUG_OBJECT (audiodec,
      "setting input data format processor=%d devhandle=%d codec=%d",
      audiodec->processor, smd_element->dev_handle, audiodec->codec);

  result = ismd_audio_input_set_data_format (audiodec->processor,
      smd_element->dev_handle, audiodec->codec);
  if (result != ISMD_SUCCESS) {
    GST_ERROR ("ismd_audio_input_set_data_format failed(%d)", result);
    goto beach;
  }

  switch (audiodec->codec) {
    case ISMD_AUDIO_MEDIA_FMT_WM9:
      GST_DEBUG_OBJECT (audiodec, "setting WMA input data format");
      result = ismd_audio_input_set_wma_format (audiodec->processor,
          smd_element->dev_handle, audiodec->wma_fmt);
      if (result != ISMD_SUCCESS) {
        GST_ERROR_OBJECT (audiodec, "ismd_audio_input_set_wma_format failed(%d)",
            result);
        goto beach;
      }
      break;
    case ISMD_AUDIO_MEDIA_FMT_PCM:

      GST_DEBUG_OBJECT (audiodec, "setting PCM input data format");
      switch (audiodec->channels) {
        case 1:
          pcm_channels = AUDIO_CHAN_CONFIG_1_CH;
          break;
        case 2:
          pcm_channels = AUDIO_CHAN_CONFIG_2_CH;
          break;
        case 6:
          pcm_channels = AUDIO_CHAN_CONFIG_6_CH;
          break;
        case 8:
          pcm_channels = AUDIO_CHAN_CONFIG_8_CH;
          break;
        default:
          pcm_channels = AUDIO_CHAN_CONFIG_2_CH;
          break;
      }
      result = ismd_audio_input_set_pcm_format (audiodec->processor,
          smd_element->dev_handle, audiodec->width, audiodec->rate,
          pcm_channels);
      if (result != ISMD_SUCCESS) {
          GST_ERROR_OBJECT (audiodec, "ismd_audio_input_set_pcm_format failed(%d)",
                  result);
          goto beach;
      }
      break;
    default:
      break;
  }

  ret = TRUE;
beach:
  return ret;
}


static inline gboolean
ismd_configure_output (ISmdGstAudioDecoder * audiodec)
{
  ismd_result_t result = ISMD_SUCCESS;
  ismd_audio_output_config_t out_config;
  ISmdGstElement *smd_element = ISMD_GST_ELEMENT (audiodec);
    
  out_config.stream_delay = audiodec->out_delay;
  out_config.sample_rate = audiodec->out_sample_rate;
  out_config.sample_size = audiodec->out_sample_size;
  out_config.out_mode = audiodec->out_mode;
  out_config.ch_config = audiodec->ch_config;
  out_config.ch_map = 0;

  GST_INFO_OBJECT (audiodec,
          "Configuring %d delay=%d size=%d chconf=0x%x mode=%d rate=%d",
          smd_element->ismd_port, out_config.stream_delay, out_config.sample_size,
          out_config.ch_config, out_config.out_mode, out_config.sample_rate);
  GST_INFO_OBJECT (audiodec, "going to reconfigure shared out port");

  result = ismd_audio_output_disable (audiodec->processor, audiodec->output_handle);
  if (result != ISMD_SUCCESS) {
      GST_ERROR_OBJECT (audiodec, "ismd_audio_output_disable  failed(%d)", result);
      goto beach;
  }

  result = ismd_audio_output_set_channel_config (audiodec->processor,
          audiodec->output_handle, out_config.ch_config);
  if (result != ISMD_SUCCESS) {
      GST_ERROR_OBJECT (audiodec, "ismd_audio_output_set_channel_config failed(%d)",
              result);
      goto beach;
  }

  result = ismd_audio_output_set_sample_size (audiodec->processor,
          audiodec->output_handle, out_config.sample_size);
  if (result != ISMD_SUCCESS) {
      GST_ERROR_OBJECT (audiodec, "ismd_audio_output_set_sample_size failed(%d)",
              result);
      goto beach;
  }

  result = ismd_audio_output_set_sample_rate (audiodec->processor,
          audiodec->output_handle, out_config.sample_rate);
  if (result != ISMD_SUCCESS) {
      GST_ERROR_OBJECT (audiodec, "ismd_audio_output_set_sample_rate failed(%d)",
              result);
      goto beach;
  }

  result = ismd_audio_output_set_mode (audiodec->processor,
          audiodec->output_handle, out_config.out_mode);
  if (result != ISMD_SUCCESS) {
      GST_ERROR_OBJECT (audiodec, "ismd_audio_output_set_mode failed(%d)",
              result);
      goto beach;
  }

  if (out_config.stream_delay) {
      result = ismd_audio_output_set_delay (audiodec->processor,
              audiodec->output_handle, out_config.stream_delay);
      if (result != ISMD_SUCCESS) {
          GST_WARNING_OBJECT (audiodec, "ismd_audio_output_set_delay failed(%d) maybe"
                  " smd_buffers_AUD_PER_OUTPUT_DELAY is missing in memory layout",
                  result);
          result = ISMD_SUCCESS;
      }
  }

beach:
  return (result == ISMD_SUCCESS) ? TRUE : FALSE;
}


static inline GstPadTemplate *
get_sink_pad_template ()
{
  GstCaps *caps = NULL;

  caps = gst_caps_from_string (PCM_CAPS);
  GST_INFO ("Added %s", PCM_CAPS);

  /* Check for LPCM from mpeg PS */
  if (ismd_audio_codec_available (ISMD_AUDIO_MEDIA_FMT_DVD_PCM) == ISMD_SUCCESS) {
    gst_caps_append (caps, gst_caps_from_string (XPRIV_LPCM_CAPS));
    GST_INFO ("Added %s", XPRIV_LPCM_CAPS);
  }

  /* Check for mpeg 1 or mpeg 2 bc */
  if (ismd_audio_codec_available (ISMD_AUDIO_MEDIA_FMT_MPEG) == ISMD_SUCCESS) {
    gst_caps_append (caps, gst_caps_from_string (MPEG12_CAPS));
    GST_INFO ("Added %s", MPEG12_CAPS);
    gst_caps_append (caps, gst_caps_from_string (XMPEG12_CAPS));
    GST_INFO ("Added %s", XMPEG12_CAPS);
  }

  /* Check for mpeg 2 or mpeg 4 aac */
  if (ismd_audio_codec_available (ISMD_AUDIO_MEDIA_FMT_AAC) == ISMD_SUCCESS) {
    gst_caps_append (caps, gst_caps_from_string (MPEG4_CAPS));
    GST_INFO ("Added %s", MPEG4_CAPS);
    gst_caps_append (caps, gst_caps_from_string (XMPEG4_CAPS));
    GST_INFO ("Added %s", XMPEG4_CAPS);
    gst_caps_append (caps, gst_caps_from_string (XAAC_CAPS));
    GST_INFO ("Added %s", XAAC_CAPS);
  }

  /* Check for ac3 */
  if (ismd_audio_codec_available (ISMD_AUDIO_MEDIA_FMT_DD) == ISMD_SUCCESS) {
    gst_caps_append (caps, gst_caps_from_string (XAC3_CAPS));
    GST_INFO ("Added %s", XAC3_CAPS);
    gst_caps_append (caps, gst_caps_from_string (XPRIV_AC3_CAPS));
    GST_INFO ("Added %s", XPRIV_AC3_CAPS);
    gst_caps_append (caps, gst_caps_from_string (XDD_CAPS));
    GST_INFO ("Added %s", XDD_CAPS);
  }

  /* Check for eac3/dolby digital+ */
  if (ismd_audio_codec_available (ISMD_AUDIO_MEDIA_FMT_DD_PLUS) == ISMD_SUCCESS) {
    gst_caps_append (caps, gst_caps_from_string (XEAC3_CAPS));
    GST_INFO ("Added %s", XEAC3_CAPS);
    gst_caps_append (caps, gst_caps_from_string (XDDPLUS_CAPS));
    GST_INFO ("Added %s", XDDPLUS_CAPS);
  }

  /* Check for dolby TrueHD */
  if (ismd_audio_codec_available (ISMD_AUDIO_MEDIA_FMT_TRUE_HD) == ISMD_SUCCESS) {
  }

  /* Check for DTS */
  if (ismd_audio_codec_available (ISMD_AUDIO_MEDIA_FMT_DTS) == ISMD_SUCCESS) {
    gst_caps_append (caps, gst_caps_from_string (XDTS_CAPS));
    GST_INFO ("Added %s", XDTS_CAPS);
    gst_caps_append (caps, gst_caps_from_string (XPRIV_DTS_CAPS));
    GST_INFO ("Added %s", XPRIV_DTS_CAPS);
  }

  /* Check for WMA */
  if (ismd_audio_codec_available (ISMD_AUDIO_MEDIA_FMT_WM9) == ISMD_SUCCESS) {
    gst_caps_append (caps, gst_caps_from_string (XWMA_CAPS));
    GST_INFO ("Added %s", XWMA_CAPS);
  }

  return gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS, caps);
}


static void
ismd_gst_audio_decoder_base_init (gpointer g_class)
{
  static const GstElementDetails ismd_gst_audio_decoder_details =
      GST_ELEMENT_DETAILS
      ("Intel Streaming Media Driver (ISMD) Hardware Audio decoder",
      "Codec/decoder/Audio",
      "GStreamer Audio decoder Element for Intel's Media Processors",
      "http://www.intelconsumerelectronics.com/");

  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (g_class);

  GstPadTemplate *templ = get_sink_pad_template();
  gst_element_class_add_pad_template (gstelement_class, templ);
  gst_object_unref (templ); 

  templ = gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
            gst_static_caps_get (&srccap));

  gst_element_class_add_pad_template (gstelement_class, templ);
  gst_object_unref (templ); 

  gst_element_class_set_details (gstelement_class,
      &ismd_gst_audio_decoder_details);
}


static inline gboolean
ismd_reconfig_output_parameter(ISmdGstAudioDecoder * audiodec, guint prop_id, guint out_value)
{
  ISmdGstElement *smd_element = ISMD_GST_ELEMENT (audiodec);
  GstState current_state = GST_STATE (smd_element);
  gboolean is_playing = FALSE;

  /*audio processor has not opened yet */
  if (audiodec->processor == -1 || audiodec->output_handle == -1 ){
      return TRUE;
  }

  if (current_state == GST_STATE_PLAYING) {
      is_playing = TRUE;
      ismd_pause(smd_element);
      ismd_dev_set_state (smd_element->dev_handle, ISMD_DEV_STATE_STOP);
  }
  if (!ismd_configure_output (audiodec))
      goto beach;

  if (!ismd_configure_srcpad_caps(audiodec))
      goto beach;

beach:
  if (is_playing) {
      ismd_start(smd_element);
      ismd_dev_set_state (smd_element->dev_handle, ISMD_DEV_STATE_PLAY);
  }
  return TRUE;
}


static void
ismd_gst_audio_decoder_dispose (GObject * object)
{
  ISmdGstAudioDecoder *audiodec;

  audiodec = ISMD_GST_AUDIO_DECODER (object);
  G_OBJECT_CLASS (parent_class)->dispose (object);
}


static void
ismd_gst_audio_decoder_finalize (GObject * object)
{
  ISmdGstAudioDecoder *audiodec;
  audiodec = ISMD_GST_AUDIO_DECODER (object);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}


static bool 
ismd_configure_srcpad_caps(ISmdGstAudioDecoder * audiodec)
{
    GstPad *srcpad = NULL;
    GstCaps *srccaps = NULL;
    int channel_num = 0;
    bool ret = TRUE;
    //set the src pad caps
    channel_num = ismd_map_cfg_chl_to_chl_numer(audiodec->ch_config);
    srcpad = gst_element_get_static_pad (GST_ELEMENT (audiodec), "src_1");
    if (audiodec->out_mode ==  OUT_PCM){
        srccaps = gst_caps_new_simple ("audio/x-raw-int",
                "width", G_TYPE_INT, audiodec->out_sample_size,
                "rate", G_TYPE_INT, audiodec->out_sample_rate,
                "channels", G_TYPE_INT, channel_num,
                "endianness", G_TYPE_INT,LITTLE_ENDIAN,
                "signed", G_TYPE_BOOLEAN,true,
                NULL);
    } else if (audiodec->out_mode ==  OUT_AC3){ 
        srccaps = gst_caps_new_simple ("audio/x-ac3",
                "width", G_TYPE_INT, audiodec->out_sample_size,
                "rate", G_TYPE_INT, audiodec->out_sample_rate,
                "channels", G_TYPE_INT, channel_num,
                NULL);

    } else if  (audiodec->out_mode ==  OUT_DTS){
        srccaps = gst_caps_new_simple ("audio/x-dts",
                "width", G_TYPE_INT, audiodec->out_sample_size,
                "rate", G_TYPE_INT, audiodec->out_sample_rate,
                "channels", G_TYPE_INT, channel_num,
                NULL);
    }

    if (gst_pad_peer_accept_caps (srcpad, srccaps)) {
        GST_DEBUG_OBJECT (audiodec, "setting caps downstream to %" GST_PTR_FORMAT,
                srccaps);
        ret = gst_pad_set_caps (srcpad, srccaps);
        if (G_UNLIKELY (!ret)) {
            GST_ERROR_OBJECT (audiodec, "failed setting downstream caps to %"
                    GST_PTR_FORMAT, srccaps);
        }
    }else {
        ret = FALSE;
    }

    gst_caps_unref (srccaps);
    gst_object_unref (srcpad);
    return ret;
}


static GstPadLinkReturn
ismd_gst_audio_decoderpad_setcaps (GstPad * pad, GstCaps * caps)
{
  gboolean ret = FALSE;
  GstStructure *structure;
  ISmdGstAudioDecoder *audiodec;
  ISmdGstElement *smd_element;
  const gchar *name = NULL;
  gboolean is_playing = FALSE;
  GstState current_state;
  gint mpegversion = 0;
  gint layer = 0;

  audiodec = ISMD_GST_AUDIO_DECODER (gst_pad_get_parent (pad));
  smd_element = ISMD_GST_ELEMENT (audiodec);
  structure = gst_caps_get_structure (caps, 0);
  name = gst_structure_get_name (structure);
  current_state = GST_STATE (smd_element);

  GST_INFO_OBJECT (pad, "setcaps called with %" GST_PTR_FORMAT, caps);

  if (current_state == GST_STATE_PLAYING) {
    GST_DEBUG_OBJECT (audiodec, "audio driver is in PLAY state");
    is_playing = TRUE;
    ismd_dev_set_state (smd_element->dev_handle, ISMD_DEV_STATE_PAUSE);
  }

  if (!gst_structure_get_int (structure, "width", &audiodec->width))
    audiodec->width = 16;
  if (!gst_structure_get_int (structure, "rate", &audiodec->rate))
    audiodec->rate = 48000;
  if (!gst_structure_get_int (structure, "channels", &audiodec->channels))
    audiodec->channels = 2;
  if (!gst_structure_get_int (structure, "bitrate", &audiodec->bitrate))
    audiodec->bitrate = -1;

  if (!strcmp (name, "audio/x-raw-int")) {
    audiodec->codec = ISMD_AUDIO_MEDIA_FMT_PCM;
  } else if (!strcmp (name, "audio/x-private1-lpcm")) {
    audiodec->codec = ISMD_AUDIO_MEDIA_FMT_DVD_PCM;
  } else if (!strcmp (name, "audio/x-eac3")) {
    audiodec->codec = ISMD_AUDIO_MEDIA_FMT_DD_PLUS;
  } else if (!strcmp (name, "audio/x-ddplus")) {
    audiodec->codec = ISMD_AUDIO_MEDIA_FMT_DD_PLUS;
  } else if (!strcmp (name, "audio/x-private1-dts") ||
      !strcmp (name, "audio/x-dts")) {
    audiodec->codec = ISMD_AUDIO_MEDIA_FMT_DTS;
  } else if (!strcmp (name, "audio/mpeg") || !strcmp (name, "audio/x-mpeg")) {
    gst_structure_get_int (structure, "mpegversion", &mpegversion);
    gst_structure_get_int (structure, "layer", &layer);
    /* NOTE: MPEG audio decoder can detect the layer */
    switch (mpegversion) {
      case 1:
        audiodec->codec = ISMD_AUDIO_MEDIA_FMT_MPEG;
        break;
      case 2:
      case 4:
        {
          const gchar * stream_format = NULL;
          stream_format = gst_structure_get_string (structure, "stream-format");
          gboolean is_loas = FALSE;
          if (stream_format) {
            if (!strcmp (stream_format, "loas") ||
                  !strcmp (stream_format, "latm")) {
              is_loas = TRUE;
            }
          }
          if (is_loas) {
            audiodec->codec = ISMD_AUDIO_MEDIA_FMT_AAC_LOAS;
            audiodec->first_aac_packet = FALSE;
            break;
          }
          audiodec->codec = ISMD_AUDIO_MEDIA_FMT_AAC;
          if (stream_format && !strcmp (stream_format, "adts")) {
              audiodec->first_aac_packet = FALSE;
              break;
          }
          if (audiodec->first_aac_packet == FALSE) {
              if (create_aac_header (audiodec, structure)) {
                  audiodec->first_aac_packet = TRUE;
              }
          }
        }
        break;
      default:
        GST_ERROR_OBJECT (audiodec, "Invalid MPEG version\n");
        goto beach;
    }
  } else if (!strcmp (name, "audio/x-aac")) {
    audiodec->codec = ISMD_AUDIO_MEDIA_FMT_AAC;
  } else if (!strcmp (name, "audio/x-ac3") || !strcmp (name, "audio/x-dd") ||
      !strcmp (name, "audio/x-private1-ac3")) {
    audiodec->codec = ISMD_AUDIO_MEDIA_FMT_DD;
  } else if (!strcmp (name, "audio/x-wma")) {
      gint wmaversion;
      const GValue *codec_data;

      gst_structure_get_int (structure, "wmaversion", &wmaversion);

      /* This is only for wma version = 2  (wma1 = 0x160, ...) */
      audiodec->wma_fmt.format_tag = 0x161;
      audiodec->wma_fmt.sample_rate = audiodec->rate;
      audiodec->wma_fmt.num_channels = audiodec->channels;
      audiodec->wma_fmt.bitrate = audiodec->bitrate / 8;
      audiodec->wma_fmt.sample_size = audiodec->width;
      gst_structure_get_int (structure, "block_align",
              &audiodec->wma_fmt.block_align);
      codec_data = gst_structure_get_value (structure, "codec_data");

      if (codec_data) {
          GstBuffer *buffer;
          guint8 *data;

          buffer = gst_value_get_buffer (codec_data);
          data = GST_BUFFER_DATA (buffer);
          audiodec->wma_fmt.encode_option = data[3] << 8 | data[4];
      } else {
          GST_ERROR_OBJECT (audiodec, "encoding information missing in codec_data");
          goto beach;
      }
      audiodec->codec = ISMD_AUDIO_MEDIA_FMT_WM9;
  } else {
      GST_ERROR_OBJECT (audiodec, "unrecognized/unsupported audio format");
      goto beach;
  }

  if (!ismd_configure_input (audiodec))
      goto beach;

  if (!ismd_configure_output (audiodec))
      goto beach;

  if (!ismd_configure_srcpad_caps(audiodec))
      goto beach;

  ismd_start(smd_element);
  ismd_dev_set_state (smd_element->dev_handle, ISMD_DEV_STATE_PLAY);
  ret = TRUE;
beach:
  gst_object_unref (audiodec);
  return ret;
}

static gboolean
ismd_start (ISmdGstElement * element)
{
    ISmdGstElement *parent = ISMD_GST_ELEMENT (element);
    ismd_result_t result;
    ISmdGstAudioDecoder *audio = ISMD_GST_AUDIO_DECODER(element);

    if (parent->dev_handle == ISMD_DEV_HANDLE_INVALID || audio->output_handle == ISMD_DEV_HANDLE_INVALID) {
        return FALSE;
    }

    result=ismd_audio_output_enable(audio->processor, audio->output_handle);
    if (result != ISMD_SUCCESS) {
        return FALSE;
    }
    return TRUE;
}


static gboolean
ismd_pause (ISmdGstElement * element)
{
    ISmdGstElement *parent = ISMD_GST_ELEMENT (element);
    ismd_result_t result;
    ISmdGstAudioDecoder *audio = ISMD_GST_AUDIO_DECODER(element);

    if (parent->dev_handle == ISMD_DEV_HANDLE_INVALID || audio->output_handle == ISMD_DEV_HANDLE_INVALID) {
        return FALSE;
    }

    result=ismd_audio_output_disable(audio->processor, audio->output_handle);
    if (result != ISMD_SUCCESS) {
        return FALSE;
    }
    return TRUE;
}


static GstStateChangeReturn
ismd_gst_audio_decoder_change_state (GstElement * element,
    GstStateChange transition)
{
  ISmdGstElement *smd_element = ISMD_GST_ELEMENT (element);
  ISmdGstAudioDecoder *audiodec = ISMD_GST_AUDIO_DECODER (element);
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

  GST_DEBUG_OBJECT (audiodec, "going to  change state from %s to %s",
      gst_element_state_get_name (GST_STATE_TRANSITION_CURRENT (transition)),
      gst_element_state_get_name (GST_STATE_TRANSITION_NEXT (transition)));


  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
    {
      if (ismd_open (element) != TRUE) {
        GST_ERROR_OBJECT (audiodec, "audio decoder(%d): ismd_open failed",
            smd_element->dev_handle);
        goto failure;
      }
      break;
    }
    case GST_STATE_CHANGE_READY_TO_PAUSED:
    {
      break;
    }
    case GST_STATE_CHANGE_READY_TO_NULL:
    {
      if (smd_element->dev_handle == ISMD_DEV_HANDLE_INVALID) {
        GST_ERROR_OBJECT (audiodec,
            "audio decoder(%d): audio decoder is not open",
            smd_element->dev_handle);
        goto failure;
      }

      if (ismd_close (element) != TRUE) {
        GST_ERROR_OBJECT (audiodec, "audio decoder(%d): ismd_close failed",
            smd_element->dev_handle);
        goto failure;
      }
      break;
    }
    default:
      break;
  }

  /* Chain up to the parent class's state change function
   * Must do this before downward transitions are handled to safely handle
   * concurrent access by multiple threads */
  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    GST_ERROR_OBJECT (audiodec, "audio decoder(%d): change state failure",
        smd_element->dev_handle);
    goto failure;
  }

  switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
        {
            GST_DEBUG_OBJECT(audiodec,"-- audio decoder(%d): PLAYING TO PAUSED entered\n",
                    smd_element->dev_handle);
            if (smd_element->dev_handle == ISMD_DEV_HANDLE_INVALID) {
                GST_ERROR_OBJECT(audiodec, "-- audio decoder(%d): PLAYING_TO_PAUSED - audio decoder is not open\n",
                        smd_element->dev_handle);
                goto failure;
            }
            if (ismd_pause (smd_element) != TRUE) {
                GST_ERROR_OBJECT(audiodec, "-- audio decoder(%d): ismd_pause failed.\n",
                        smd_element->dev_handle);
                goto failure;
            }
            break;
        }
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
        if (smd_element->dev_handle == ISMD_DEV_HANDLE_INVALID) {
            GST_ERROR_OBJECT(audiodec, "-- audio decoder(%d): PLAYING_TO_PAUSED - audio decoder is not open\n",
                    smd_element->dev_handle);
            goto failure;
        }
        if (ismd_start (smd_element) != TRUE) {
            GST_ERROR_OBJECT(audiodec, "-- audio decoder(%d): ismd_start failed.\n",
                    smd_element->dev_handle);
            goto failure;
        }
        break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
    {
      if (ismd_stop (element) != TRUE) {
        GST_ERROR_OBJECT (audiodec, "audio decoder(%d): ismd_stop failed.",
            smd_element->dev_handle);
        goto failure;
      }
      break;
    }
    default:
      break;
  }

  GST_DEBUG_OBJECT (audiodec, "changed state from %s to %s",
      gst_element_state_get_name (GST_STATE_TRANSITION_CURRENT (transition)),
      gst_element_state_get_name (GST_STATE_TRANSITION_NEXT (transition)));

  return ret;

failure:
  GST_ERROR_OBJECT (audiodec, "change state failed");
  return GST_STATE_CHANGE_FAILURE;
}

/**
 * ismd_open
 * Open the audio decoder and get the port
 * @element - element containing audioaudiodec
 * returns TRUE if setup is successful
 */
static inline gboolean
ismd_open (GstElement * element)
{
    ismd_result_t result = ISMD_SUCCESS;
    ismd_port_handle_t port = -1;
    ISmdGstAudioDecoder *audiodec = ISMD_GST_AUDIO_DECODER (element);
    ISmdGstElement *smd_element = ISMD_GST_ELEMENT (element);
    gboolean ret = FALSE;
    gboolean timed = audiodec->sync;
    ISmdGstPad *srcpad;
    GList *padlist;

    if (audiodec->use_global_proc) {
        GST_INFO_OBJECT (audiodec, "Opening global audio processor");
        result = ismd_audio_open_global_processor (&audiodec->processor);
    } else {
        GST_INFO_OBJECT (audiodec, "Opening regular audio processor");
        result = ismd_audio_open_processor (&audiodec->processor);
    }
    if (result != ISMD_SUCCESS) {
        audiodec->processor = ISMD_DEV_HANDLE_INVALID;
        GST_ERROR_OBJECT (audiodec, "ismd_audio_open_*_processor failed (%d)", result);
        goto beach;
    }

    GST_DEBUG_OBJECT (audiodec, "audio processor %d opened", audiodec->processor);

    result = ismd_audio_add_input_port (audiodec->processor, timed,
            &smd_element->dev_handle, &port);
    if (result != ISMD_SUCCESS) {
        GST_ERROR_OBJECT (audiodec, "ismd_audio_add_input_port failed (%d)", result);
        goto error;
    }
    GST_DEBUG_OBJECT (audiodec, "ismd_audio_add_input_port(proc=%d, timed=%d,"
            " aud_dev_handle=%d, aud_port=%d) succeded",
            audiodec->processor, timed, smd_element->dev_handle, port);
    (smd_element->sink_pad)->ismd_port = port;


    // add the output port  port
    padlist = (smd_element->src_pads); 
    srcpad = ISMD_GST_PAD (padlist->data);
    ismd_audio_output_config_t out_config;
    out_config.stream_delay = audiodec->out_delay;
    out_config.sample_rate = audiodec->out_sample_rate;
    out_config.sample_size = audiodec->out_sample_size;
    out_config.out_mode = audiodec->out_mode;
    out_config.ch_config = audiodec->ch_config;
    out_config.ch_map = 0;

    result = ismd_audio_add_output_port(audiodec->processor, out_config,
            &audiodec->output_handle, &srcpad->ismd_port);
    smd_element->ismd_port = srcpad->ismd_port;
    ret= TRUE;
beach:
    return ret;

error:
    if (!ismd_close (element)) {
        GST_ERROR_OBJECT (audiodec, "ismd_close() failed.");
    }
    return FALSE;
}

/**
 * ismd_stop
 * Stop the audio decoder
 * @element - element containing audioaudiodec
 * returns TRUE if audio decoder stop was successful
 */
    static inline gboolean
ismd_stop (GstElement * element)
{
    ISmdGstAudioDecoder *audiodec = ISMD_GST_AUDIO_DECODER (element);
    ISmdGstElement *parent = ISMD_GST_ELEMENT (element);
    GST_DEBUG_OBJECT (audiodec, "ismd_stop()");

    if (parent->dev_handle == ISMD_DEV_HANDLE_INVALID) {
        GST_ERROR_OBJECT( audiodec,"audio decoder: audio is not open. So cannot be stopped.\n");
        return FALSE;
    }

    ismd_dev_flush(parent->dev_handle);
    audiodec->first_aac_packet = FALSE;
    return TRUE;
}

/**
 * ismd_close
 * Close resources
 * @element - element containing audioaudiodec
 * returns TRUE if closing resources was successful
 */
static inline gboolean
ismd_close (GstElement * element)
{
    ISmdGstAudioDecoder *audiodec = ISMD_GST_AUDIO_DECODER (element);
    ISmdGstElement *ismd_element = ISMD_GST_ELEMENT (element);

    if (audiodec->processor != ISMD_DEV_HANDLE_INVALID) {
        ismd_unconfigure_output(audiodec);
        ismd_unconfigure_input (audiodec);
        ismd_audio_close_processor (audiodec->processor);
        audiodec->processor = ISMD_DEV_HANDLE_INVALID;
        ismd_element->sink_pad->is_event_queue_attached = FALSE;
    }
    return TRUE;
}


static void
ismd_gst_audio_decoder_get_property (GObject * object, guint prop_id,
        GValue * value, GParamSpec * pspec)
{
    ISmdGstAudioDecoder *audiodec = ISMD_GST_AUDIO_DECODER (object);

    switch (prop_id) {
        case PROP_AUDIO_GLOBAL_PROC:
            g_value_set_boolean (value, audiodec->use_global_proc);
            break;
        case PROP_AUDIO_SAMPLE_SIZE:
            g_value_set_enum(value, audiodec->out_sample_size);
            break;
        case PROP_AUDIO_SAMPLE_RATE:
            g_value_set_enum(value, audiodec->out_sample_rate);
            break;
        case PROP_AUDIO_CH_CONFIG:
            g_value_set_enum(value, audiodec->ch_config);
            break;
        case PROP_AUDIO_OUT_MODE:
            g_value_set_enum(value, audiodec->out_mode);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
ismd_gst_audio_decoder_set_property (GObject * object, guint prop_id,
        const GValue * value, GParamSpec * pspec)
{
    ISmdGstAudioDecoder *audiodec = ISMD_GST_AUDIO_DECODER (object);
    guint out_value = 0;

    switch (prop_id) {
        case PROP_AUDIO_GLOBAL_PROC:
            audiodec->use_global_proc = g_value_get_boolean (value);
            GST_DEBUG_OBJECT (audiodec, "use global processor set to %d",
                    audiodec->use_global_proc);
            break;
        case PROP_AUDIO_SAMPLE_SIZE:
            out_value = g_value_get_enum(value);
            printf("PROP_AUDIO_SAMPLE_SIZE :%d\n", out_value);
            if (out_value != audiodec->out_sample_size){
                audiodec->out_sample_size = out_value;
                ismd_reconfig_output_parameter(audiodec, prop_id, out_value);
            }
            break;
        case PROP_AUDIO_SAMPLE_RATE:
            out_value = g_value_get_enum(value);
            if (out_value != audiodec->out_sample_rate){
                audiodec->out_sample_rate = out_value;
                ismd_reconfig_output_parameter(audiodec, prop_id, out_value);
            }
            break;
        case PROP_AUDIO_CH_CONFIG:
            out_value = g_value_get_enum(value);
            if (out_value != audiodec->ch_config){
                audiodec->ch_config = out_value;
                ismd_reconfig_output_parameter(audiodec, prop_id, out_value);
            }
            break;
        case PROP_AUDIO_OUT_MODE:
            out_value = g_value_get_enum(value);
            if (out_value != audiodec->out_mode){
                audiodec->out_mode = out_value;
                ismd_reconfig_output_parameter(audiodec, prop_id, out_value);
            }
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static GstBuffer *
ismd_gst_audio_decoder_convert (ISmdGstElement * smd_element, GstBuffer * buf,
        gboolean need_seq_hdr)
{
    ISmdGstAudioDecoder *audiodec = ISMD_GST_AUDIO_DECODER (smd_element);
    GstBuffer *convbuf = NULL;
    int len = AAC_ADTS_HEADER_SIZE + GST_BUFFER_SIZE (buf);
   
    if (audiodec->first_aac_packet == FALSE)
        goto end;

    convbuf = gst_buffer_new_and_alloc (len);
    if (convbuf == NULL) {
        GST_ERROR_OBJECT (audiodec, "Failed to create GST Buffer");
        goto end;
    }
    memset (convbuf->data, 0x00, len);
    memcpy (convbuf->data, audiodec->aac_header_data, AAC_ADTS_HEADER_SIZE);
    memcpy (convbuf->data + AAC_ADTS_HEADER_SIZE, GST_BUFFER_DATA (buf),
            GST_BUFFER_SIZE (buf));
    GST_BUFFER_SIZE (convbuf) = len;

    /* Update the frame length */
    convbuf->data[3] |= ((len >> 11) & 0x03);
    convbuf->data[4] |= ((len >> 3) & 0xFF);
    convbuf->data[5] |= ((len & 0x07) << 5);

    GST_BUFFER_TIMESTAMP (convbuf) = GST_BUFFER_TIMESTAMP (buf);
    GST_BUFFER_FLAGS (convbuf) = GST_BUFFER_FLAGS (buf);
    gst_buffer_set_caps (convbuf, buf->caps);

    gst_buffer_unref (buf);
    buf = convbuf;

end:
    return buf;
}


static void
ismd_gst_audio_decoder_class_init (ISmdGstAudioDecoderClass * klass)
{
    ISmdGstElementClass *iclass = ISMD_GST_ELEMENT_CLASS (klass);
    GstElementClass *eclass = GST_ELEMENT_CLASS (klass);
    GObjectClass *oclass = G_OBJECT_CLASS (klass);

    oclass->get_property = ismd_gst_audio_decoder_get_property;
    oclass->set_property = ismd_gst_audio_decoder_set_property;

    g_object_class_install_property (oclass,
            PROP_AUDIO_GLOBAL_PROC,
            g_param_spec_boolean ("global-processor",
                "global processor",
                "Use the global audio procesor",
                DEFAULT_ISMD_AUDIO_DECODER_GLOBAL_PROC, G_PARAM_READWRITE));

    g_object_class_install_property (oclass,
            PROP_AUDIO_SAMPLE_SIZE,
            g_param_spec_enum("audio-output-sample-size",
                "sample size",
                "the sample size of the audio output",
                GST_TYPE_ISMD_AUDIO_DECODER_OUT_SAMPLESIZE_COMPLEX,
                DEFAULT_ISMD_AUDIO_DECODER_SAMPLE_SIZE, G_PARAM_READWRITE));

    g_object_class_install_property (oclass,
            PROP_AUDIO_SAMPLE_RATE,
            g_param_spec_enum("audio-output-sample-rate",
                "sample rate",
                "the sample rate of the audio output",
                GST_TYPE_ISMD_AUDIO_DECODER_OUT_SAMPLERATE_COMPLEX,
                DEFAULT_ISMD_AUDIO_DECODER_SAMPLE_RATE, G_PARAM_READWRITE));

    g_object_class_install_property (oclass,
            PROP_AUDIO_CH_CONFIG,
            g_param_spec_enum ("audio-channel-config",
                "audio output channel config",
                "Define the audio output channelconfigure",
                GST_TYPE_ISMD_AUDIO_DECODER_CHANNEL_CFG_COMPLEX,
                DEFAULT_ISMD_AUDIO_DECODER_CHANNEL_CFG, G_PARAM_READWRITE));

    g_object_class_install_property (oclass,
            PROP_AUDIO_OUT_MODE,
            g_param_spec_enum ("audio-output-mode",
                "audio output file mode",
                "Define the audio output file mode",
                GST_TYPE_ISMD_AUDIO_DECODER_OUT_MODE_COMPLEX,
                DEFAULT_ISMD_AUDIO_DECODER_OUT_MODE, G_PARAM_READWRITE));


    oclass->dispose = GST_DEBUG_FUNCPTR (ismd_gst_audio_decoder_dispose);
    oclass->finalize = GST_DEBUG_FUNCPTR (ismd_gst_audio_decoder_finalize);
    eclass->change_state = GST_DEBUG_FUNCPTR (ismd_gst_audio_decoder_change_state);

    iclass->convert = ismd_gst_audio_decoder_convert;
    iclass->is_decoder = TRUE;
    iclass->is_sink = FALSE;
    iclass->is_audio_decoder = TRUE;
}


static void
ismd_gst_audio_decoder_init (ISmdGstAudioDecoder * audiodec,
        ISmdGstAudioDecoderClass * g_class)
{
    GstPad *pad = NULL;
    GstPadTemplate *templ = NULL;
    ISmdGstPad *srcpad = NULL;
    ISmdGstElement *smd_element = ISMD_GST_ELEMENT (audiodec);

    //sink pad
    templ = get_sink_pad_template();
    smd_element->sink_pad = ismd_gst_pad_new_from_template (templ, "sink");
    gst_object_unref (templ);
    pad = GST_PAD (smd_element->sink_pad);
    gst_element_add_pad (GST_ELEMENT (smd_element), pad);
    ismd_gst_element_register_sinkpad(smd_element, smd_element->sink_pad);
    gst_pad_set_setcaps_function (pad,
            GST_DEBUG_FUNCPTR (ismd_gst_audio_decoderpad_setcaps));
    //src pad. 
    templ = gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
            gst_static_caps_get (&srccap));
    srcpad = ismd_gst_pad_new_from_template (templ, "src_1");
    ismd_gst_element_register_srcpad (smd_element, srcpad);
    gst_element_add_pad (GST_ELEMENT (smd_element), GST_PAD (srcpad));
    gst_object_unref (templ);

    /* Initialize to invalid port id - to prevent buffer passing */
    (smd_element->sink_pad)->ismd_port = -1;

    audiodec->processor = ISMD_DEV_HANDLE_INVALID;
    audiodec->sync = DEFAULT_ISMD_AUDIO_DECODER_SYNC;
    audiodec->use_global_proc = DEFAULT_ISMD_AUDIO_DECODER_GLOBAL_PROC;
    audiodec->out_delay = DEFAULT_ISMD_AUDIO_DECODER_DELAY;
    audiodec->out_sample_size  = DEFAULT_ISMD_AUDIO_DECODER_SAMPLE_SIZE;
    audiodec->out_sample_rate = DEFAULT_ISMD_AUDIO_DECODER_SAMPLE_RATE;
    audiodec->ch_config = DEFAULT_ISMD_AUDIO_DECODER_CHANNEL_CFG;
    audiodec->out_mode = DEFAULT_ISMD_AUDIO_DECODER_OUT_MODE;
    audiodec->output_handle = ISMD_DEV_HANDLE_INVALID;
    audiodec->first_aac_packet = FALSE;

}
