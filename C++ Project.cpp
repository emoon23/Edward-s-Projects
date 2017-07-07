#include<iostream>
#include<iomanip>
#include<sys/wait.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<semaphore.h>
#include<errno.h>
#include<sstream>
#include<cstring>
#include<cstdlib>
#include<unistd.h>

//**********************************************
//  PROGRAMMER: Edward Moon 
//  PURPOSE: Write a linux program in C++ to
//           shared memory and semaphores.
//
//  1. Intersection of Lincoln and Annie Glidden
//
//  2. Four traffic lanes: EastBound Lincoln,
//                         WestBound Lincoln,
//                         NorthBound Annie Glidden
//                         SouthBound Annie Glidden
//  3. Avoid deadlock and fight for lanes
//***********************************************

using namespace std;

//sem_t full;  //The semaphore for the full buffer
//sem_t empty;  //The semaphore for the empty buffer
//pthread_mutex_t mutex; //The mutex lock


//This is the structure for the semaphores
struct semstuff_t
 {
   sem_t nw; //Northwest
   sem_t ne; //NorthEast
   sem_t sw;  //SouthWest
   sem_t se;  //SouthEast
   sem_t Cout; //Output
  };

//void semDirect (sem_t direct);
void semDirect(int i, semstuff_t *sem2stff);

int main(int argc, char *argv[])
{

    int segment_id; //Shared memory segment
    int i = 0; //i used for the fork
    semstuff_t *sem2stff; //pointer to struct
    int rtn = 0; //return code

  cout << unitbuf;
  //allocate a shared memory segment
  segment_id = shmget(IPC_PRIVATE, sizeof(semstuff_t),( S_IRUSR | S_IWUSR));
   if(segment_id == -1)
   {
    cout << "Failed to share memory" << endl;
    return 1;

   }
   //shared memory segment
   sem2stff = (semstuff_t *) shmat(segment_id, NULL, 0);
   if(sem2stff == (void *)-1)
    {
     cout << "Failed to attach to shared memory" << endl;
     return 1;
    }

//       sem2stff = new sem_t;
         //Print out shared memory
       cout << "Got Shared memory with id: " << segment_id
            << "\nAttached to shared memory at: " << (void *)sem2stff << endl;

    //Initialize the semaphores(NE, NW, SE, SW)
            sem_init(&sem2stff-> ne,1,1);
            if( rtn != -1)
              {
               cout << "Initialized to semaphore # 0: (NE) to 1" << endl;
               }
              else{
                    cout << "Failed to initialize semaphore NE" << endl;
                  }
             sem_init(&sem2stff->nw,1,1);
             if( rtn != -1)
               {
                 cout << "Initialized to semaphore # 1: (NW) to 1" <<  endl;
                }
              else{
                cout << "Failed to initialze semaphore NW" << endl;
                }
               sem_init(&sem2stff-> se,1,1);
                if(rtn != -1)
                {
                  cout << "Initialized to semaphore # 2: (SE) to 1" << endl;
                 }
                 else{
                 cout << "Failed to initialize semaphore SE" << endl;
                  }
              sem_init(&sem2stff->sw,1,1);
              cout << "Initialized to semaphore # 3: (SW) to 1" << endl;
              sem_init(&sem2stff->Cout,1,1);
              cout << "Initialized to semaphore # 4: for cout\n" << endl;


    //Loop 4 times within the child
    for( i = 0; i < 4; i++)
     {
        rtn = fork(); //Fork process
        if(rtn < 0)
        {
          cout << "Failed to create process" << endl;
          return 1;
          }
        if( rtn == 0) //the child process
          {
            semDirect(i, sem2stff);
            exit(0);
          }
      } //end of for loop
        //parent process
        //Wait for the 4 processes to be done
            wait(NULL);
            wait(NULL);
            wait(NULL);
            wait(NULL);
//            exit(0);

         //Shared memory detach
           rtn = shmdt((void *)sem2stff);
           if(rtn == -1)
            {
               cout << "Failed to detach memory" << endl;
               return 1;
             }
           //Memory freedom
           rtn = shmctl(segment_id, IPC_RMID, NULL);
           if(rtn == -1)
           {
             cout << "Failed to free memory" << endl;
             return 1;
           }
        //exit(0);

} // end of main

// The wrapper function for locking and unlocking semaphores
//The lock for semaphores using sem_wait
int Sem_wait(sem_t *sema)
{
  int retr = sem_wait(sema);
  while(retr == -1)
   {
    if(errno != EINTR)
     {
       //Error message
       cout << "Failed to lock semaphore" << endl;
       exit(-1);
      }
      retr = sem_wait(sema);
    }
   return retr;
 }
  //unlock
 int Sem_post(sem_t *sems)
  {
    int pst = sem_post(sems);
    if( pst == -1)
     {
       //Error message
       cout << "Failed to unlock" << endl;
       exit(-1);
      }
     return pst;
   }
