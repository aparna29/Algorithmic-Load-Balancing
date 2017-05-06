#include<iostream>
#include<algorithm>
#include<math.h>
#include<cstdlib>
#include<boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/status.hpp>
#define ELECTED_TAG 2


using namespace boost::mpi;
using namespace std;

environment env;
communicator world;
float rates[4] = {1599.992, 1600.125,1599.992,1600.125};
float bids[4],total_arr_rate,load[4],cost[4],total_cost,profit[4],pay[4],total_profit=0.0;
int slave_bids;
/*rates[0] = 1599.992;
rates[1] = 1600.125;
rates[2] = 1599.992;
rates[3] = 1600.125;
*/
int nprocs;
void printCost()
{
	int i;
	/*for(i=0;i<slave_bids;i++)
	{
		printf("Cost of processor %d = %f\n",i,cost[i]);
	}*/
	printf("\nTotal cost = %f",total_cost);	
}

void printProfit()
{
	int i;
	for(i=0;i<slave_bids;i++)
	{
		total_profit+=profit[i];
		printf("Profit of processor %d = %f\n",i,profit[i]);
	}
	printf("\nFaulty: total profit = %f",total_profit);
	
}
void calculate_payment()
{
	int i;
	for(i=0;i<slave_bids;i++)
	{
		pay[i] = (bids[i]+1)*load[i];
		profit[i] = pay[i] - cost[i];
		//printf("load = %f, rate = %f\n",load[i],rates[i+1]);
	}
	printProfit();
}

void calculate_cost()
{
	int i;
	for(i=0;i<slave_bids;i++)
	{
		cost[i] = load[i]/rates[i+1];
		total_cost+=cost[i];
		//printf("load = %f, rate = %f\n",load[i],rates[i+1]);
	}
	calculate_payment();
}


void master()
{
	int i;
	sort(bids,bids+3);
	/*for(i=0;i<slave_bids;i++)
		printf("%f\n",bids[i]);*/
	float c,bids_total = 0.0,sqrt_bids=0.0;
	for(i=0;i<slave_bids;i++)
	{
		bids_total += 1.0/bids[i];
		sqrt_bids += sqrt(1.0/bids[i]);
	}
	c = (bids_total - total_arr_rate)/(sqrt_bids);
	int n = slave_bids;
	while(c > sqrt(1.0/bids[slave_bids-1]))
	{
		load[n]=0.0;
		n--;
		for(i=0;i<n;i++)
		{
			bids_total += 1.0/bids[i];
			sqrt_bids += sqrt(1.0/bids[i]);
		}
		c = (bids_total - total_arr_rate)/(sqrt_bids);
	}
	for(i=0;i<slave_bids;i++)
	{
		load[i] = 1.0/bids[i] - c * sqrt(1.0/bids[i]);
		//printf("Load %d = %f\n",i,load[i]);
		MPI_Send(&load[i],1,MPI_FLOAT,i+1,0,world);
	}
	calculate_cost();
	printCost();
}

int main(int argc, char const *argv[])
{
	int no_of_jobs=0,world_rank;
	
	MPI_Comm_size (MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	if (nprocs < 2) {
        	printf ("Number of processors < 2\nExiting\n");
        	exit (1);
    	}
    	slave_bids = nprocs-1;
    	// Leader election
	bool terminated=false;
	int inmsg[2];	//incoming msg
	int msg[2];
	msg[0]=world_rank;
	msg[1]=0;
	MPI_Status Stat;
	int min=world_rank; 	//initially the minimum ID is node's own ID
	int lcounter=0;	//local counter of received messages (for protocol)
	int lmc=0;		//local counter of send and received messages (for analysis)
	int dummy=1;
	int leader;
	//send my ID with counter to the right
	MPI_Send(&msg, 2, MPI_INT, (world_rank+1)%nprocs, 1, MPI_COMM_WORLD);
	lmc++; 		//first message sent	

	while (!terminated){
		//receive ID with counter from left
		MPI_Recv(&inmsg, 2, MPI_INT, (world_rank-1)%nprocs, 1, MPI_COMM_WORLD, &Stat);
		lmc++;
		lcounter++;	//increment local counter
		if (min>inmsg[0]) min=inmsg[0];	//update minimum value if needed
		inmsg[1]++;	//increment the counter before forwarding

		if (inmsg[0]==world_rank && lcounter==(inmsg[1])) {
			terminated=true;
		}
	
		//forward received ID
		MPI_Send(&inmsg, 2, MPI_INT, (world_rank+1)%nprocs, 1, MPI_COMM_WORLD);		
	}
	leader = min;
	//printf("\nLeader node %d",leader);
	FILE *fp;
    	fp = fopen("cmd_lines","r");
    	if(fp == NULL)
    	{
    		printf("\nCannot open file");
    		return 0;
    	}
    	char c;
    	for(c = getc(fp); c!=EOF ; c=getc(fp))
    	{
    		if(c=='\n')
    			no_of_jobs++;
    	}
    	total_arr_rate = no_of_jobs;
  //  	printf("%d\n",no_of_jobs);
    	fclose(fp);
    	if(world_rank!=leader)
	{    	
		float proc_load;
		MPI_Send(&rates[world_rank],1,MPI_FLOAT,leader,0,world);
		MPI_Recv(&proc_load,1,MPI_FLOAT,leader,0,world,MPI_STATUS_IGNORE);
		printf("Load received by  processor %d = %f\n",world_rank,proc_load);
		load[world_rank-1]=proc_load;
	}
	else if (world_rank == leader)
	{
		float num;
		int rank;
		for (rank = 1; rank < nprocs; ++rank) {
		// Receiving the bids from slaves
			MPI_Recv(&num, 1, MPI_FLOAT, MPI_ANY_SOURCE,0, world, MPI_STATUS_IGNORE);
			bids[rank-1]=4.0/num;
		//	printf("Num %f Rank %d \n",num,rank);
		}
		master();
	}
}
