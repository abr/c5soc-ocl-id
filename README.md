# What is this?

This is a method of reading the Unique Chip ID from the Intel Cyclone V SoC
device using the OpenCL for FPGA design flow. The Intel provided IP used
to read the Unique Chip ID is packaged into an OpenCL library
for easy use in other applications.

This is built using version 16.1 of both Quartus Prime Lite and
the Intel FPGA SDK for OpenCL. Hardware was tested on the DE1-SoC board
from Terasic using the BSP for Altera tools v16.0.


## OpenCL Library

The ``device/chip_id_lib`` directory contains the source code used to
create the OpenCL library:

- ``altchip_id.v``: The Chip ID core from Intel with slight modifications
    that allow it to be used as an OpenCL library function.
- ``altchip_id_spec.xml``: The specifications of the verilog core that
  communicate the expected behaviour to the OpenCL compiler.
- ``altchip_id.cl``: A simple behavioural model of the verilog function
  that allows for simulation of the OpenCL code.


To use the OpenCL library in other applications, you will need these 3
source files and the packaged ``aoclib`` file available to be compiled
with your new application. The generated ``aoclib`` file is described in more
detail below in [Building From Source](#building-from-source).


## Demo Implementation on DE1-SoC

The OpenCL library can be easily dropped into an arbitrary OpenCL FPGA design
and this repository includes a minimal example of how to do this.

Inside the ``device`` directory you will find ``id_extractor.cl``. This
is a simple OpenCL kernel that uses the OpenCL library to read the
Unique Chip ID.

In the ``host/src`` directory there is the ``id_extractor_host.cpp``
host code.

Note the ``library`` branch compiles the host code to a shared library for
use with other applications.

### Test it out

In the ``bin`` directory you will find the compiled host code,
``id_extractor_host``, and the compiled hardware design,
``id_extractor.aocx``. Copy both of these files to the DE1-SoC board in the
same directory. Run the application by executing the host code:

    ./id_extractor_host


## Building From Source

The ``Makefile`` has three options which build the demo:

- ``make lib``: This will create the OpenCL library, ``chip_id_lib.aoclib``,
  and place it in the ``device/chip_id_lib`` directory.
- ``make hw``: This will generate the hardware using the demo kernel and
  the created OpenCL library. The generated files are placed in the ``bin``
  directory. *NB - you must run ``make lib`` before generating the hardware*
- ``make host``: This will compile the C++ host code into an executable and
  drop it in the ``bin`` directory.


### Porting to a new device

If you aren't using the DE1-SoC device then you may need to modify the
Makefile and/or the OpenCL library implementation. In any case, this repo
should serve as a good starting point and, without going into too much
detail, note the following:

- In the Makefile:
    - This application uses the ARM cross compiler and the ``--arm`` flag
      for ``aocl compile-config`` and ``aocl link-config``.
    - For both the ``aoc`` commands and the ``aocl`` command, the board is
      specified as ``--board de1soc_sharedonly``.

- For the OpenCL library, you may need to modify the verilog source and
  xml specification files if you use a different chip, especially for
  Stratix.
