# StormByte
![Linux](https://img.shields.io/badge/Linux-Supported-1793D1?logo=linux&logoColor=white)
![Windows](https://img.shields.io/badge/Windows-Supported-0078D6?logo=windows&logoColor=white)
![C++23](https://img.shields.io/badge/C%2B%2B-23-00599C?logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.12+-064F8C?logo=cmake&logoColor=white)
![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)
![Status](https://img.shields.io/badge/Status-Active-success)
[![Compile & Test](https://github.com/StormBytePP/StormByte-Multimedia/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/StormBytePP/StormByte-Multimedia/actions/workflows/build.yml)

StormByte is a comprehensive, cross-platform C++ library aimed at easing system programming, configuration management, logging, and database handling tasks. This library provides a unified API that abstracts away the complexities and inconsistencies of different platforms (Windows, Linux).

## Features

- **Multimedia**:
  - Unified multimedia backend with modular codec and container support.
  - FFmpeg‑powered plugin system that enables only the codecs actually available.
  - Automatic codec registration and propagation of build options.
  - Metadata and format introspection (resolución, duración, framerate, streams…).
  - Video decoding into safe, typed image buffers.
  - Video encoding using supported codecs (H.264, H.265, VP8/VP9, AAC, Opus, MP3, Vorbis…).
  - Pixel format conversion (RGB, YUV, planar/packed).
  - Colorspace handling (BT.601, BT.709, BT.2020).
  - High‑quality image scaling via libswscale (upscale/downscale).
  - Audio decoding and encoding for common formats and codecs.
  - Container support for MP4, MKV, OGG, WebM, WAV and others.
  - RAII wrappers for all FFmpeg resources (AVFrame, AVPacket, AVCodecContext…).
  - Error‑safe API with typed results and no raw FFmpeg exposure.
  - Plugin‑driven build system: each codec/feature compiles only if available.

## Table of Contents

- [Repository](#Repository)
- [Installation](#Installation)
- [Modules](#Modules)
	- [Base](https://dev.stormbyte.org/StormByte)
	- [Buffer](https://dev.stormbyte.org/StormByte-Buffer)
	- [Config](https://dev.stormbyte.org/StormByte-Config)
	- [Crypto](https://dev.stormbyte.org/StormByte-Crypto)
	- [Database](https://dev.stormbyte.org/StormByte-Database)
	- [Logger](https://github.com/StormBytePP/StormByte-Logger.git)
	- **Multimedia**
	- [Network](https://dev.stormbyte.org/StormByte-Network)
	- [System](https://dev.stormbyte.org/StormByte-System)
- [Contributing](#Contributing)
- [License](#License)

## Repository

You can visit the code repository at [GitHub](https://github.com/StormBytePP/StormByte-Config)

## Installation

### Prerequisites

Ensure you have the following installed:

- C++23 compatible compiler
- CMake 3.12 or higher

### Building

To build the library, follow these steps:

```sh
git clone https://github.com/StormBytePP/StormByte-Multimedia.git
cd StormByte-Multimedia
mkdir build
cd build
cmake ..
make
```

## Modules

StormByte Library is composed by several modules:

### Multimedia

#### Overview

StormByte‑Multimedia provides a clean, modular, and cross‑platform backend for working with audio and video in C++.  
Instead of exposing the complexity of FFmpeg, the module offers a high‑level, type‑safe API built around small, focused components that handle decoding, encoding, format introspection, frame transformation, and container management.

The design follows StormByte’s core philosophy: each operation is isolated in its own module, every resource is managed through RAII, and all low‑level details remain hidden behind stable, expressive abstractions.  
This allows developers to work with multimedia pipelines—transcoding, analysis, normalization, or format conversion—without dealing with raw pointers, global state, or FFmpeg’s internal structures.

The module automatically adapts to the capabilities available at build time through a plugin‑driven system, enabling only the codecs and features that are actually present if bundled ffmpeg backend is chosen.  
Combined with components such as the VideoScaler, AudioResampler, and FrameNormalizer, StormByte‑Multimedia ensures that input and output formats are always reconciled safely and predictably, making multimedia processing both powerful and approachable.

## Contributing

Contributions are welcome! Please fork the repository and submit pull requests for any enhancements or bug fixes.

## License

This project is licensed under LGPL v3 License - see the [LICENSE](LICENSE) file for details.
