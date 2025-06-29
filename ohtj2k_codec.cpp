#include "ohtj2k_codec.h"

#include <memory>
#include <stdexcept>

constexpr uint32_t NUMTHREADS = 1;

htj2k::OHTJ2KEncoder::OHTJ2KEncoder() : num_threads(NUMTHREADS){};

htj2k::CodestreamBuffer htj2k::OHTJ2KEncoder::encodeRGB8(const uint8_t* pixels, uint32_t width,
                                                         uint32_t height, uint8_t QF = 90) {
  return this->encode8(pixels, width, height, 3, QF);
}

htj2k::CodestreamBuffer htj2k::OHTJ2KEncoder::encodeRGBA8(const uint8_t* pixels, uint32_t width,
                                                          uint32_t height, uint8_t QF = 90) {
  return this->encode8(pixels, width, height, 4, QF);
}

htj2k::CodestreamBuffer htj2k::OHTJ2KEncoder::encode8(const uint8_t* pixels, uint32_t width,
                                                      uint32_t height, uint8_t num_comps, uint8_t QF = 90) {
  open_htj2k::siz_params siz;
  siz.Rsiz   = 0;
  siz.Xsiz   = width;
  siz.Ysiz   = height;
  siz.XOsiz  = 0;
  siz.YOsiz  = 0;
  siz.XTsiz  = 0;
  siz.YTsiz  = 0;
  siz.XTOsiz = 0;
  siz.YTOsiz = 0;
  siz.Csiz   = num_comps;
  for (auto c = 0; c < siz.Csiz; ++c) {
    siz.Ssiz.push_back(7);
    auto compw = width;
    auto comph = height;
    siz.XRsiz.push_back(((siz.Xsiz - siz.XOsiz) + compw - 1) / compw);
    siz.YRsiz.push_back(((siz.Ysiz - siz.YOsiz) + comph - 1) / comph);
  }

  open_htj2k::cod_params cod;
  cod.blkwidth          = 4;
  cod.blkheight         = 4;
  cod.is_max_precincts  = false;
  cod.use_SOP           = false;
  cod.use_EPH           = false;
  cod.progression_order = 2;
  cod.number_of_layers  = 1;
  cod.use_color_trafo   = 1;
  cod.dwt_levels        = 5;
  cod.codeblock_style   = 0x040;
  cod.transformation    = 0;  // lossy = 0, lossles = 1

  for (size_t i = 0; i < cod.dwt_levels; ++i) {
    cod.PPx.push_back(8);
    cod.PPy.push_back(8);
  }
  cod.PPx.push_back(7);
  cod.PPy.push_back(7);

  open_htj2k::qcd_params qcd;
  qcd.is_derived          = false;
  qcd.number_of_guardbits = 1;
  qcd.base_step           = 1.0 / 256;
  if (qcd.base_step == 0.0) {
    qcd.base_step = 1.0f / static_cast<float>(1 << 8);
  }

  bool isJPH          = false;
  uint8_t Qfactor     = QF;  // 0xff = no Qfacor;
  uint8_t color_space = 0;

  auto buf = std::make_unique<std::unique_ptr<int32_t[]>[]>(num_comps);
  for (size_t i = 0; i < num_comps; ++i) {
    buf[i] = std::make_unique<int32_t[]>(width * height);
  }

  std::vector<int32_t*> input_buf;
  for (auto c = 0; c < num_comps; ++c) {
    input_buf.push_back(buf[c].get());
  }
  const uint8_t* line = pixels;
  for (uint32_t i = 0; i < height; ++i) {
    for (uint32_t c = 0; c < num_comps; ++c) {
      const uint8_t* in = line + c;
      int32_t* out      = buf[c].get() + i * width;
      for (uint32_t p = 0; p < width; ++p) {
        *out = *in;
        out += 1;
        in += num_comps;
      }
    }
    line += num_comps * width;
  }
  open_htj2k::openhtj2k_encoder encoder("", input_buf, siz, cod, qcd, Qfactor, isJPH, color_space,
                                        num_threads);
  encoder.set_output_buffer(this->outbuf_);
  size_t total_size = encoder.invoke();

  htj2k::CodestreamBuffer cb;

  cb.codestream = outbuf_.data();
  cb.size       = total_size;

  return cb;
}

htj2k::OHTJ2KDecoder::OHTJ2KDecoder() : num_threads(NUMTHREADS){};

htj2k::PixelBuffer htj2k::OHTJ2KDecoder::decodeRGB8(const uint8_t* codestream, size_t size, uint32_t width,
                                                    uint32_t height, const uint8_t* init_data,
                                                    size_t init_data_size) {
  return this->decode8(codestream, size, 3);
}

htj2k::PixelBuffer htj2k::OHTJ2KDecoder::decodeRGBA8(const uint8_t* codestream, size_t size, uint32_t width,
                                                     uint32_t height, const uint8_t* init_data,
                                                     size_t init_data_size) {
  return this->decode8(codestream, size, 4);
}

htj2k::PixelBuffer htj2k::OHTJ2KDecoder::decode8(const uint8_t* codestream, size_t size,
                                                 uint8_t num_comps) {
  open_htj2k::openhtj2k_decoder decoder(codestream, size, 0, num_threads);
  std::vector<int32_t*> buf;
  std::vector<uint32_t> img_width;
  std::vector<uint32_t> img_height;
  std::vector<uint8_t> img_depth;
  std::vector<bool> img_signed;
  decoder.invoke(buf, img_width, img_height, img_depth, img_signed);
  uint32_t width  = img_width[0];
  uint32_t height = img_height[0];
  this->pixels_.resize(width * height * num_comps);
  for (uint32_t i = 0; i < height; ++i) {
    uint8_t* line = &this->pixels_.data()[width * i * num_comps];
    for (uint32_t c = 0; c < num_comps; ++c) {
      uint8_t* out = line + c;
      int32_t* in  = buf[c] + width * i;
      for (uint32_t p = 0; p < width; ++p) {
        *out = *in;
        out += num_comps;
        in += 1;
      }
    }
  }
  htj2k::PixelBuffer pb = {
      .height = height, .width = width, .num_comps = num_comps, .pixels = &this->pixels_.data()[0]};
  return pb;
}
