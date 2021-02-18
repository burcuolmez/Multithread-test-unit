#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h> 

#define PATIENT_NUM 30
#define ROOM_NUM 8
#define ROOM_CAPACITY 3

void *patient(void *); //for patient operations
void *room(void *); //for room operations
void simulation() ;

pthread_t rtid[ROOM_NUM]; //created threads for rooms and patients
pthread_t tid[PATIENT_NUM];

//created 2 main semaphores and 2 critical area semaphores
sem_t hospital;
sem_t healthcare;
sem_t criticialAreaPatient;
sem_t criticialAreaRoom;

int roomNo=0; // room no has created as a global variable because we need to reach from patient function
int allDone = 0; //useful to end the day
int countOfRoom=0; //to check capacity of each room
int countOfPeople=0; //total number of people who has tested

int main(int argc, char *argv[]){
    //created  ids for each thread 
    int patients[PATIENT_NUM];
    int rooms[ROOM_NUM];
    int i=0;
    for (i = 0; i < PATIENT_NUM; i++) {
        patients[i] = i;
    }
    for (i = 0; i < ROOM_NUM; i++) {
        rooms[i] = i;
    }
    //initialized each semaphore 
    sem_init(&hospital, 0, ROOM_CAPACITY*ROOM_NUM); //it is hospital capacity
    sem_init(&healthcare, 0, 0);  //to proceed test in each room
    sem_init(&criticialAreaPatient, 0, 1);  //patients critical area
    sem_init(&criticialAreaRoom, 0, 0); //to start and finish rooms process
    
    for (i = 0; i < ROOM_NUM; i++) {
        pthread_create(&rtid[i], NULL, room, (void *)&rooms[i]);
    }
    for (i = 0; i < PATIENT_NUM; i++) {
        pthread_create(&tid[i], NULL, patient, (void *)&patients[i]);
    }
     for (i = 0; i < PATIENT_NUM; i++) {
        pthread_join(tid[i],NULL);
    }
    allDone = 1; // to send healthcare stuff to home
    for (i = 0; i < ROOM_NUM; i++) {
        sem_post(&criticialAreaRoom); //we need to post it here for each room because semaphore waits in the while loop
    }
    
    
    for (i = 0; i < ROOM_NUM; i++) {
        pthread_join(rtid[i],NULL);
    }
    printf("The Healthcare stuff are going home for the day.\n");
    
    return 0;
    
}

void *patient(void *number) {
    sem_wait(&hospital);
    int patientNo = *(int *)number;
    int random= rand()%7+1;
    sleep(random); //people will come to unit in random periods
    printf("Patient %d entering the hospital.\n", patientNo+1);
    sem_wait(&criticialAreaPatient); //used semaphore as mutex here

    if(countOfRoom==0){
       
        sem_post(&criticialAreaRoom);
    }
    sleep(4); //to get rooms id 
    countOfRoom++;
    printf("Patient %d is at Covid 19 Test Unit %d's waiting room.\n", patientNo+1,roomNo+1);
    simulation();

    sem_post(&criticialAreaPatient); //released semaphore to keep people coming in continuosly
}
void *room(void *numb) {
    int rn= *(int *)numb;
    
    while (!allDone)
    {
        printf("Healthcare stuff %d is ventilating the Covid 19 Test Unit %d's waiting room.\n", rn+1,rn+1);
        //sleep(2);
        sem_wait(&criticialAreaRoom); // to wait for room capacity
        roomNo=rn; // to proceed true rooms process
        if(!allDone){
            
            sleep(2); 
            sem_wait(&healthcare);
            sleep(3); //to avoid confusion about room ids
            int i;
            for (i = 0; i < 3; i++)
            {
                sem_post(&hospital); //so new person can come in to the hospital
                countOfPeople++;
            }
            
            printf("Room no: %d has finished testing\n",rn+1);
            printf("%d people have tested.\n",countOfPeople);

        }
        else{
            break;
        }
    }
    
}

void simulation(){
   
    if(countOfRoom==1){
        printf("Room no: %d [X][][]\n",roomNo+1);
        printf("Healthcare stuff %d is anouncing 'the Covid 19 Test Unit %d's waiting room has %d empty place. Lets start!'\n", roomNo+1,roomNo+1,3-countOfRoom);
    
    }
    else if(countOfRoom==2){
        printf("Room no: %d [X][X][]\n",roomNo+1);
        printf("Healthcare stuff %d is anouncing 'the Covid 19 Test Unit %d's waiting room has %d empty place. Lets start!'\n", roomNo+1,roomNo+1,3-countOfRoom);
    }
    else if(countOfRoom==3){

        printf("Room no: %d [X][X][X]\n",roomNo+1);
        printf("Room no: %d has started\n",roomNo+1);
        sem_post(&healthcare); //to start rooms process 
        countOfRoom=0;
        
        
    }

}