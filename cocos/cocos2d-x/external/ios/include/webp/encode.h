
#ifndef WEBP_WEBP_ENCODE_H_
#define WEBP_WEBP_ENCODE_H_

#include "./types.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define WEBP_ENCODER_ABI_VERSION 0x0200    // MAJOR(8b) + MINOR(8b)

WEBP_EXTERN(int) WebPGetEncoderVersion(void);


WEBP_EXTERN(size_t) WebPEncodeRGB(const uint8_t* rgb,
                                  int width, int height, int stride,
                                  float quality_factor, uint8_t** output);
WEBP_EXTERN(size_t) WebPEncodeBGR(const uint8_t* bgr,
                                  int width, int height, int stride,
                                  float quality_factor, uint8_t** output);
WEBP_EXTERN(size_t) WebPEncodeRGBA(const uint8_t* rgba,
                                   int width, int height, int stride,
                                   float quality_factor, uint8_t** output);
WEBP_EXTERN(size_t) WebPEncodeBGRA(const uint8_t* bgra,
                                   int width, int height, int stride,
                                   float quality_factor, uint8_t** output);

WEBP_EXTERN(size_t) WebPEncodeLosslessRGB(const uint8_t* rgb,
                                          int width, int height, int stride,
                                          uint8_t** output);
WEBP_EXTERN(size_t) WebPEncodeLosslessBGR(const uint8_t* bgr,
                                          int width, int height, int stride,
                                          uint8_t** output);
WEBP_EXTERN(size_t) WebPEncodeLosslessRGBA(const uint8_t* rgba,
                                           int width, int height, int stride,
                                           uint8_t** output);
WEBP_EXTERN(size_t) WebPEncodeLosslessBGRA(const uint8_t* bgra,
                                           int width, int height, int stride,
                                           uint8_t** output);


typedef enum {
  WEBP_HINT_DEFAULT = 0,  // default preset.
  WEBP_HINT_PICTURE,      // digital picture, like portrait, inner shot
  WEBP_HINT_PHOTO,        // outdoor photograph, with natural lighting
  WEBP_HINT_GRAPH,        // Discrete tone image (graph, map-tile etc).
  WEBP_HINT_LAST
} WebPImageHint;

typedef struct {
  int lossless;           // Lossless encoding (0=lossy(default), 1=lossless).
  float quality;          // between 0 (smallest file) and 100 (biggest)
  int method;             // quality/speed trade-off (0=fast, 6=slower-better)

  WebPImageHint image_hint;  // Hint for image type (lossless only for now).

  int target_size;        // if non-zero, set the desired target size in bytes.
  float target_PSNR;      // if non-zero, specifies the minimal distortion to
  int segments;           // maximum number of segments to use, in [1..4]
  int sns_strength;       // Spatial Noise Shaping. 0=off, 100=maximum.
  int filter_strength;    // range: [0 = off .. 100 = strongest]
  int filter_sharpness;   // range: [0 = off .. 7 = least sharp]
  int filter_type;        // filtering type: 0 = simple, 1 = strong (only used
  int autofilter;         // Auto adjust filter's strength [0 = off, 1 = on]
  int alpha_compression;  // Algorithm for encoding the alpha plane (0 = none,
  int alpha_filtering;    // Predictive filtering method for alpha plane.
  int alpha_quality;      // Between 0 (smallest size) and 100 (lossless).
  int pass;               // number of entropy-analysis passes (in [1..10]).

  int show_compressed;    // if true, export the compressed picture back.
  int preprocessing;      // preprocessing filter (0=none, 1=segment-smooth)
  int partitions;         // log2(number of token partitions) in [0..3]. Default
  int partition_limit;    // quality degradation allowed to fit the 512k limit

  uint32_t pad[8];        // padding for later use
} WebPConfig;

typedef enum {
  WEBP_PRESET_DEFAULT = 0,  // default preset.
  WEBP_PRESET_PICTURE,      // digital picture, like portrait, inner shot
  WEBP_PRESET_PHOTO,        // outdoor photograph, with natural lighting
  WEBP_PRESET_DRAWING,      // hand or line drawing, with high-contrast details
  WEBP_PRESET_ICON,         // small-sized colorful images
  WEBP_PRESET_TEXT          // text-like
} WebPPreset;

