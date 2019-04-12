#include <stdio.h>
#include <math.h>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "CL/opencl.h"
#include "AOCLUtils/aocl_utils.h"

using namespace aocl_utils;

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
ulong chip_id_host;

// Function prototypes
bool setup(char* aocx_file);
void cleanup();

int main() {
  cl_int status;
  cl_event kernel_event;
  char *bitstream = "id_extractor";

  if (!setup(bitstream)) {
    return -1;
  }

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
    queue, chip_id_buf, 1, 0, 8, &chip_id_host, 0, 0, 0);
  checkError(status, "Failed to chip ID from buffer");

  // printf("Found unique chip ID: 0x%016X", chip_id_host);
  cleanup();
  return 0;
}

bool setup(char* aocx_file) {
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

  // get device
  device.reset(getDevices(platform, CL_DEVICE_TYPE_ALL, &n_devices));

  // get context
  context = clCreateContext(
      NULL, 1, device, &oclContextCallback, NULL, &status);
  checkError(status, "Failed to create context");

  // create program
  std::string binary_file = getBoardBinaryFile(aocx_file, device[0]);
  program = createProgramFromBinary(context, binary_file.c_str(), device, 1);

  // build the program
  status = clBuildProgram(
    program, 0, NULL, "-cl-fast-relaxed-math", NULL, NULL);
  checkError(status, "Failed to build program");

  // Create OpenCL objects
  // Out of Order queues not supported, so need two command queues to run
  // two kernels concurrently
  queue = clCreateCommandQueue(
    context, device[0], CL_QUEUE_PROFILING_ENABLE, &status);
  checkError(status, "Failed to create command queue");

  kernel = clCreateKernel(program, "id_extractor", &status);
  checkError(status, "Failed to create kernel");

  // Create buffer for reading
  // CL_MEM_ALLOC_HOST_PTR flag allocates shared memory
  chip_id_buf = clCreateBuffer(
      context, CL_MEM_WRITE_ONLY, 8, NULL, &status);
  checkError(status, "Failed to create buffer for chip_id");

  return true;
}

void cleanup() {
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
