
#ifndef WEBP_WEBP_DECODE_H_
#define WEBP_WEBP_DECODE_H_

#include "./types.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define WEBP_DECODER_ABI_VERSION 0x0200    // MAJOR(8b) + MINOR(8b)

WEBP_EXTERN(int) WebPGetDecoderVersion(void);

WEBP_EXTERN(int) WebPGetInfo(const uint8_t* data, size_t data_size,
                             int* width, int* height);

WEBP_EXTERN(uint8_t*) WebPDecodeRGBA(const uint8_t* data, size_t data_size,
                                     int* width, int* height);

WEBP_EXTERN(uint8_t*) WebPDecodeARGB(const uint8_t* data, size_t data_size,
                                     int* width, int* height);

WEBP_EXTERN(uint8_t*) WebPDecodeBGRA(const uint8_t* data, size_t data_size,
                                     int* width, int* height);

WEBP_EXTERN(uint8_t*) WebPDecodeRGB(const uint8_t* data, size_t data_size,
                                    int* width, int* height);

WEBP_EXTERN(uint8_t*) WebPDecodeBGR(const uint8_t* data, size_t data_size,
                                    int* width, int* height);


WEBP_EXTERN(uint8_t*) WebPDecodeYUV(const uint8_t* data, size_t data_size,
                                    int* width, int* height,
                                    uint8_t** u, uint8_t** v,
                                    int* stride, int* uv_stride);

WEBP_EXTERN(uint8_t*) WebPDecodeRGBAInto(
    const uint8_t* data, size_t data_size,
    uint8_t* output_buffer, size_t output_buffer_size, int output_stride);
WEBP_EXTERN(uint8_t*) WebPDecodeARGBInto(
    const uint8_t* data, size_t data_size,
    uint8_t* output_buffer, size_t output_buffer_size, int output_stride);
WEBP_EXTERN(uint8_t*) WebPDecodeBGRAInto(
    const uint8_t* data, size_t data_size,
    uint8_t* output_buffer, size_t output_buffer_size, int output_stride);

WEBP_EXTERN(uint8_t*) WebPDecodeRGBInto(
    const uint8_t* data, size_t data_size,
    uint8_t* output_buffer, size_t output_buffer_size, int output_stride);
WEBP_EXTERN(uint8_t*) WebPDecodeBGRInto(
    const uint8_t* data, size_t data_size,
    uint8_t* output_buffer, size_t output_buffer_size, int output_stride);

WEBP_EXTERN(uint8_t*) WebPDecodeYUVInto(
    const uint8_t* data, size_t data_size,
    uint8_t* luma, size_t luma_size, int luma_stride,
    uint8_t* u, size_t u_size, int u_stride,
    uint8_t* v, size_t v_size, int v_stride);


typedef enum { MODE_RGB = 0, MODE_RGBA = 1,
               MODE_BGR = 2, MODE_BGRA = 3,
               MODE_ARGB = 4, MODE_RGBA_4444 = 5,
               MODE_RGB_565 = 6,
               MODE_rgbA = 7,
               MODE_bgrA = 8,
               MODE_Argb = 9,
               MODE_rgbA_4444 = 10,
               MODE_YUV = 11, MODE_YUVA = 12,  // yuv 4:2:0
               MODE_LAST = 13
             } WEBP_CSP_MODE;

static WEBP_INLINE int WebPIsPremultipliedMode(WEBP_CSP_MODE mode) {
  return (mode == MODE_rgbA || mode == MODE_bgrA || mode == MODE_Argb ||
          mode == MODE_rgbA_4444);
}

static WEBP_INLINE int WebPIsAlphaMode(WEBP_CSP_MODE mode) {
  return (mode == MODE_RGBA || mode == MODE_BGRA || mode == MODE_ARGB ||
          mode == MODE_RGBA_4444 || mode == MODE_YUVA ||
          WebPIsPremultipliedMode(mode));
}

static WEBP_INLINE int WebPIsRGBMode(WEBP_CSP_MODE mode) {
  return (mode < MODE_YUV);
}


typedef struct {    // view as RGBA
  uint8_t* rgba;    // pointer to RGBA samples
  int stride;       // stride in bytes from one scanline to the next.
  size_t size;      // total size of the *rgba buffer.
} WebPRGBABuffer;

typedef struct {              // view as YUVA
  uint8_t* y, *u, *v, *a;     // pointer to luma, chroma U/V, alpha samples
  int y_stride;               // luma stride
  int u_stride, v_stride;     // chroma strides
  int a_stride;               // alpha stride
  size_t y_size;              // luma plane size
  size_t u_size, v_size;      // chroma planes size
  size_t a_size;              // alpha-plane size
} WebPYUVABuffer;

typedef struct {
  WEBP_CSP_MODE colorspace;  // Colorspace.
  int width, height;         // Dimensions.
  int is_external_memory;    // If true, 'internal_memory' pointer is not used.
  union {
    WebPRGBABuffer RGBA;
    WebPYUVABuffer YUVA;
  } u;                       // Nameless union of buffer parameters.
  uint32_t       pad[4];     // padding for later use

  uint8_t* private_memory;   // Internally allocated memory (only when
} WebPDecBuffer;

WEBP_EXTERN(int) WebPInitDecBufferInternal(WebPDecBuffer*, int);

static WEBP_INLINE int WebPInitDecBuffer(WebPDecBuffer* buffer) {
  return WebPInitDecBufferInternal(buffer, WEBP_DECODER_ABI_VERSION);
}

WEBP_EXTERN(void) WebPFreeDecBuffer(WebPDecBuffer* buffer);


