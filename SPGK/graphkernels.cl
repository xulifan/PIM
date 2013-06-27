#define infinity INFINITY//0x100000
#if defined(cl_khr_fp64)  // Khronos extension available?
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#elif defined(cl_amd_fp64)  // AMD extension available?
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#endif



__kernel void vertex_gauss_kernel(__global double *vert_gaussian, __global double *feat_g1, __global double *feat_g2, int n_feat, int n1, int n2, double paramy)
{
    int idx1 = get_global_id(0);
    int idx2 = get_global_id(1);
    
if(idx1 < n1 && idx2 < n2){
    int g1_offset=idx1*n_feat;
    int g2_offset=idx2*n_feat;
    double vk=0;
    for (int i = 0 ; i < n_feat ; i ++) {
		double diff = feat_g1[g1_offset + i] - feat_g2[g2_offset + i];
		vk += (diff * diff);
        
    }
	vert_gaussian[idx1 * n2 + idx2] = exp(-vk/paramy);
}   
    return;
} 


__kernel void edge_kernel_1d(__global double *edge_kernel, __global double *vertex_kernel, __global double *edge_g1, __global double *edge_g2, __global int *edge_x1, __global int * edge_x2, __global int *edge_y1, __global int *edge_y2, int n_edge1, int n_edge2, int n_node1, int n_node2, double paramx)
{
    int i=get_global_id(0);
    int x1,x2,y1,y2;
    double e1,e2;
    double k=0;
    
if(i<n_edge1){
    x1=edge_x1[i];
    y1=edge_y1[i];
    e1=edge_g1[i];
    for(int j=0;j<n_edge2;j++){
    
        x2=edge_x2[j];
        y2=edge_y2[j];
        e2=edge_g2[j];
        
        double k_edge = fmax(0.0,paramx - fabs(e1-e2));
        if(k_edge>0) k_edge = vertex_kernel[x1*n_node2+x2] * k_edge * vertex_kernel[y1*n_node2+y2];
        k+=k_edge;
    }
    edge_kernel[i]=k;
}

    return;
}



__kernel void reduce(__global double *input, __global double *output, int n, __local double *share)
{
    unsigned int tid = get_local_id(0);
    unsigned int bid = get_group_id(0);
    unsigned int gid = get_global_id(0);
    unsigned int localSize = get_local_size(0);
    
    if(gid<n) share[tid]=input[gid];
    else share[tid]=0;
    
    barrier(CLK_LOCAL_MEM_FENCE);
    
    for(unsigned int s = localSize >> 1; s > 0; s >>= 1) 
    {
        if(tid < s && gid < n) 
        {
            share[tid] += share[tid + s];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    if(tid == 0) output[bid] = share[0];
    
    return;
}


__kernel void reduce_nolocal(__global double *input, __global double *output, int n)
{
    unsigned int tid = get_local_id(0);
    unsigned int bid = get_group_id(0);
    unsigned int gid = get_global_id(0);
    unsigned int localSize = get_local_size(0);
    
    double sum=0;
    if(tid == 0){
        for(unsigned int i=0;i<localSize;i++){
            if(gid+i<n) sum+=input[gid+i];
        }
        output[bid] = sum;
    }
    return;
}


double gaussian(__global double *feat_g1, __global double *feat_g2, int idx1, int idx2, int n_feat, double paramy)
{
    int g1_offset=idx1*n_feat;
    int g2_offset=idx2*n_feat;
    double vk=0;
    for (int i = 0 ; i < n_feat ; i ++) {
		double diff = feat_g1[g1_offset + i] - feat_g2[g2_offset + i];
		vk += (diff * diff);
	}
	return exp(-vk/paramy);
}

// no pre-calculation of vertex_kernel
// calculate vertex_kernel when needed
__kernel void edge_kernel_multipim_1(__global double *edge_kernel, __global double *feat_g1, __global double *feat_g2, __global double *edge_g1, __global double *edge_g2, __global int *edge_x1, __global int * edge_x2, __global int *edge_y1, __global int *edge_y2, int n_edge1, int n_edge2, int n_node1, int n_node2, int n_feat, double paramx, double paramy, int start_edge, int end_edge, int own_num_edge)
{
    int i=get_global_id(0);
    int x1,x2,y1,y2;
    double e1,e2;
    double k=0;
    
if(i<own_num_edge){
    int edge_id=i+start_edge;
    x1=edge_x1[edge_id];
    y1=edge_y1[edge_id];
    e1=edge_g1[edge_id];
    for(int j=0;j<n_edge2;j++){
    
        x2=edge_x2[j];
        y2=edge_y2[j];
        e2=edge_g2[j];
        
        double k_edge = fmax(0.0,paramx - fabs(e1-e2));
        if(k_edge>0) k_edge = gaussian(feat_g1,feat_g2,x1,x2,n_feat,paramy) * k_edge * gaussian(feat_g1,feat_g2,y1,y2,n_feat,paramy);
        k+=k_edge;
    }
    edge_kernel[i]=k;
}

    return;
}

__kernel void edge_kernel_multipim_2(__global double *edge_kernel, __global double *vertex_kernel, __global double *edge_g1, __global double *edge_g2, __global int *edge_x1, __global int * edge_x2, __global int *edge_y1, __global int *edge_y2, int n_edge1, int n_edge2, int n_node1, int n_node2, double paramx, int start_edge, int end_edge, int own_num_edge)
{
    int i=get_global_id(0);
    int x1,x2,y1,y2;
    double e1,e2;
    double k=0;
    
if(i<own_num_edge){
    int edge_id=i+start_edge;
    x1=edge_x1[edge_id];
    y1=edge_y1[edge_id];
    e1=edge_g1[edge_id];
    for(int j=0;j<n_edge2;j++){
    
        x2=edge_x2[j];
        y2=edge_y2[j];
        e2=edge_g2[j];
        
        double k_edge = fmax(0.0,paramx - fabs(e1-e2));
        if(k_edge>0) k_edge = vertex_kernel[x1*n_node2+x2] * k_edge * vertex_kernel[y1*n_node2+y2];
        k+=k_edge;
    }
    edge_kernel[i]=k;
}

    return;
}
