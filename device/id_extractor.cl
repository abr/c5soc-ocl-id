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

// library function prototype. No header file for a single prototype...
ulong get_chip_id(char dummy);

// NB: OpenCL ulong is 64b, but C++ ulong is 32b only!
__kernel void id_extractor(__global ulong *restrict chip_id){

    char dummy = 0;  // OCL lib functions require an input, this does nothing
    *chip_id = get_chip_id(dummy);  // Call lib function to read ID
}
