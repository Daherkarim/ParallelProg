#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>


float gClock = 8.00;
int j = 0;

int flag_empty =0;


struct service
{
	float time_needed;
	int type_service; 		 	 // type of service
	float schedule;  		 
	char name[20];					 // Deposit or Withdraw
};

struct service services[10];
int size = sizeof(services)/32;

pthread_t *pservices;
pthread_t *ptellers;


sem_t buffer_mutex,empty_count,fill_count;

int q_pos=0,prod_count,con_count,buf_len;





int randNum()
{
  int i, stime;
  long ltime;
  ltime = time(NULL);
  stime = (unsigned) ltime/2;
  srand(stime);

  int r = rand()%2;

  return r;
}

float randClock(){
	int flag = 0;
  	int i, stime;
  	long ltime;
  	ltime = time(NULL);
  	stime = (unsigned) ltime/2;
  	srand(stime);

  	float r = rand()%100;
   	r = r/200;

		while(!flag){
			float max = rand()%14;
  			float min = rand()%8;
  	 		float sum = max-min;

  	 		if(sum>=8){
  	 			flag = 1;
  	 			r = r+sum;
  	 		}

  	 	}


  return r;
}




struct service produce(pthread_t self){
	struct service s;
	s.type_service = randNum();
	s.schedule = randClock();
	int i =0;


	while(!pthread_equal(*(pservices+i),self) && i < prod_count){
		i++;
	}


	

		if(s.type_service == 0 ){
				strcpy(s.name,"Deposit");
				s.time_needed += 0.3;
				printf("A new customer requested a Deposit at time %f\n Global clock is : %f\n",s.schedule,gClock);
				sleep(1 + rand()%3);
			}
		
		else{
				strcpy(s.name,"Withdrawal");
				s.time_needed += 0.7;
				printf("A new customer requested a Withdrawal at time %f\n Global clock is : %f\n",s.schedule,gClock);
				sleep(1 + rand()%3);
			}
	


	return s;
}


void* producer(void *args){

	while(1){
		

		sem_wait(&empty_count);
		sem_wait(&buffer_mutex);
		
		struct service p = produce(pthread_self());
		
		if(q_pos<=0 &&services[0].schedule<=0){
			flag_empty=1;
		}

		if(p.schedule>gClock && flag_empty){
			services[0] = p;
			printf("Added a new Service at pos %d\n",q_pos);
			++q_pos;
		}


		  else if(p.schedule>gClock && q_pos <size && flag_empty){
			services[q_pos] = p;		
			printf("Added a new Service at pos %d\n",q_pos);
			++q_pos;

		}


		sem_post(&buffer_mutex);
		sem_post(&fill_count);
		sleep(1 + rand()%3);

		if(gClock>=14){
			printf("DONE FOR THE DAY....");
			exit(0);
		}



	}


	
	return NULL;
}



void consume(float num,pthread_t self){
	int i = 0;
	while(!pthread_equal(*(ptellers+i),self) && i < con_count){
		i++;
	}


	printf("List of remaining jobs:\n");
	for(i=0;i<q_pos;++i){
		printf("Job: #%d  Type: %s  Arrival Time: %f\nGlobal Clock: %f\n\n",i,services[i].name,services[i].schedule,gClock);

	}
		float difference = services[q_pos-1].schedule - gClock;
		printf("\nTeller wait for %f and serviced  job at time : %f \n",difference,services[--q_pos].schedule);
	
	
}



void* consumer(void *args){
	
	while(1){
		
		
		sem_wait(&fill_count);
		sem_wait(&buffer_mutex);


		if(q_pos>0){
		float c = services[q_pos].schedule;
		consume(c,pthread_self());
		gClock = services[q_pos].schedule + services[q_pos].time_needed;
		

		}	
		
		sem_post(&buffer_mutex);
		sem_post(&empty_count);
		sleep(3+rand()%5);
	
	}

	return NULL;
}



int main(){
	int i,err;

	sem_init(&buffer_mutex,0,1);
	sem_init(&fill_count,0,0);
	sem_init(&empty_count,0,10);

	printf("The number of Producers threads if always 1\n");
	prod_count =1;
	pservices = (pthread_t*) malloc(prod_count*sizeof(pthread_t));

	printf("Enter the number of Teller threads:");
	scanf("%d",&con_count);
	ptellers = (pthread_t*) malloc(con_count*sizeof(pthread_t));

	

	for(i=0;i<prod_count;i++){
		err = pthread_create(pservices+i,NULL,&producer,NULL);
		if(err != 0){
			printf("Error creating producer %d: %s\n",i+1,strerror(err));
		}else{
			printf("Successfully created customer %d\n",i+1);
		}
	}

	for(i=0;i<con_count;i++){
		err = pthread_create(ptellers+i,NULL,&consumer,NULL);
		if(err != 0){
			printf("Error creating consumer %d: %s\n",i+1,strerror(err));
		}else{
			printf("Successfully created teller %d\n",i+1);
			sleep(1);

		}
	}

	for(i=0;i<prod_count;i++){
		pthread_join(*(pservices+i),NULL);
	}
	for(i=0;i<con_count;i++){
		pthread_join(*(ptellers+i),NULL);
	}


	return 0;
	

}



