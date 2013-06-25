void NormalizeKMatrix()
{
    double **k_temp=malloc2ddouble(num_graph,num_graph);
    for(int i=0;i<num_graph;i++){
        for(int j=0;j<num_graph;j++){
            k_temp[i][j] = K_Matrix[i][j]/sqrtf(K_Matrix[i][i]*K_Matrix[j][j]);
            printf("%f ",k_temp[i][j]);
        }
        printf("\n");
    }
    //print2d(k_temp,num_graph,num_graph);
    free2d(k_temp);
}

void insert(int *dest, int num, int n)
{
    for(int i=1;i<n;i++){
        if(dest[i]==0){ dest[i]=num; return;}
        else{
            if(num<dest[i]){
                for(int j=n-1;j>i;j--) dest[j]=dest[j-1];
                dest[i]=num;
                return;
            }
            else i=i;
        }
    }
    return;
}

int iskey(char **dest, char *str, int n)
{
    for(int i=0;i<n;i++){
        if(strcmp(dest[i],str)==0) return i;
    }
    return -1;
}


int lookupchar(char **dest, char *str, int n)
{
    for(int i=0;i<n;i++){
        if(strcmp(dest[i],str)==0) return i;
    }
    return -1;
}

int lookupint(int **dest, int *key, int dim0, int dim1)
{
    for(int i=0;i<dim0;i++){
        //print1dint(dest[i],dim1);
        //print1dint(key,dim1);
        if(memcmp(dest[i],key,sizeof(int)*dim1)==0) return i;
    }
    return -1;
}



double rtclock()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double t1;
    t1=tv.tv_sec+(tv.tv_usec/1000000.0);
    
    return t1;
}





template <typename T>
void print1d(T *a,int x)
{
    cout<<endl;
    for(int i=0;i<x;i++){
        cout<<a[i]<<" ";
    }
    cout<<endl;
    cout<<endl;
    return;
}




template <typename T>
void print2d(T **a,int x, int y)
{
    cout<<endl;
    for(int i=0;i<x;i++){
        for(int j=0;j<y;j++){
            cout<<a[i][j]<<" ";
        }
        cout<<endl;
    }
    cout<<endl;
    return;
}

template <typename T>
void print3d(T ***a,int x, int y, int z)
{
    cout<<endl;
    for(int i=0;i<x;i++){
        for(int j=0;j<y;j++){
            for(int k=0;k<z;k++){
                cout<<a[i][j][k]<<" ";
            }
            cout<<endl;
        }
        cout<<endl;
    }
    cout<<endl;
    cout<<endl;
    return;
}

template <typename T>
void print1dto2d(T *a, int x, int y)
{
    cout<<endl;
    for(int i=0;i<x;i++){
        for(int j=0;j<y;j++){
            cout<<a[i*y+j]<<" ";
        }
        cout<<endl;
    }
    cout<<endl;
    return;
}


template <typename T>
void array_init(T *a, int n, T val)
{
    for(int i=0;i<n;i++) a[i]=val;
    return;
}

 

template <typename T> void matrixmul_cpu(T *m1, int rowa, int cola, T *m2, int rowb, int colb, T *m3)
{
    for(int i=0;i<rowa;i++){
        for(int j=0;j<colb;j++){
            T k=0;
            for(int n=0;n<cola;n++){
                k+=m1[i*cola+n]*m2[n*colb+j];
            }
            m3[i*colb+j]=k;
        }
    }
    //print1dto2d(m3,rowa,colb);
    return;
}

template <typename T> T vect_sum(T *a, int n)
{
    T sum=a[0];
    for(int i=1;i<n;i++) sum+=a[i];
    //cout<<"sum is : "<<sum<<endl;
    return sum;
}

/*
** a = a + xb 
*/
template <typename T> void vect_add(T *a, T *b, int n, double x)
{
    
    for(int i=0;i<n;i++) a[i]+=x * b[i];
    
    return;
}

template <typename T> void printgpumem1d(cl_mem a, int n, T nth)
{
    T *temp= new T[n];
    errcode = clEnqueueReadBuffer(clCommandQue, a, CL_TRUE, 0, sizeof(T)*n, temp, 0, NULL, NULL);
    if(errcode != CL_SUCCESS) {printf("Error in reading printgpumem1d mem %d\n",errcode); exit(EXIT_FAILURE);}
    print1d(temp,n);
    delete[] temp;
    return;
}

template <typename T> void printgpumem2d(cl_mem a, int n, int m,T nth)
{
    T *temp= new T[n*m];
    errcode = clEnqueueReadBuffer(clCommandQue, a, CL_TRUE, 0, sizeof(T)*n*m, temp, 0, NULL, NULL);
    if(errcode != CL_SUCCESS) {printf("Error in reading printgpumem1d mem %d\n",errcode); exit(EXIT_FAILURE);}
    print1dto2d(temp,n,m);
    delete[] temp;
    return;
}