typedef enum {
  VP8_STATUS_OK = 0,
  VP8_STATUS_OUT_OF_MEMORY,
  VP8_STATUS_INVALID_PARAM,
  VP8_STATUS_BITSTREAM_ERROR,
  VP8_STATUS_UNSUPPORTED_FEATURE,
  VP8_STATUS_SUSPENDED,
  VP8_STATUS_USER_ABORT,
  VP8_STATUS_NOT_ENOUGH_DATA
} VP8StatusCode;


typedef struct WebPIDecoder WebPIDecoder;

WEBP_EXTERN(WebPIDecoder*) WebPINewDecoder(WebPDecBuffer* output_buffer);

WEBP_EXTERN(WebPIDecoder*) WebPINewRGB(
    WEBP_CSP_MODE csp,
    uint8_t* output_buffer, size_t output_buffer_size, int output_stride);

WEBP_EXTERN(WebPIDecoder*) WebPINewYUVA(
    uint8_t* luma, size_t luma_size, int luma_stride,
    uint8_t* u, size_t u_size, int u_stride,
    uint8_t* v, size_t v_size, int v_stride,
    uint8_t* a, size_t a_size, int a_stride);

WEBP_EXTERN(WebPIDecoder*) WebPINewYUV(
    uint8_t* luma, size_t luma_size, int luma_stride,
    uint8_t* u, size_t u_size, int u_stride,
    uint8_t* v, size_t v_size, int v_stride);

WEBP_EXTERN(void) WebPIDelete(WebPIDecoder* idec);

WEBP_EXTERN(VP8StatusCode) WebPIAppend(
    WebPIDecoder* idec, const uint8_t* data, size_t data_size);

WEBP_EXTERN(VP8StatusCode) WebPIUpdate(
    WebPIDecoder* idec, const uint8_t* data, size_t data_size);

WEBP_EXTERN(uint8_t*) WebPIDecGetRGB(
    const WebPIDecoder* idec, int* last_y,
    int* width, int* height, int* stride);

WEBP_EXTERN(uint8_t*) WebPIDecGetYUVA(
    const WebPIDecoder* idec, int* last_y,
    uint8_t** u, uint8_t** v, uint8_t** a,
    int* width, int* height, int* stride, int* uv_stride, int* a_stride);

static WEBP_INLINE uint8_t* WebPIDecGetYUV(
    const WebPIDecoder* idec, int* last_y, uint8_t** u, uint8_t** v,
    int* width, int* height, int* stride, int* uv_stride) {
  return WebPIDecGetYUVA(idec, last_y, u, v, NULL, width, height,
                         stride, uv_stride, NULL);
}

WEBP_EXTERN(const WebPDecBuffer*) WebPIDecodedArea(
    const WebPIDecoder* idec, int* left, int* top, int* width, int* height);

/*
     WebPDecoderConfig config;
     CHECK(WebPInitDecoderConfig(&config));

     CHECK(WebPGetFeatures(data, data_size, &config.input) == VP8_STATUS_OK);

     config.no_fancy = 1;
     config.output.colorspace = MODE_BGRA;


     CHECK(WebPDecode(data, data_size, &config) == VP8_STATUS_OK);


     WebPFreeDecBuffer(&config.output);
*/

typedef struct {
  int width;        // Width in pixels, as read from the bitstream.
  int height;       // Height in pixels, as read from the bitstream.
  int has_alpha;    // True if the bitstream contains an alpha channel.

  int bitstream_version;        // should be 0 for now. TODO(later)
  int no_incremental_decoding;  // if true, using incremental decoding is not
  int rotate;                   // TODO(later)
  int uv_sampling;              // should be 0 for now. TODO(later)
  uint32_t pad[3];              // padding for later use
} WebPBitstreamFeatures;

WEBP_EXTERN(VP8StatusCode) WebPGetFeaturesInternal(
    const uint8_t*, size_t, WebPBitstreamFeatures*, int);

static WEBP_INLINE VP8StatusCode WebPGetFeatures(
    const uint8_t* data, size_t data_size,
    WebPBitstreamFeatures* features) {
  return WebPGetFeaturesInternal(data, data_size, features,
                                 WEBP_DECODER_ABI_VERSION);
}

typedef struct {
  int bypass_filtering;               // if true, skip the in-loop filtering
  int no_fancy_upsampling;            // if true, use faster pointwise upsampler
  int use_cropping;                   // if true, cropping is applied _first_
  int crop_left, crop_top;            // top-left position for cropping.
  int crop_width, crop_height;        // dimension of the cropping area
  int use_scaling;                    // if true, scaling is applied _afterward_
  int scaled_width, scaled_height;    // final resolution
  int use_threads;                    // if true, use multi-threaded decoding

  int force_rotation;                 // forced rotation (to be applied _last_)
  int no_enhancement;                 // if true, discard enhancement layer
  uint32_t pad[6];                    // padding for later use
} WebPDecoderOptions;

typedef struct {
  WebPBitstreamFeatures input;  // Immutable bitstream features (optional)
  WebPDecBuffer output;         // Output buffer (can point to external mem)
  WebPDecoderOptions options;   // Decoding options
} WebPDecoderConfig;

WEBP_EXTERN(int) WebPInitDecoderConfigInternal(WebPDecoderConfig*, int);

static WEBP_INLINE int WebPInitDecoderConfig(WebPDecoderConfig* config) {
  return WebPInitDecoderConfigInternal(config, WEBP_DECODER_ABI_VERSION);
}

WEBP_EXTERN(WebPIDecoder*) WebPIDecode(const uint8_t* data, size_t data_size,
                                       WebPDecoderConfig* config);

WEBP_EXTERN(VP8StatusCode) WebPDecode(const uint8_t* data, size_t data_size,
                                      WebPDecoderConfig* config);

#if defined(__cplusplus) || defined(c_plusplus)
}    // extern "C"
#endif

#endif  /* WEBP_WEBP_DECODE_H_ */
