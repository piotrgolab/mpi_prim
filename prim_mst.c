#include <stdio.h>
#include <mpi.h>




int** alloc_2d_int(int rows, int cols) {
    int *data = (int *)malloc(rows*cols*sizeof(int));
    int **array= (int **)malloc(rows*sizeof(int*));
    for (int i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}



int main(int argc, char *argv[])
{


    int INF = 999;
    int n = 6; //liczba wierzcholkow
    int p = 3; //liczba procesów
    //Let p be the number of processes, and let n be the number of vertices in the graph.
    //The set V is partitioned into p subsets using the 1-D block mapping

    //Reprezentacja grafu przez macierz sasiedztwa, graf z ksiazki
    int V[6][6] = { {0, 1, 3, INF, INF, 3}, {1, 0, 5, 1, INF, INF}, {3, 5, 0, 2, 1, INF}, {INF, 1, 2, 0, 4, INF}, {INF, INF, 1, 4, 0, 5}, {2, INF, INF, INF, 5, 0} };

    //Zalozylem, ze skoro mamy 6 rzedow i kolumn, a dzielic bede na grupy rzedow,
    //to odpowiednia liczba procesow dla tak malego grafu wyniesie 3 (po 2 rzedy na proces)
    //
    //liczba procesow



    //Each subset has n/p consecutive vertices
    //
    //
    //(3.41)
    //The kth part contains rows : kn/p...(k + 1)n/p
    //where 0 <= k < p
    //(k: 0...p-1)

    //(10.2)
    // Let V_i be the subset of vertices assigned to process P_i for i = 0, 1, ..., p - 1.
    //Each process Pi stores the part of the array d that corresponds to V_i (that is, process P_i stores d [v] such that v  V_i)

    //Macierz macierzy z czesciami
    int V_i[p][n / p][n];
    //
    //Dzielenie rzedami, po 2 rzedy na 1 czesc
    //
    //
    //
    //
    //for each part
    for(int k = 0; k < p ; k ++)
    {
        //for each row in part
        for(int row = 0; row < n / p ; row ++)
        {
            //for each element in row
            for(int elem = 0; elem < n ; elem++)
            {
                V_i[k][row][elem] = V[k * n / p + row][elem];
            }
        }
    }
    //
    //Wypisywanie czesci
    //
    //
    //
    //
    //for each part
    // {
    //     for(int k = 0; k < p ; k ++)
    //     {
    //         //for each row in part
    //         printf("Part %d:\n", k + 1);
    //         for(int row = 0; row < n / p ; row ++)
    //         {
    //             //for each element in row
    //             for(int elem = 0; elem < n ; elem++)
    //             {
    //                 printf("%d ", V_i[k][row][elem]);
    //             }
    //             printf("\n");
    //         }
    //     }
    // }





    int numberOfProcesses = p;
    int dimOfMatrix;    //dimension of divided matrix
    int V_iLocal[n][n]; //local storage for V_i
    int minLocal;       //minium weight localy
    int minGlobal;      //minium weight globaly
    int visited[n]; //0 not visited, 1 visited
    int key[n];
    int prod;           //start of columns containing in local process
    int sup;            //end of columns containing in local process
    int strip;
    int temp;           //will be used to help reduce edge
    int indexMinLocal;  //index of matrix in local process
    int indexMinGlobal; //index of matrix in global process
    int costLocal;      //cost of adding edge in local process
    int costGlobal;     //cost of MST

    int previewOfMST[n];//holds number of vertices added to MST



    int rank;
    MPI_Status status;      //status of recv
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0)
    {
        printf("Jestem procesem 0\n" );
    }

    //BCast value of n to other processes
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    dimOfMatrix = n / numberOfProcesses;    // !!! doesnt check if there is more processes than vertices -- now only works with n%p==0!!!

    int **m;
    /*...*/
    //TODO pierwszy zmieniec na n/p
    m = alloc_2d_int(dimOfMatrix,n);

    if(rank == 0)
    {
        //TODO implement sending parts of V to other processes

        //prawdopodobnie nalezy póścić to w forze i w pierwszym polu wpisywać początki tablicy a w dest numer iteracji
        for (int j = 1; j < numberOfProcesses; j++)
        {
            int V_temp[n / p][n];
            for(int a = 0 ; a < (n/p) ; a ++)
            {
                for (int b = 0 ; b < n ; b ++)
                {
                    V_temp[a][b] = V_i[j - 1][a][b];
                }   
            }       
            MPI_Send(&(V_temp[0][0])/*buff initial adres &(V_i[j - 1][n][n])*/, n*(n/p)/*dimOfMatrix*/, MPI_INT, /*dest*/ j, 1, MPI_COMM_WORLD);
        }
    }
    else
    {
        //TODO implemet receving V_i from process 0 and saveing to V_iLocal
        //

        MPI_Recv(/*local Vi*/&(m[0][0]), n*(n/p)/*dimOfMatrix*/, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    //initial key values for algorithm

    if(rank != 0)
    {
        for(int i = 0; i < dimOfMatrix; i++)
            key[i] = INF;
    }
    else
    {
        for(int i = 1; i < dimOfMatrix; i++)
            key[i] = INF;
        key[0] = 0;
    }

    if(rank != numberOfProcesses - 1)
    {
        prod = rank * dimOfMatrix;
        sup = prod + dimOfMatrix;
    }
    else
    {
        prod = rank * strip;
        sup = n;
    }
    MPI_Barrier(MPI_COMM_WORLD);

    for(;;)
    {
        minLocal = INF;
        for(int i = 0; i < dimOfMatrix; i++)
        {
            //Each process find minimal local key
            if( visited[i] == 0 && key[i] < minLocal)
            {
                minLocal = key[i];
                indexMinLocal = i;
            }
        }

        //Determine gobal minimum of weight
        MPI_Allreduce(&minLocal, &minGlobal, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

        if(minGlobal == INF) break;

        if(minLocal == minGlobal)
        {
            temp = indexMinLocal + prod;
            MPI_Allreduce(&temp, &indexMinGlobal, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
        }
        else
            MPI_Allreduce(&n, &indexMinGlobal, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);


        //If newest node added to MST is from this local process mark it as visited

        if(indexMinGlobal >= prod && indexMinGlobal < sup)
        {
            visited[indexMinGlobal - prod] = 1;
            costLocal += minLocal;
        }

        //TODO add putting value to the key and index of added edge
        for(int i = 0; i < dimOfMatrix; i++)
        {
            if(m[indexMinGlobal][i] < key[i] && visited[i] == 0)
            {
                //TODO zamiana indeksów
                key[i] = m[indexMinGlobal][i];
                previewOfMST[i] = indexMinGlobal;
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Reduce(&costLocal, &costGlobal, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    if(rank == 0)
    {
        printf("Waga drzewa MST: %d\n", costGlobal);
    }


    //(10.2)
    //Each process Pi computes di[u] = min{di[v]|v  (V - VT)  Vi} during each iteration of the while loop.
    //The global minimum is then obtained over all di[u] by using the all-to-one reduction operation (Section 4.1) and is stored in process P0.
    //Process P0 now holds the new vertex u, which will be inserted into VT.
    //Process P0 broadcasts u to all processes by using one-to-all broadcast (Section 4.1).
    //The process Pi responsible for vertex u marks u as belonging to set VT.
    //Finally, each process updates the values of d[v] for its local vertices.
    //
    //
    //Pseudokod z ksiazki (Algorithm 10.1)
    //
    /*
    1.   procedure PRIM_MST(V, E, w, r)
    2.   begin
    3.      VT := {r};
    4.      d[r] := 0;
    5.      for all v  (V - VT ) do
    6.         if edge (r, v) exists set d[v] := w(r, v);
    7.         else set d[v] := ;
    8.      while VT  V do
    9.      begin
    10.        find a vertex u such that d[u] :=min{d[v]|v  (V - VT )};
    11.        VT := VT  {u};
    12.        for all v  (V - VT ) do
    13.            d[v] := min{d[v], w(u, v)};
    14.     endwhile
    15.  end PRIM_MST
    */

    MPI_Finalize();
    return 0;
}