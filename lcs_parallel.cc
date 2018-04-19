// Noah Jacobson
// This is an OpenMPI program that determines the longest common sequence
// of 2 given strings from the user. It accomplishes the task by pipelinng information
// across the processes. 

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>
#include <string>
#include <mpi.h>

using namespace std;

// Function given by Dr Juedes of Ohio University
// It returns the max of the 3 parameters passed
int max3(int x,int y, int z) {
  return max(max(x, y), z);
}

int main()
{
  string x1;        // string x
  string y1;        // string y
  int x_length;     // length of string x
  int y_length;     // length of string y

  int n;            // size of all processors
  int p_id;         // process number
  int div;          // breaking up the code evenly

  // determining what process number and how many process are availble to spread out the work
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &n);
  MPI_Comm_rank(MPI_COMM_WORLD, &p_id);

  // mom processor that spreads out all the info to the child processes
  // before it starts crunching numbers
  if(p_id == 0) {

    // getting information from user
    getline(cin,x1);
    getline(cin,y1);

    x_length = x1.length(); 
    y_length = y1.length();

    div = x_length / n; // the size of the chunk

    // sending the strings and div to all processes
    for(int x = 1; x < n; x++) {
      MPI_Send(&x_length, 1, MPI_INT, x, 0, MPI_COMM_WORLD);
      MPI_Send(&y_length, 1, MPI_INT, x, 0, MPI_COMM_WORLD);
      MPI_Send(x1.c_str(), x_length, MPI_CHAR, x, 0, MPI_COMM_WORLD);
      MPI_Send(y1.c_str(), y_length, MPI_CHAR, x, 0, MPI_COMM_WORLD);
      MPI_Send(&div, 1, MPI_INT, x, 0, MPI_COMM_WORLD);
    }

    int LCS[y_length+1][div+1]; // creating 2 d array

    LCS[0][0] = 0;

    for(int i = 0; i <= div; i++) { // column
      LCS[0][i] = 0;
    }
    for(int i = 0; i <= y_length; i++) { // rows 
      LCS[i][0] = 0;
    }

    for(int j = 1; j <= y_length; j++) {
      for(int i = 1; i <= div; i++) {

        // determining the number
        if(x1[i-1] == y1[j-1]) {
          LCS[j][i] = max3(LCS[j-1][i-1]+1, LCS[j-1][i], LCS[j][i-1]);
        }
        else {
          LCS[j][i] = max(LCS[j-1][i],LCS[j][i-1]);
        }
        // sending padding for next array over
        if(i == div) {
          MPI_Send(&LCS[j][i], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        }
      }
    }
  }
  else
  {
    // recieving information
    // since MPI can't send strings the strings are recieved as c_stringss
    // and then converted back to strings
    char *xstr;
    char *ystr;
    MPI_Recv(&x_length, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);
    MPI_Recv(&y_length, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);
    xstr = new char[x_length];
    ystr = new char[y_length];
    MPI_Recv(xstr, x_length, MPI_CHAR, 0, 0, MPI_COMM_WORLD, NULL);
    MPI_Recv(ystr, y_length, MPI_CHAR, 0, 0, MPI_COMM_WORLD, NULL);
    MPI_Recv(&div, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);

    x1 = xstr;
    y1 = ystr;

    if(p_id == n-1) { // last process

      int LCS[y_length+1][div+1];

      LCS[0][0] = 0;

      for(int i = 0; i <= div; i++) { // column
        LCS[0][i] = 0;
      }

      // doing work on the 2d array
      for(int j = 0; j <= y_length; j++) {
        for(int i = 0; i <= div; i++) {

          if(j == 0) {
            LCS[j][i] = 0;
          }
          // padding
          else if(i == 0) {
            MPI_Recv(&LCS[j][0], 1, MPI_INT, p_id-1, 0, MPI_COMM_WORLD, NULL);
          }
          //determining value
          else {
            if(x1[i-1] == y1[j-1]) {
              LCS[j][i] = max3(LCS[j-1][i-1]+1, LCS[j-1][i], LCS[j][i-1]);
            }
            else {
              LCS[j][i] = max(LCS[j-1][i],LCS[j][i-1]);
            }
          }
        }
      }

      cout << "LCS Length: " << LCS[y_length][div] << endl;
    }
    else {
      int LCS[y_length+1][div+1];

      LCS[0][0] = 0;

      // padding
      for(int i = 0; i <= div; i++) { // column
        LCS[0][i] = 0;
      }

      // doing work on tthe 2d array
      for(int j = 1; j <= y_length; j++) {
        for(int i = 0; i <= div; i++) {
          // padding
          if(i == 0) {
            MPI_Recv(&LCS[j][i], 1, MPI_INT, p_id-1, 0, MPI_COMM_WORLD, NULL);
          } 
          else {
            if(x1[i-1] == y1[j-1]) {
              LCS[j][i] = max3(LCS[j-1][i-1]+1, LCS[j-1][i], LCS[j][i-1]);
            }
            else {
              LCS[j][i] = max(LCS[j-1][i],LCS[j][i-1]);
            }
          }
          // sending padding for next array over
          if(i == div) {
            MPI_Send(&LCS[j][i], 1, MPI_INT, p_id+1, 0, MPI_COMM_WORLD);
          }
        }
      }
    }
  }
  
  MPI_Finalize();
  return 0;
}