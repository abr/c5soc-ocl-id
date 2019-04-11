ulong get_chip_id(char dummy);

__attribute__((num_compute_units(1)))
__attribute__((reqd_work_group_size(1,1,1)))
__attribute__((max_work_group_size(1)))
__kernel void id_extractor(__global ulong *restrict chip_id){

    char dummy = 0;  // OCL lib functions require an input, this does nothing
    *chip_id = get_chip_id(dummy);  // Call lib function
}