WEBP_EXTERN(int) WebPConfigInitInternal(WebPConfig*, WebPPreset, float, int);

static WEBP_INLINE int WebPConfigInit(WebPConfig* config) {
  return WebPConfigInitInternal(config, WEBP_PRESET_DEFAULT, 75.f,
                                WEBP_ENCODER_ABI_VERSION);
}

static WEBP_INLINE int WebPConfigPreset(WebPConfig* config,
                                        WebPPreset preset, float quality) {
  return WebPConfigInitInternal(config, preset, quality,
                                WEBP_ENCODER_ABI_VERSION);
}

WEBP_EXTERN(int) WebPValidateConfig(const WebPConfig* config);


typedef struct WebPPicture WebPPicture;   // main structure for I/O

typedef struct {
  int coded_size;         // final size

  float PSNR[5];          // peak-signal-to-noise ratio for Y/U/V/All/Alpha
  int block_count[3];     // number of intra4/intra16/skipped macroblocks
  int header_bytes[2];    // approximate number of bytes spent for header
  int residual_bytes[3][4];  // approximate number of bytes spent for
  int segment_size[4];    // number of macroblocks in each segments
  int segment_quant[4];   // quantizer values for each segments
  int segment_level[4];   // filtering strength for each segments [0..63]

  int alpha_data_size;    // size of the transparency data
  int layer_data_size;    // size of the enhancement layer data

  uint32_t lossless_features;  // bit0:predictor bit1:cross-color transform
  int histogram_bits;          // number of precision bits of histogram
  int transform_bits;          // precision bits for transform
  int cache_bits;              // number of bits for color cache lookup
  int palette_size;            // number of color in palette, if used
  int lossless_size;           // final lossless size

  uint32_t pad[4];        // padding for later use
} WebPAuxStats;

typedef int (*WebPWriterFunction)(const uint8_t* data, size_t data_size,
                                  const WebPPicture* picture);

typedef struct {
  uint8_t* mem;       // final buffer (of size 'max_size', larger than 'size').
  size_t   size;      // final size
  size_t   max_size;  // total capacity
  uint32_t pad[1];    // padding for later use
} WebPMemoryWriter;

WEBP_EXTERN(void) WebPMemoryWriterInit(WebPMemoryWriter* writer);

WEBP_EXTERN(int) WebPMemoryWrite(const uint8_t* data, size_t data_size,
                                 const WebPPicture* picture);

typedef int (*WebPProgressHook)(int percent, const WebPPicture* picture);

typedef enum {
  WEBP_YUV420 = 0,   // 4:2:0
  WEBP_YUV422 = 1,   // 4:2:2
  WEBP_YUV444 = 2,   // 4:4:4
  WEBP_YUV400 = 3,   // grayscale
  WEBP_CSP_UV_MASK = 3,   // bit-mask to get the UV sampling factors
  WEBP_YUV420A = 4,
  WEBP_YUV422A = 5,
  WEBP_YUV444A = 6,
  WEBP_YUV400A = 7,   // grayscale + alpha
  WEBP_CSP_ALPHA_BIT = 4   // bit that is set if alpha is present
} WebPEncCSP;

typedef enum {
  VP8_ENC_OK = 0,
  VP8_ENC_ERROR_OUT_OF_MEMORY,            // memory error allocating objects
  VP8_ENC_ERROR_BITSTREAM_OUT_OF_MEMORY,  // memory error while flushing bits
  VP8_ENC_ERROR_NULL_PARAMETER,           // a pointer parameter is NULL
  VP8_ENC_ERROR_INVALID_CONFIGURATION,    // configuration is invalid
  VP8_ENC_ERROR_BAD_DIMENSION,            // picture has invalid width/height
  VP8_ENC_ERROR_PARTITION0_OVERFLOW,      // partition is bigger than 512k
  VP8_ENC_ERROR_PARTITION_OVERFLOW,       // partition is bigger than 16M
  VP8_ENC_ERROR_BAD_WRITE,                // error while flushing bytes
  VP8_ENC_ERROR_FILE_TOO_BIG,             // file is bigger than 4G
  VP8_ENC_ERROR_USER_ABORT,               // abort request by user
  VP8_ENC_ERROR_LAST                      // list terminator. always last.
} WebPEncodingError;

#define WEBP_MAX_DIMENSION 16383

