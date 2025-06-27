# 🧠 EdgeVision - YOLOv5 Object Detection on Raspberry Pi

EdgeVision is a lightweight object detection system built for edge devices like the Raspberry Pi. It leverages **YOLOv5** models exported to **ONNX**, combined with **OpenCV** and **C++**, to perform real-time object detection on video or still images.

---

## 🚀 Features

- 🖼️ Still and live video detection (`yolo_still`, `yolo_vid`)
- 🎥 Real-time camera input support
- 🧠 YOLOv5 ONNX model inference
- 🧪 Runs efficiently on Raspberry Pi
- 📦 Minimal dependencies, optimized build with CMake

---

## 🛠️ Technologies

- C++17
- OpenCV
- ONNX Runtime
- CMake
- Raspberry Pi OS (64-bit)

---

## 📁 Project Structure

EdgeVision/
├── bin/ # Output binaries
├── build/ # CMake build files
├── models/ # YOLOv5 ONNX models
├── dataset/ # (Optional) For training custom data
├── ext/ # OpenHTJ2K compression lib
├── main_still.cpp # Still image inference
├── main_vid.cpp # Video inference
├── CMakeLists.txt # Build config
└── README.md # You are here


---

## 🔧 Installation & Build

### 1. Clone the repo

```bash
git clone https://github.com/Bhavanapoli08/EdgeVision.git
cd EdgeVision

2. Install dependencies
Make sure OpenCV and ONNX Runtime are installed. On Raspberry Pi:
sudo apt update
sudo apt install libopencv-dev cmake build-essential

3. Build the project
mkdir build && cd build
cmake ..
cmake --build .

🧪 Run Detection
Still Image
 ./bin/yolo_still image.jpg

Live Video (e.g., from USB camera)
./bin/yolo_vid

🖍️ Custom Model
Want to detect pens, tools, or custom objects?

Train a YOLOv5 model on your dataset

Export to ONNX:
python export.py --weights runs/train/exp/weights/best.pt --include onnx
Replace the .onnx model in models/ and update labels.txt.

