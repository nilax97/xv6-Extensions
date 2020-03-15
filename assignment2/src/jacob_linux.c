#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

// #define N 11
// #define E 0.00001
// #define T 100.0
// #define P 2
// #define L 20000

float fabsm(float a){
	if(a<0)
	return -1*a;
return a;
}

int main(int argc, char *argv[])
{
	int N,L,P;
	float E,T;
	FILE *f1 = fopen("assig2a.inp","r");
	fscanf(f1,"%d%f%f%d%d",&N,&E,&T,&P,&L);
	float diff;
	int i,j,k;
	float mean = 0.0;
	float u[N][N];
	float w[N][N];
	int count = 0;

	float finished = 0.0;

	int child_pid[P];
	int from_parent[P][2];
	int to_parent[P][2];
	int child_front[P-1][2];
	int child_back[P-1][2];

	for(i=0; i<P; ++i)
	{
		child_pid[i] = -1;
		pipe(from_parent[i]);
		pipe(to_parent[i]);
		if(i<P-1)
		{
			pipe(child_front[i]);
			pipe(child_back[i]);
		}
	}

	for (i = 0; i < N; i++){
		u[i][0] = u[i][N-1] = u[0][i] = T;
		u[N-1][i] = 0.0;
		mean += u[i][0] + u[i][N-1] + u[0][i] + u[N-1][i];
	}
	mean /= (4.0 * N);

	for (i = 1; i < N-1; i++ )
		for ( j= 1; j < N-1; j++) 
			u[i][j] = mean;

	for(i=0; i<P; ++i)
	{
		child_pid[i] = fork();
		if(child_pid[i] == 0)
			break;
	}

	for(i=0; i<P; ++i)
	{
		if(child_pid[i]==0)
		{
			int start = i * (N-2)/P;
			int end = (i+1) * (N-2)/P + 1;
			if(i == P-1)
				end = N-1;
			int len = (end - start + 1) * N;

			read(from_parent[i][0], &finished, sizeof(float));

			while(!finished)
			{
				if(i != 0)
					write(child_back[i-1][1], u + (start + 1), N * sizeof(float));
				if(i != P-1)
					write(child_front[i][1], u + (end - 1), N * sizeof(float));
				if(i != 0)
					read(child_front[i-1][0], u + start, N * sizeof(float));
				if(i != P-1)
					read(child_back[i][0], u + end, N * sizeof(float));
				diff = 0.0;
				for(j=start+1; j<end; ++j)
				{
					for(k=1; k<N-1; ++k)
					{
						w[j][k] = (u[j+1][k] + u[j-1][k] + u[j][k+1] + u[j][k-1])/4.0;
						if(fabsm(w[j][k] - u[j][k]) > diff)
							diff = fabsm(w[j][k] - u[j][k]);
					}
				}
				for(j=start+1; j<end; ++j)
				{
					for(k=1; k<N-1; ++k)
					{
						u[j][k] = w[j][k];
					}
				}

				write(to_parent[i][1], &diff, sizeof(float));
				read(from_parent[i][0], &finished, sizeof(float));
			}

			write(to_parent[i][1], u + (start + 1), (len - 2*N) * sizeof(float));
			exit(0);
		}
	}
	for(;;)
	{
		count++;
		diff = 0.0;
		for(i=0; i<P; ++i)
			write(from_parent[i][1], &finished, sizeof(float));

		for(i=0; i<P; ++i)
		{
			float diff_temp;
			read(to_parent[i][0], &diff_temp, sizeof(float));
			if(diff_temp > diff)
				diff = diff_temp;
		}
		if(diff < E || count > L)
		{
			finished = 1.0;
			for(i=0; i<P; ++i)
			{
				write(from_parent[i][1], &finished, sizeof(float));
				int start = i * (N-2)/P;
				int end = (i+1) * (N-2)/P + 1;
				if(i == P-1)
					end = N-1;
				int len = (end - start + 1) * N;
				read(to_parent[i][0], u + start + 1, (len - 2*N) * sizeof(float));
			}
			break;
		}
	}
	for(i =0; i <N; i++){
		for(j = 0; j<N; j++)
			printf("%d ",((int)u[i][j]));
		printf("\n");
	}
	exit(0);
}