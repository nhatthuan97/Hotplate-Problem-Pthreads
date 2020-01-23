#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#define MAXTHREADS 32
///usr/bin/time -p  ./p2  1500 3000 100 100 100 400 0.015 16
double timestamp()
{
    struct timeval tv;

    gettimeofday( &tv, ( struct timezone * ) 0 );
    return ( tv.tv_sec + (tv.tv_usec / 1000000.0) );
}
float **arr1;
float **arr2;
pthread_barrier_t barr;
int remains;
int stand_cal;
int num_threads;
float delta_list[MAXTHREADS];
void *mainProcedures(void *arg);
float epsilon;
int itt;

int process_finished;
int num_cols,num_rows;
pthread_mutex_t lock;
int main(int argc, char *argv[]){
  num_rows=atoi(argv[1]);
  num_cols=atoi(argv[2]);
  float top_temp=atof(argv[3]);
  float left_temp=atof(argv[4]);
  float right_temp=atof(argv[5]);
  float bot_temp=atof(argv[6]);
  epsilon=atof(argv[7]);
  num_threads = atoi(argv[8]);

  arr1 = (float **)malloc(num_rows * sizeof(float *));
  arr2 = (float **)malloc(num_rows * sizeof(float *));
  for (int i=0;i<num_rows;i++){
    arr1[i]=(float *)malloc(num_cols*sizeof(float));
    arr2[i]=(float *)malloc(num_cols*sizeof(float));
  }


  //Initialized old array
  for (int i=0; i<num_rows;i++){
    for (int j=0;j<num_cols;j++){
      arr1[i][j]=0;
      arr2[i][j]=0;
    }
  }
  //Initialized array top rows
  for (int i=0;i<num_cols;i++){
    arr1[0][i]=top_temp;
    arr2[0][i]=top_temp;
  }
  //Initialized lef cols
  for (int i=0;i<num_rows;i++){
    arr1[i][0]=left_temp;
    arr2[i][0]=left_temp;
  }
  //Right cols next
  for (int i=0;i<num_rows;i++){
    arr1[i][num_cols-1]=right_temp;
    arr2[i][num_cols-1]=right_temp;
  }
  //Bot rows last
  for (int i=0;i<num_cols;i++){
    arr1[num_rows-1][i]=bot_temp;
    arr2[num_rows-1][i]=bot_temp;
  }

  //Calculate value to Initialized
  float sum=0;
  float edge_amt=0;
  for (int i=0;i<num_cols;i++){
    sum=arr1[0][i]+arr1[num_rows-1][i]+sum;
    edge_amt=edge_amt+2;
  }
  for (int i=1;i<num_rows-1;i++){
    sum=arr1[i][0]+arr1[i][num_cols-1]+sum;
    edge_amt=edge_amt+2;
  }
  float value = sum/edge_amt;
  for (int i=1; i<num_rows-1;i++){
    for (int j=1;j<num_cols-1;j++){
      arr1[i][j]=value;
    }
  }
  double stime, etime;
  //Parallelized section
  stime=timestamp();
  remains=(num_rows-2)%num_threads;
  stand_cal=(num_rows-2)/num_threads;
  pthread_t tid[num_threads];
  int myid[num_threads];
  pthread_barrier_init(&barr,NULL,num_threads);
  itt=0;
  for (int i=1;i<num_threads;i++){
    myid[i]=i;
    pthread_create(&tid[i],NULL,mainProcedures,&myid[i]);
  }
  myid[0]=0;
  *mainProcedures(&myid[0]);
  for (int i=1;i<num_threads;i++){
    pthread_join(tid[i], NULL);
  }
  etime = timestamp();
  printf("TOTAL TIME %.2f\n",etime-stime );
  return 0;
}
void *mainProcedures(void *arg){
  //Get the starting row for calulation and ending rows for calulation
  float max_delta;
  int myid=*((int*) arg);
  int IDforCalculation=myid+1;
  int end = IDforCalculation*stand_cal;
  if (IDforCalculation<=remains){
    end=end+IDforCalculation;
  }
  else{
    end=end+remains;
  }
  int start = end - stand_cal+1;
  if (IDforCalculation<=remains){
    start = start -1;
  }
  do {
    //Calculate data for arrays 2
    float new_value=(arr1[start-1][1]+arr1[start+1][1]+arr1[start][0]+arr1[start][2])/4;
    max_delta = fabs(arr1[start][1]-new_value);
    for (int i=start; i<=end;i++){
      for (int j=1;j<num_cols-1;j++){
        new_value=(arr1[i-1][j]+arr1[i+1][j]+arr1[i][j-1]+arr1[i][j+1])/4;
        arr2[i][j]=new_value;
        if (max_delta<fabs(arr1[i][j] -arr2[i][j])) {
          max_delta=fabs(arr1[i][j] -arr2[i][j]);
        }
      }
    }
    delta_list[myid]=max_delta;
    //Synchronization
    //Find the maxvalue from max_delta
    pthread_barrier_wait(&barr);
    if (myid==0){
      float max_fromList = delta_list[0];
      for (int i =0;i<num_threads;i++){
        if (max_fromList<=delta_list[i]){
          max_fromList=delta_list[i];
        }
      }
      if (max_fromList<=epsilon){
        printf("%d %f\n",itt,max_fromList);
        process_finished=1;
      }
      else{
        process_finished=0;
        //To the power of 2 check
        int logTrunct = (int) log2(itt);
        if (log2(itt)==logTrunct){
          printf("%d %f\n",itt,max_fromList);
        }
        itt++;
        float **arrTemp=arr1;
        arr1=arr2;
        arr2=arrTemp;
      }
    }
    pthread_barrier_wait(&barr);
  }
  while (!process_finished);
}
