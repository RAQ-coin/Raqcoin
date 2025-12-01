#ifndef RAQCOIN_GPU_MINING_H
#define RAQCOIN_GPU_MINING_H
uint64_t GPUSearchSolution(uint32_t* coefficients, unsigned int number_of_variables,
                           unsigned int number_of_equations);
int  GetDeviceCount();
void SetDevice(int device);
#endif
