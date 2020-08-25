# EDAflow_lc3-OpenRoadFlow
You can get more information by looking at [OpenROAD-flow-public README](https://github.com/The-OpenROAD-Project/OpenROAD-flow-public/blob/master/README.md).

## Code Organization
This repository serves as an example RTL-to-GDS flow using the OpenROAD tools.
The two main components are:
1. **tools**: This directory contains the source code for the entire `openroad`
   app (via submodules) as well as other tools required for the flow. The script
   `build_openroad.sh` in this repository will automatically build the OpenROAD
   toolchain.

2. **flow**: This directory contains reference recipes and scripts to run      |
   designs through the flow. It also contains platforms and test designs.

## Setup
The flow has the following dependencies:
* OpenROAD
* KLayout
* TritonRoute
* Yosys

The dependencies can either be obtained from a pre-compiled build export or
built manually. See the [KLayout website](https://www.klayout.de/) for
installation instructions.

### Option 1: Building the tools using docker
This build option leverages a multi-step docker flow to install the tools and
dependencies to a runner image. To follow these instructions, you must have
docker installed, permissions to run docker, and docker container network access
enabled. This step will create a runner image tagged as `openroad/flow`.
1.  Clone the OpenROAD-flow repository
```
git clone --recursive https://github.com/Hairline-Keeper/EDAflow_lc3-OpenRoadFlow.git
```
2. Ensure your docker daemon is running and `docker` is in your PATH, then run
the docker build.
```
./build_openroad.sh
```
3. Start an interactive shell in a docker container using your user credentials
```
docker run -it -u $(id -u ${USER}):$(id -g ${USER}) openroad/flow bash
```

### Option 2: Building the tools locally
1. 
  a. CentOS7 : Reference the Dockerfiles and READMEs for the separate tools on the build steps and dependencies.
  ```
  OpenROAD-flow/tools/OpenROAD/Dockerfile
  OpenROAD-flow/tools/yosys/Dockerfile
  OpenROAD-flow/tools/TritonRoute/Dockerfile
  ```
  b. Ubuntu20 : Reference the following command
  ```
  apt install build-essential
  apt install tcl-dev
  apt install libreadline6-dev
  apt install bison
  apt install flex
  apt install libffi-dev
  apt install cmake
  apt install libboost-all-dev
  apt install swig
  apt install klayout
  ```
  See the [KLayout](https://www.klayout.de) instructions for installing KLayout from source.

2. Run the build script
```
./build_openroad.sh
```
3. Update your shell environment
```
source setup_env.sh
```
`klayout` must be added to the path manually.

## Using the flow
See the flow [README](flow) for details about the flow and how
to run designs through the flow

## License
The OpenROAD-flow repository (build and run scripts) has a BSD 3-Clause License.
The flow relies on several tools, platforms and designs that each have their own
licenses:
- Find the tool license at: `OpenROAD-flow/tools/{tool}/` or
`OpenROAD-flow/tools/OpenROAD/src/{tool}/`
- Find the platform license at: `OpenROAD-flow/flow/platforms/{platform}/`
- Find the design license at: `OpenROAD-flow/flow/designs/src/{design}/`
