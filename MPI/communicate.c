#include <mpi.h>
#include <stdio.h>
#include <string.h>

void easy_communicate_bcast(int my_rank)
{
    int data;
    if(my_rank == 0)
        data = 114514;
    // all process must take part in the bcast. not only the sender
    MPI_Bcast(&data, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (my_rank){
        printf("process [%d]: %d received through broadcast\n", my_rank, data);
    }
}

void easy_communicate_send_recv(int my_rank, int comm_sz)
{
    int data;
    if(my_rank == 0){
        data = 1919810;
        for(int q = 1; q < comm_sz; q++){
            MPI_Send(&data, 1, MPI_INT, q, 0, MPI_COMM_WORLD);
        }
    }
    else{
        MPI_Status status;
        MPI_Recv(&data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        printf("process [%d]: %d received from = %d\n", my_rank, data, status.MPI_SOURCE);
    }
}

void array_sum_easy(int a[], int n, int my_rank, int comm_sz){
    int n_elem_per_process = n / comm_sz;
    int local_sum = 0;
    MPI_Status status;
    if (my_rank == 0) {
        // deliver tasks
        // the last chunk is left to master process
        for (int q = 1; q < comm_sz; q++ ) {
            int *start = a + (q-1) * n_elem_per_process;
            MPI_Send(start, n_elem_per_process, MPI_INT, q, 0, MPI_COMM_WORLD);
        }
        // compute master process's own sum
        int reminder = n % n_elem_per_process;
        int elems = n_elem_per_process + reminder;
        for(int i = (comm_sz-1)*n_elem_per_process; i < n; i++)
            local_sum += a[i];
        int sum_vec[comm_sz];
        // collect all the local sums
        memset(sum_vec, 0, comm_sz * sizeof(int));
        for(int q = 1; q < comm_sz; q++)
            MPI_Recv(sum_vec + q-1, 1, MPI_INT, q, 0, MPI_COMM_WORLD, &status);
        // and add together
        int sum = local_sum;
        for(int i = 0; i < comm_sz; i++)
            sum += sum_vec[i];
        
        printf("total sum = %d\n", sum);
    }
    else {
        // free(): corrupted unsorted chunks
        //int *local_buffer = (int *)malloc(n_elem_per_process);
        int local_buffer[n_elem_per_process];
        MPI_Recv(local_buffer, n_elem_per_process, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        for(int i = 0; i < n_elem_per_process; i++)
            local_sum += local_buffer[i];
        
        MPI_Send(&local_sum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    printf("sum from %d: %d\n", my_rank, local_sum);
}

int main(int argc, char **argv)
{
    MPI_Init(0, 0);
    int my_rank, comm_sz;
    int data;

	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

//    easy_communicate_bcast(my_rank);
//    easy_communicate_send_recv(my_rank, comm_sz);

    int a[1041] = {[0 ... 1040] = 1};
    array_sum_easy(a, 1041, my_rank, comm_sz);

    MPI_Finalize();
}