/*
Copyright (c) 2013-2019 Applied Brain Research Inc.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Ben Morcos
2019-04-12
*/
#include <stdio.h>
#include "CL/opencl.h"
#include "AOCLUtils/aocl_utils.h"

namespace aocl_utils {

  // OpenCL Setup
  cl_platform_id platform = NULL;
  scoped_array<cl_device_id> device;
  unsigned n_devices = 0;
  cl_context context;
  cl_command_queue queue;
  cl_program program = NULL;
  cl_kernel kernel;
  cl_mem chip_id_buf;

  // Data pointer for passing data between host/device
  typedef unsigned long long uint64;
  uint64 chip_id_host;  // 64b

  extern "C" uint64 get_id() {
    cl_int status;
    cl_event kernel_event;

    // Set kernel arg
    unsigned argi = 0;
    status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &chip_id_buf);
    checkError(status, "Failed to set argument %d", argi - 1);

    // Run
    printf("Launching kernel... \n\r");
    status = clEnqueueTask(queue, kernel, 0, NULL, &kernel_event);
    checkError(status, "Failed to launch kernel");

    clWaitForEvents(cl_uint(1), &kernel_event);  // wait for kernels to end
    clReleaseEvent(kernel_event);

    // Read output
    printf("Reading ID from buffer...\n");
    status = clEnqueueReadBuffer(
      queue, chip_id_buf, CL_TRUE, 0, 8, &chip_id_host, 0, 0, 0);
    checkError(status, "Failed to chip ID from buffer");

    return chip_id_host;
  }

  extern "C" bool setup(char* aocx_file) {
    /*

    `aocx_file` - full path to *.aocx omitting the extension
        eg. "/home/root/id_extractor"
    */
    cl_int status;

    printf("AOCX FILE: %s\n", aocx_file);

    /*
     * Setup all the OpenCL objects needed
     */

    // Get the OpenCL platform.
    platform = findPlatform("Altera");
    if (platform == NULL) {
        printf("ERROR: Unable to find Altera OpenCL platform.\n");
        return false;
    }

    // Get device
    device.reset(getDevices(platform, CL_DEVICE_TYPE_ALL, &n_devices));

    // Get context
    context = clCreateContext(
        NULL, 1, device, &oclContextCallback, NULL, &status);
    checkError(status, "Failed to create context");

    // Create program
    std::string binary_file = getBoardBinaryFile(aocx_file, device[0]);
    program = createProgramFromBinary(context, binary_file.c_str(), device, 1);

    // Build the program
    status = clBuildProgram(
      program, 0, NULL, NULL, NULL, NULL);
    checkError(status, "Failed to build program");

    // Create OpenCL objects
    queue = clCreateCommandQueue(
      context, device[0], CL_QUEUE_PROFILING_ENABLE, &status);
    checkError(status, "Failed to create command queue");

    kernel = clCreateKernel(program, "id_extractor", &status);
    checkError(status, "Failed to create kernel");

    // Create buffer for reading
    chip_id_buf = clCreateBuffer(
        context, CL_MEM_WRITE_ONLY, 8, NULL, &status);
    checkError(status, "Failed to create buffer for chip_id");

    return true;
  }

  extern "C" void cleanup() {
    /*
    Release all memory objects
    */
    if (kernel) {
        clReleaseKernel(kernel);
    }
    if (queue) {
        clReleaseCommandQueue(queue);
    }
    if (chip_id_buf) {
        clReleaseMemObject(chip_id_buf);
    }
    if (program) {
        clReleaseProgram(program);
    }
    if (context) {
        clReleaseContext(context);
    }
  }
}  // namespace aocl_utils