//worker function
//in parentheses have an int, name of struct, and pointer to struct
void semDirect(int i, semstuff_t * sem3stff)
{
//   char *c;
     sem_t *semNE = &(sem3stff->ne); //The sem here is pointing to the struct for northeast
     sem_t *semNW = &(sem3stff->nw); //The sem here is pointing to the struct for northwest
     sem_t *semSE = &(sem3stff->se); //The sem here is pointing to the struct for southeast
     sem_t *semSW = &(sem3stff->sw); //The sem here is pointing to the struct for southwest
     sem_t *semCout = &(sem3stff->Cout); //The sem here is pointing to the struct for Cout

///******************************************
//  Here is where the streets are. What lane the
//  car 1 is going. The purpose here is to see
//  which car gets locked, unlocked, and released
//  in the lane the car 1 is in.
//******************************************
         if(i == 0) //first street /child
          {
             Sem_wait(semCout);
             cout << "Starting process for street 0: Lincoln Hwy West\n" << endl;
             Sem_post(semCout);
             for(int e = 1; e < 4; e++) //car 1
               {
		 Sem_wait(semSE); //Wait for car 1
                 Sem_wait(semCout); //Wait for the print
                 cout << "On Lincoln Hwy west, car " << e << " has obtained lock for SE" << endl;
                 Sem_post(semCout); //unlock the print
                 Sem_wait(semSW);
                 Sem_wait(semCout);
                 cout << "On Lincoln Hwy west, car " << e << " has obtained lock for SW" << endl;
                 cout << "On Lincoln Hwy west, both locks have been obtained, car " << e << " is crossing" << endl;
                 Sem_post(semCout);
                 Sem_post(semSE);
                 Sem_wait(semCout);
                 cout << "On Lincoln Hwy west, car " << e << " has been released SE" << endl;
                 Sem_post(semCout);
                 Sem_post(semSW);
                 Sem_wait(semCout);
                 cout << "On Lincoln Hwy west, both locks been released " << endl;
                 Sem_post(semCout);
                } //End of the for loop for car 1
            }
//            return;
///******************************************
//  Here is where the streets are. What lane the
//  car 2 is going. The purpose here is to see
//  which car gets locked, unlocked, and released
//  in the lane the car 2 is in.
//******************************************
            if(i == 1) //The second lane/ second process
            {
             Sem_wait(semCout);
             cout << "Starting process for street 1: Annie Glidden North\n";
             Sem_post(semCout);
              for( int c = 1; c < 4; c++) //car 2
              {
                Sem_wait(semSW);
                Sem_wait(semCout);
                cout << "On Annie Glidden North, car "<< c << " has obtained lock for SW" << endl;
                Sem_post(semCout);
                Sem_wait(semNW);
                Sem_wait(semCout);
                cout << "On Annie Glidden North, car " << c << " has obtained lock for NW" << endl;
                cout << "On Annie Glidden North, both locks have been obtained, car " << c << " is crossing" << endl;
                Sem_post(semCout);
                Sem_post(semSW);
                Sem_wait(semCout);
                cout << "On Annie Glidden North, car "<< c << " has been released lock for SW" << endl;
                Sem_post(semCout);
                Sem_post(semNW);
                Sem_wait(semCout);
                cout << "On Annie Glidden North, both locks been released "<< endl;
                Sem_post(semCout);
               }
             } //end of 2nd process
 //            return;
///******************************************
//  Here is where the streets are. What lane the
//  car 3 is going. The purpose here is to see
//  which car gets locked, unlocked, and released
//  in the lane the car 3 is in.
//******************************************
            if(i == 2)
             {
               Sem_wait(semCout);
               cout << "\nStarting process for street 2: Lincoln Hwy East\n" << endl;
               Sem_post(semCout);
               for (int j = 1; j < 4; j++) //car 3
               {
                Sem_wait(semSW);
                Sem_wait(semCout);
                cout << "On Lincoln Hwy East, car "<< j << " has obtained lock for SW" << endl;
                Sem_post(semCout);
                Sem_wait(semNE);
                Sem_wait(semCout);
                cout << "On Lincoln Hwy East, car "<< j << " has obtained lock for NE" << endl;
                cout << "On Lincoln Hwy East, both locks have been obtained, car " << j << " is crossing" << endl;
                Sem_post(semCout);
                Sem_post(semSW);
                Sem_wait(semCout);
                cout << "On Lincoln Hwy East, car "<< j << " has been released lock for SW" << endl;
                Sem_post(semCout);
                Sem_post(semNE);
                Sem_wait(semCout);
                 cout << "On Lincoln Hwy East, both locks been released "<< endl;
                 Sem_post(semCout);
                }

              }//end of third process
///******************************************
//  Here is where the streets are. What lane the
//  car is going. The purpose here is to see
//  which car gets locked, unlocked, and released
//  in the lane the car is in.
//******************************************
            if(i == 3)
              {
               Sem_wait(semCout);
               cout << "\nStarting process for street 3: Annie Glidden South\n" << endl;
               Sem_post(semCout);
              for( int r = 1; r < 4; r++)
                {
                 Sem_wait(semSE);
                 Sem_wait(semCout);
                 cout << "On Annie Glidden South, car "<< r << " has obtained lock for SE" << endl;
                 Sem_post(semCout);
                 Sem_wait(semNE);
                 Sem_wait(semCout);
                 cout << "On Annie Glidden South, car "<< r << " has obtained lock for NE" << endl;
                 cout << "On Annie Glidden South, both locks have been obtained, car " << r << " is crossing" << endl;
                 Sem_post(semCout);
                 Sem_post(semSE);
                 Sem_wait(semCout);
                 cout << "On Annie Glidden South, car " << r << " has been released locked for SE" << endl;
                 Sem_post(semCout);
                 Sem_post(semNE);
                 Sem_wait(semCout);
                 cout << "On Annie Glidden South, both locks been released " << endl;
                 Sem_post(semCout);
                 }
              }//end of child3

               exit(0);



} //end of the worker