struct WebPPicture {

  int use_argb;

  WebPEncCSP colorspace;     // colorspace: should be YUV420 for now (=Y'CbCr).
  int width, height;         // dimensions (less or equal to WEBP_MAX_DIMENSION)
  uint8_t *y, *u, *v;        // pointers to luma/chroma planes.
  int y_stride, uv_stride;   // luma/chroma strides.
  uint8_t* a;                // pointer to the alpha plane
  int a_stride;              // stride of the alpha plane
  uint32_t pad1[2];          // padding for later use

  uint32_t* argb;            // Pointer to argb (32 bit) plane.
  int argb_stride;           // This is stride in pixels units, not bytes.
  uint32_t pad2[3];          // padding for later use

  WebPWriterFunction writer;  // can be NULL
  void* custom_ptr;           // can be used by the writer.

  int extra_info_type;    // 1: intra type, 2: segment, 3: quant
  uint8_t* extra_info;    // if not NULL, points to an array of size

  WebPAuxStats* stats;

  WebPEncodingError error_code;

  WebPProgressHook progress_hook;

  void* user_data;        // this field is free to be set to any value and

  uint32_t pad3[3];       // padding for later use

  uint8_t *u0, *v0;
  int uv0_stride;

  uint32_t pad4[7];       // padding for later use

  void* memory_;          // row chunk of memory for yuva planes
  void* memory_argb_;     // and for argb too.
  void* pad5[2];          // padding for later use
};

WEBP_EXTERN(int) WebPPictureInitInternal(WebPPicture*, int);

static WEBP_INLINE int WebPPictureInit(WebPPicture* picture) {
  return WebPPictureInitInternal(picture, WEBP_ENCODER_ABI_VERSION);
}


WEBP_EXTERN(int) WebPPictureAlloc(WebPPicture* picture);

WEBP_EXTERN(void) WebPPictureFree(WebPPicture* picture);

WEBP_EXTERN(int) WebPPictureCopy(const WebPPicture* src, WebPPicture* dst);

WEBP_EXTERN(int) WebPPictureDistortion(
    const WebPPicture* pic1, const WebPPicture* pic2,
    int metric_type,           // 0 = PSNR, 1 = SSIM
    float result[5]);

WEBP_EXTERN(int) WebPPictureCrop(WebPPicture* picture,
                                 int left, int top, int width, int height);

WEBP_EXTERN(int) WebPPictureView(const WebPPicture* src,
                                 int left, int top, int width, int height,
                                 WebPPicture* dst);

WEBP_EXTERN(int) WebPPictureIsView(const WebPPicture* picture);

WEBP_EXTERN(int) WebPPictureRescale(WebPPicture* pic, int width, int height);

WEBP_EXTERN(int) WebPPictureImportRGB(
    WebPPicture* picture, const uint8_t* rgb, int rgb_stride);
WEBP_EXTERN(int) WebPPictureImportRGBA(
    WebPPicture* picture, const uint8_t* rgba, int rgba_stride);
WEBP_EXTERN(int) WebPPictureImportRGBX(
    WebPPicture* picture, const uint8_t* rgbx, int rgbx_stride);

WEBP_EXTERN(int) WebPPictureImportBGR(
    WebPPicture* picture, const uint8_t* bgr, int bgr_stride);
WEBP_EXTERN(int) WebPPictureImportBGRA(
    WebPPicture* picture, const uint8_t* bgra, int bgra_stride);
WEBP_EXTERN(int) WebPPictureImportBGRX(
    WebPPicture* picture, const uint8_t* bgrx, int bgrx_stride);

WEBP_EXTERN(int) WebPPictureARGBToYUVA(WebPPicture* picture,
                                       WebPEncCSP colorspace);

WEBP_EXTERN(int) WebPPictureYUVAToARGB(WebPPicture* picture);

WEBP_EXTERN(void) WebPCleanupTransparentArea(WebPPicture* picture);

WEBP_EXTERN(int) WebPPictureHasTransparency(const WebPPicture* picture);


WEBP_EXTERN(int) WebPEncode(const WebPConfig* config, WebPPicture* picture);


#if defined(__cplusplus) || defined(c_plusplus)
}    // extern "C"
#endif

#endif  /* WEBP_WEBP_ENCODE_H_ */
