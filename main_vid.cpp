#include <cstdint>
#include <cstdio>
#include <cassert>
#include <chrono>
#include <string>
#include <ctime>
#include "ohtj2k_codec.h"
#include "yolo.hpp"
#include "create_filename.hpp"

#include "model_config.hpp"

int main(int argc, char *argv[]) {
  htj2k::Encoder *htenc;
  htenc = new htj2k::OHTJ2KEncoder();

  if (argc != 3 && argc != 5) {
    std::printf("usage: %s class_list modelfile(.onnx) <capture-width capture-height>\n", argv[0]);
    return EXIT_FAILURE;
  }
  const char *fname_class_list = argv[1];
  const char *onnx_file        = argv[2];

  int32_t tmpw = 640, tmph = 480;
  tmpw                     = std::stoi(argv[3]);
  tmph                     = std::stoi(argv[4]);
  const int32_t cap_width  = tmpw;
  const int32_t cap_height = tmph;

  yolo_class yolo(MODEL_WIDTH, MODEL_HEIGHT, SCORE_THRESHOLD, NMS_THRESHOLD, CONFIDENCE_THRESHOLD);
  // Create a YOLO instance
  try {
    yolo.init(fname_class_list, onnx_file);
  } catch (std::exception &exc) {
    return EXIT_FAILURE;
  }

  assert(yolo.is_empty());

  // Load or capture an image
  cv::Mat frame;
  cv::VideoCapture camera(0);
  if (camera.isOpened() == false) {
    std::printf("ERROR: cannot open the camera.\n");
    return EXIT_FAILURE;
  }

  camera.set(cv::CAP_PROP_FRAME_WIDTH, cap_width);
  camera.set(cv::CAP_PROP_FRAME_HEIGHT, cap_height);
  cv::Mat output_image;

  while (true) {
    if (camera.read(frame) == false) {
      printf("ERROR: cannot grab a frame\n");
      break;
    }
    [[maybe_unused]] int tr0 = yolo.get_aftrigger();


    // Process the image
    output_image = yolo.invoke(frame);

    int tr1 = yolo.get_aftrigger();

    // Put efficiency information
    double t          = yolo.get_inference_time();
    std::string label = cv::format("Model: %s , Inference time: %6.2f ms", onnx_file, t);
    cv::putText(output_image, label, cv::Point(20, 40), FONT_FACE, FONT_SCALE, RED, 2);
    imshow("Output", output_image);

    const uint8_t Quality = 95;
    // compress a frame into HTJ2K and save it as a file
    if (tr1) {
      std::string fname = create_filename_based_on_time();
      cv::cvtColor(frame, output_image, cv::COLOR_BGR2RGB);
      auto t_j2k_0 = std::chrono::high_resolution_clock::now();
      htj2k::CodestreamBuffer cb =
          htenc->encodeRGB8(output_image.data, output_image.cols, output_image.rows, Quality);
      auto t_j2k    = std::chrono::high_resolution_clock::now() - t_j2k_0;
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t_j2k).count();
      printf("HT Encoding takes %f [ms], codestream size = %zu bytes\n",
             static_cast<double>(duration) / 1000.0, cb.size);
      FILE *fp = fopen(fname.c_str(), "wb");
      fwrite(cb.codestream, sizeof(uint8_t), cb.size, fp);
      fclose(fp);
    }

    int32_t keycode = cv::waitKey(1);

    if (keycode == 'q') {
      break;
    }
  }

  cv::destroyAllWindows();
  return EXIT_SUCCESS;
}