void OpenCLErrorCheck(const cl_int status, const char * func)
{
    if(status == CL_SUCCESS){ return;}
    else{
        printf("%s with error ",func);
        switch (status) {
        case CL_DEVICE_NOT_FOUND:                           printf("CL_DEVICE_NOT_FOUND\n");  break;
        case CL_DEVICE_NOT_AVAILABLE :                      printf("CL_DEVICE_NOT_AVAILABLE \n");  break;
        case CL_COMPILER_NOT_AVAILABLE:                     printf("CL_COMPILER_NOT_AVAILABLE\n");  break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:              printf("CL_MEM_OBJECT_ALLOCATION_FAILURE\n");  break;
        case CL_OUT_OF_RESOURCES:                           printf("CL_OUT_OF_RESOURCES\n");  break;
        case CL_OUT_OF_HOST_MEMORY:                         printf("CL_OUT_OF_HOST_MEMORY\n");  break;
        case CL_PROFILING_INFO_NOT_AVAILABLE:               printf("CL_PROFILING_INFO_NOT_AVAILABLE\n");  break;
        case CL_MEM_COPY_OVERLAP:                           printf("CL_MEM_COPY_OVERLAP\n");  break;
        case CL_IMAGE_FORMAT_MISMATCH:                      printf("CL_IMAGE_FORMAT_MISMATCH\n");  break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:                 printf("CL_IMAGE_FORMAT_NOT_SUPPORTED\n");  break;
        case CL_BUILD_PROGRAM_FAILURE:                      printf("CL_BUILD_PROGRAM_FAILURE\n");  break;
        case CL_MAP_FAILURE:                                printf("CL_MAP_FAILURE\n");  break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:               printf("CL_MISALIGNED_SUB_BUFFER_OFFSET\n");  break;
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:  printf("CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST\n");  break;
        case CL_INVALID_VALUE:                              printf("CL_INVALID_VALUE\n");  break;
        case CL_INVALID_DEVICE_TYPE:                        printf("CL_INVALID_DEVICE_TYPE\n");  break;
        case CL_INVALID_PLATFORM:                           printf("CL_INVALID_PLATFORM\n");  break;
        case CL_INVALID_DEVICE:                             printf("CL_INVALID_DEVICE\n");  break;
        case CL_INVALID_CONTEXT:                            printf("CL_INVALID_CONTEXT\n");  break;
        case CL_INVALID_QUEUE_PROPERTIES:                   printf("CL_INVALID_QUEUE_PROPERTIES\n");  break;
        case CL_INVALID_COMMAND_QUEUE:                      printf("CL_INVALID_COMMAND_QUEUE\n");  break;
        case CL_INVALID_HOST_PTR:                           printf("CL_INVALID_HOST_PTR\n");  break;
        case CL_INVALID_MEM_OBJECT:                         printf("CL_INVALID_MEM_OBJECT\n");  break;
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:            printf("CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n");  break;
        case CL_INVALID_IMAGE_SIZE:                         printf("CL_INVALID_IMAGE_SIZE\n");  break;
        case CL_INVALID_SAMPLER:                            printf("CL_INVALID_SAMPLER\n");  break;
        case CL_INVALID_BINARY:                             printf("CL_INVALID_BINARY\n");  break;
        case CL_INVALID_BUILD_OPTIONS:                      printf("CL_INVALID_BUILD_OPTIONS\n");  break;
        case CL_INVALID_PROGRAM:                            printf("CL_INVALID_PROGRAM\n");  break;
        case CL_INVALID_PROGRAM_EXECUTABLE:                 printf("CL_INVALID_PROGRAM_EXECUTABLE\n");  break;
        case CL_INVALID_KERNEL_NAME:                        printf("CL_INVALID_KERNEL_NAME\n");  break;
        case CL_INVALID_KERNEL_DEFINITION:                  printf("CL_INVALID_KERNEL_DEFINITION\n");  break;
        case CL_INVALID_KERNEL:                             printf("CL_INVALID_KERNEL\n");  break;
        case CL_INVALID_ARG_INDEX:                          printf("CL_INVALID_ARG_INDEX\n");  break;
        case CL_INVALID_ARG_VALUE:                          printf("CL_INVALID_ARG_VALUE\n");  break;
        case CL_INVALID_ARG_SIZE:                           printf("CL_INVALID_ARG_SIZE\n");  break;
        case CL_INVALID_KERNEL_ARGS:                        printf("CL_INVALID_KERNEL_ARGS\n");  break;
        case CL_INVALID_WORK_DIMENSION:                     printf("CL_INVALID_WORK_DIMENSION\n");  break;
        case CL_INVALID_WORK_GROUP_SIZE:                    printf("CL_INVALID_WORK_GROUP_SIZE\n");  break;
        case CL_INVALID_WORK_ITEM_SIZE:                     printf("CL_INVALID_WORK_ITEM_SIZE\n");  break;
        case CL_INVALID_GLOBAL_OFFSET:                      printf("CL_INVALID_GLOBAL_OFFSET\n");  break;
        case CL_INVALID_EVENT_WAIT_LIST:                    printf("CL_INVALID_EVENT_WAIT_LIST\n");  break;
        case CL_INVALID_EVENT:                              printf("CL_INVALID_EVENT\n");  break;
        case CL_INVALID_OPERATION:                          printf("CL_INVALID_OPERATION\n");  break;
        case CL_INVALID_GL_OBJECT:                          printf("CL_INVALID_GL_OBJECT\n");  break;
        case CL_INVALID_BUFFER_SIZE:                        printf("CL_INVALID_BUFFER_SIZE\n");  break;
        case CL_INVALID_MIP_LEVEL:                          printf("CL_INVALID_MIP_LEVEL\n");  break;
        case CL_INVALID_GLOBAL_WORK_SIZE:                   printf("CL_INVALID_GLOBAL_WORK_SIZE\n");  break;
        //case CL_INVALID_PROPERTY:                           printf("CL_INVALID_PROPERTY\n");  break;
        default: printf("unknown error\n"); break;
        }
        exit(EXIT_FAILURE);
    }
}

