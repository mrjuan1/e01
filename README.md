# E01

A small WIP game engine for personal projects.

## Contents

- [Requirements](#requirements)
- [Configuration](#configuration)
- [Building](#building)
- [Usage](#usage)
- [Development](#development)

[Back to top](#e01)

## Requirements

For building, a C compiler and compatible C library of your choice can be used. This project was developed using GCC and GLIBC.

Additional libraries required are SDL2, OpenGL ES 3.1 and cglm. Tools required to prepare assets are the model and texture tool, which are provided as submodules and will be built during the [configuration](#configuration) process. More dependencies will be added later.

To run or debug this engine, a valid Blender model (see the model tool's README.md) and a valid PNG, JPG or BMP texture (see the texture tool's README.md) will be required in the project's parent directory. Once placed there, see each tool's README.md on how to prepare these assets.

[Back to top](#e01)

## Configuration

This project can be configured for building using the `./configure` script. This script allows you to specify which platform you'd like to build for.

Currently, only `linux` is supported with plans to add support for Windows, Android and Switch (homebrew) later.

[Back to top](#e01)

## Building

After [configuring](#configuration), the project can be built using the `make` command. This will build the executable in debug mode and prepare all tools and assets required for the engine to run.

For more `make` options, run `make help`.

[Back to top](#e01)

## Usage

The engine can be run with a simple `./e01`, assuming all assets are prepared and in-place.

[Back to top](#e01)

## Development

To debug this project, GDB will be required. It can be launched using `make debug`. Support for debugging via VS Code's C/C++ extension is also available.

[Back to top](#e01)
