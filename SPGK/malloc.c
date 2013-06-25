template <typename T>
void free2d(T **a)
{
    free(a[0]);
    free(a);
}

template <typename T>
void free3d(T ***a)
{
    free(a[0][0]);
    free(a[0]);
    free(a);

}


 
int **malloc2d(int x, int y)
{
    int i;
    int j;
    int **a;
    a = (int **)malloc(sizeof(int *) * x);
    if(a == NULL)
	{
	    printf("memory allocation error\n");
	    exit(EXIT_FAILURE);
	}
    a[0] = (int *)malloc(sizeof(int) * x * y);
    if(a[0] == NULL)
	{
        printf("memory allocation error\n");
        exit(EXIT_FAILURE);
	}
    for(i = 1; i < x; i++)
    {
        a[i] = a[i-1] + y;
    }
    for(i =0; i < x; i++)
    {
        for(j = 0; j < y; j++)
        {
           a[i][j] = 0;
        }
    }
	return a;
}

float **malloc2dfloat(int x, int y)
{
    int i;
    int j;
    float **a;
    a = (float **)malloc(sizeof(float *) * x);
    if(a == NULL)
	{
	    printf("memory allocation error\n");
	    exit(EXIT_FAILURE);
	}
    a[0] = (float *)malloc(sizeof(float) * x * y);
    if(a[0] == NULL)
	{
        printf("memory allocation error\n");
        exit(EXIT_FAILURE);
	}
    for(i = 1; i < x; i++)
    {
        a[i] = a[i-1] + y;
    }
    for(i =0; i < x; i++)
    {
        for(j = 0; j < y; j++)
        {
           a[i][j] = 0;
        }
    }
	return a;
}

double **malloc2ddouble(int x, int y)
{
    int i;
    int j;
    double **a;
    a = (double **)malloc(sizeof(double *) * x);
    if(a == NULL)
	{
	    printf("memory allocation error\n");
	    exit(EXIT_FAILURE);
	}
    a[0] = (double *)malloc(sizeof(double) * x * y);
    if(a[0] == NULL)
	{
        printf("memory allocation error\n");
        exit(EXIT_FAILURE);
	}
    for(i = 1; i < x; i++)
    {
        a[i] = a[i-1] + y;
    }
    for(i =0; i < x; i++)
    {
        for(j = 0; j < y; j++)
        {
           a[i][j] = 0;
        }
    }
	return a;
}

char **malloc2dchar(int x, int y)
{
    int i;
    int j;
    char **a;
    a = (char **)malloc(sizeof(char *) * x);
    if(a == NULL)
	{
	    printf("memory allocation error\n");
	    exit(EXIT_FAILURE);
	}
    a[0] = (char *)malloc(sizeof(char) * x * y);
    if(a[0] == NULL)
	{
        printf("memory allocation error\n");
        exit(EXIT_FAILURE);
	}
    for(i = 1; i < x; i++)
    {
        a[i] = a[i-1] + y;
    }
    for(i =0; i < x; i++)
    {
        for(j = 0; j < y; j++)
        {
           a[i][j] = 0;
        }
    }
	return a;
}



int ***malloc3d(int x, int y, int z)
{
    int i;
    int j;
    int k;
    int ***a;
    int **a_xy;
    int *a_xyz;
	a = (int ***)malloc(sizeof(int **) * x);
	if(a == NULL)
	{
	    printf("memory allocation error\n");
	    exit(EXIT_FAILURE);
	}
	a_xy = (int **)malloc(sizeof(int *) * x * y);
	if(a_xy == NULL)
	{
	    printf("memory allocation error\n");
	    exit(EXIT_FAILURE);
	}
	a_xyz = (int *) malloc(sizeof(int) * x * y * z);
	if(a_xyz == NULL)
	{
	    printf("memory allocation error\n");
	    exit(EXIT_FAILURE);
	}
	for(i =0; i< x; i++,a_xy+= y)
	{
	    a[i] = a_xy;
	    for(j = 0; j < y; j++, a_xyz += z)
	    {
	        a[i][j] = a_xyz;
	    }
	}
	for(i =0; i < x; i++)
    {
        for(j = 0; j < y; j++)
        {
            for(k = 0; k < z; k++)
            {
               a[i][j][k] =0;
            }
        }
    }

	return a;
}

char ***malloc3dchar(int x, int y, int z)
{
    int i;
    int j;
    int k;
    char ***a;
    char **a_xy;
    char *a_xyz;
	a = (char ***)malloc(sizeof(char **) * x);
	if(a == NULL)
	{
	    printf("memory allocation error\n");
	    exit(EXIT_FAILURE);
	}
	a_xy = (char **)malloc(sizeof(char *) * x * y);
	if(a_xy == NULL)
	{
	    printf("memory allocation error\n");
	    exit(EXIT_FAILURE);
	}
	a_xyz = (char *) malloc(sizeof(char) * x * y * z);
	if(a_xyz == NULL)
	{
	    printf("memory allocation error\n");
	    exit(EXIT_FAILURE);
	}
	for(i =0; i< x; i++,a_xy+= y)
	{
	    a[i] = a_xy;
	    for(j = 0; j < y; j++, a_xyz += z)
	    {
	        a[i][j] = a_xyz;
	    }
	}
	for(i =0; i < x; i++)
    {
        for(j = 0; j < y; j++)
        {
            for(k = 0; k < z; k++)
            {
               a[i][j][k] =0;
            }
        }
    }

	return a;
}

double ***malloc3ddouble(int x, int y, int z)
{
    int i;
    int j;
    int k;
    double ***a;
    double **a_xy;
    double *a_xyz;
	a = (double ***)malloc(sizeof(double **) * x);
	if(a == NULL)
	{
	    printf("memory allocation error\n");
	    exit(EXIT_FAILURE);
	}
	a_xy = (double **)malloc(sizeof(double *) * x * y);
	if(a_xy == NULL)
	{
	    printf("memory allocation error\n");
	    exit(EXIT_FAILURE);
	}
	a_xyz = (double *) malloc(sizeof(double) * x * y * z);
	if(a_xyz == NULL)
	{
	    printf("memory allocation error\n");
	    exit(EXIT_FAILURE);
	}
	for(i =0; i< x; i++,a_xy+= y)
	{
	    a[i] = a_xy;
	    for(j = 0; j < y; j++, a_xyz += z)
	    {
	        a[i][j] = a_xyz;
	    }
	}
	for(i =0; i < x; i++)
    {
        for(j = 0; j < y; j++)
        {
            for(k = 0; k < z; k++)
            {
               a[i][j][k] =0;
            }
        }
    }

	return a;
}

