#define TYPE       float 

__kernel void test(const int a, const int n, __global const TYPE* input, __global TYPE* output)
{
    const int tid = get_global_id(0);

    if(tid<n){
        output[tid]=a*input[tid];
    }
    return;
}


