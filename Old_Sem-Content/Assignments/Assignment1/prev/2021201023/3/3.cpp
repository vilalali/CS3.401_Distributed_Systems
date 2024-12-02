
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include<bits/stdc++.h>
using namespace std;

struct node{
    int value;
    int freq;
};

struct cost_matrix_node{
    int cost;
    int root;
};

struct myComparator
{
    inline bool operator() (const struct node& p1, const struct node& p2)
    {
        return p1.value<p2.value;
    }
};


class Compare
{
public:
    bool operator() (pair<struct node,int> &p1, pair<struct node,int> &p2)
    {
        return p1.first.value > p2.first.value;
    }
};

void findParent(int &N, int p, int i, int j, vector<struct cost_matrix_node> &DP, vector<int> &parent)
{
    if(i>j)
        return;
    int c= DP[N*i +j].root;  //root[i][j];
    parent[c] = p+1; 
    findParent(N,c,i,c-1,DP,parent);
    findParent(N,c,c+1,j,DP,parent);
}

int main(int argc, char **argv) {
    // ifstream cin("./i.txt");

    int size, rank;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Datatype MPI_NODE,oldtypes[1];
    MPI_Aint offsets[1], lb, extent;
    int blockcounts[1];
    oldtypes[0] = MPI_INT;
    offsets[0] = 0;
    blockcounts[0] = 2;
    MPI_Type_get_extent(MPI_INT, &lb, &extent);
    // oldtypes[1] = MPI_CHAR;
    // offsets[1] = 2 * extent;
    // blockcounts[1] = 1;
    MPI_Type_create_struct(1, blockcounts, offsets, oldtypes, &MPI_NODE);
    MPI_Type_commit(&MPI_NODE);


    int N,u,v;
    vector<struct node> nodes;
    int size_arr[size];
    int displacement[size];
    double start_time = 0.0;
    if(rank==0){
        // cout<<"Enter the value of N: ";
        cin>>N;
        // cout<<"Enter the confuguration: \n";
        for(int i=0;i<N;i++){
            cin>>u>>v;
            struct node sn;
            sn.value = u;
            sn.freq = v;
            nodes.push_back(sn);
        }
        start_time = MPI_Wtime();
        int avg_size = ceill((double)N*1.0/size);
        
        for(int i=0;i<size;i++){
            int start = max(0,i*avg_size);
            int end = min(N-1,(i+1)*avg_size - 1);
            if(start>end){
                size_arr[i] = 0;
            }
            else{
                size_arr[i] = end - start +1;
            }
            displacement[i] = start;
        }
    }

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(size_arr, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(displacement, size, MPI_INT, 0, MPI_COMM_WORLD);
   
    vector<struct node> local_data(size_arr[rank]);
    int np = size_arr[rank];
    MPI_Scatterv( nodes.data() , size_arr , displacement , MPI_NODE , local_data.data() , np , MPI_NODE , 0 , MPI_COMM_WORLD);
    // each process will execute
    sort(local_data.begin(),local_data.end(),myComparator());
    // till here
    MPI_Gatherv( local_data.data() , local_data.size() , MPI_NODE , nodes.data() , size_arr , displacement , MPI_NODE , 0 , MPI_COMM_WORLD);
    vector<struct node> sorted_nodes;
    if(rank==0){
        unordered_map<int,vector<struct node>> mp;
        for(int i=0;i<size;i++){
            for(int j=displacement[i];j<displacement[i]+size_arr[i];j++){
                mp[i].push_back(nodes[j]);
            }
        }

        priority_queue<pair<struct node,int>,vector<pair<struct node,int>> , Compare> min_heap;
        for(int i=0;i<size;i++){
            if(mp[i].size()>0){
                min_heap.push({mp[i][0],i});
                mp[i].erase(mp[i].begin());
            }
        }

        while(!min_heap.empty()){
            auto p = min_heap.top();
            min_heap.pop();
            sorted_nodes.push_back(p.first);
            if(mp[p.second].size()>0){
                min_heap.push({mp[p.second][0],p.second});
                mp[p.second].erase(mp[p.second].begin());
            }
        }
        // cout<<"Value: "<<"Frequency\n";
        // for(int i=0;i<sorted_nodes.size();i++){
        //     printf("%d  %d\n",sorted_nodes[i].value,sorted_nodes[i].freq);
        // }

    }

    // Merge sort is done , sorted nodes are present at sorted_nodes

    /*
        Now lets do the main work
    */

   
   vector<struct cost_matrix_node> DP(N*N);
   if(rank==0){

        for(int i=0;i<N;i++){
            struct cost_matrix_node cn;
            cn.cost = sorted_nodes[i].freq;
            cn.root = i;
            DP[i*N+i] = cn;
        }
        // printf("rank= %d\n",rank);
        // for(int i=0;i<N;i++){
        //     for(int j=0;j<N;j++){
        //         printf("%d\t",DP[i*N + j].cost);
        //     }
        //     printf("\n");
        // }
   }
   if(size>1){
        for(int itr=1;itr<N;itr++){

            int nps = N-itr;
            int avg = ceill(double((nps)*1.0/size));
            if(rank==0){
                // printf("iteartion = %d\n",itr);
                for(int ran=0;ran<size;ran++){
                    int start = max(0,ran*avg);
                    int end = min(nps-1,(ran+1)*avg-1);
                    int each = end-start+1;
                    MPI_Send(&each,1,MPI_INT,ran,0,MPI_COMM_WORLD);
                    if(each>0){
                        int array[each];
                        for(int k=0;k<each;k++){
                            array[k] = itr + (ran*avg*(N+1)) + (N+1)*k;
                            // printf("rank = %d, val = %d \n",ran,array[k]);
                        }
                        MPI_Send(array,each,MPI_INT,ran,0,MPI_COMM_WORLD);
                        MPI_Send(DP.data(),N*N,MPI_NODE,ran,0,MPI_COMM_WORLD);

                    }
                }
            }

            int each = 0;
            MPI_Recv(&each,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Send(&each,1,MPI_INT,0,0,MPI_COMM_WORLD);
            if(each>0){
                vector<struct cost_matrix_node> local_dp(N*N);
                int array[each];
                MPI_Recv(array,each,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                MPI_Recv(local_dp.data(),N*N,MPI_NODE,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                struct cost_matrix_node cost_estimate[each];
                for(int cell=0;cell<each;cell++){
                    int i = array[cell]/N;
                    int j = array[cell]%N;
                    // optimal cost code

                    int cost_val = INT_MAX;
                    int root = 0;
                    int off_set_sum = 0;
                    for(int k=i;k<=j;k++){
                        off_set_sum += local_dp[k*N + k].cost;
                    }

                    // Try making all keys in interval keys[i..j] as root
                    for (int r = i; r <= j; r++)
                    {
                        int c = ((r > i)? local_dp[i*N + r-1].cost : 0) + ((r < j)? local_dp[N*(r+1) + j].cost : 0) + off_set_sum;
                        if (c < cost_val){
                            cost_val = c;
                            root = r;
                        }
                    }

                    struct cost_matrix_node nd;
                    nd.cost = cost_val;
                    nd.root = root;
                    cost_estimate[cell] = nd;
                }
                
                MPI_Send(cost_estimate,each,MPI_NODE,0,0,MPI_COMM_WORLD);
                MPI_Send(array,each,MPI_INT,0,0,MPI_COMM_WORLD);
            }

            // printf("itr= %d, rank= %d, each= %d\n",itr,rank,each);
            MPI_Barrier(MPI_COMM_WORLD);

            if(rank==0){
                for(int ran=0;ran<size;ran++){
                    int each=0;
                    MPI_Recv(&each,1,MPI_INT,ran,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                    if(each>0){
                        struct cost_matrix_node cost_estimate[each];
                        MPI_Recv(cost_estimate,each,MPI_NODE,ran,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                        int array[each];
                        MPI_Recv(array,each,MPI_INT,ran,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                        // printf("Accumulation: itr= %d, rank= %d, each= %d\n",itr,ran,each);
                        for(int i=0;i<each;i++){
                            DP[array[i]] = cost_estimate[i];
                        }
                    }

                }
                // printf("Update DP for iteartion = %d\n",itr);
                // for(int i=0;i<N;i++){
                //     for(int j=0;j<N;j++){
                //         printf("(%d, %d)\t",DP[i*N + j].cost, DP[i*N+j].root);
                //     }
                //     printf("\n");
                // }
            }
        }
    }
    else{
        for(int itr = 1;itr<N;itr++){
            int nps = N-itr;
            int array[nps];
            for(int k=0;k<nps;k++){
                array[k] = itr + (N+1)*k;
                // printf("rank = %d, val = %d \n",ran,array[k]);
            }

            for(int cell=0;cell<nps;cell++){
                int i = array[cell]/N;
                int j = array[cell]%N;
                // optimal cost code

                int cost_val = INT_MAX;
                int root = 0;
                int off_set_sum = 0;
                for(int k=i;k<=j;k++){
                    off_set_sum += DP[k*N + k].cost;
                }

                // Try making all keys in interval keys[i..j] as root
                for (int r = i; r <= j; r++)
                {
                    int c = ((r > i)? DP[i*N + r-1].cost : 0) + ((r < j)? DP[N*(r+1) + j].cost : 0) + off_set_sum;
                    if (c < cost_val){
                        cost_val = c;
                        root = r;
                    }
                }

                struct cost_matrix_node nd;
                nd.cost = cost_val;
                nd.root = root;
                DP[array[cell]]  = nd;
            }
        }
    }
    if(rank==0){
        printf("%d\n",DP[N-1].cost);
        vector<int> parent(N,-1);
        findParent(N,-1,0,N-1,DP,parent);
        // printf("Parent Node from Node 1 to N: \n");
        for(int i=0;i<N;i++){
            printf("%d ",parent[i]);
        }
        cout<<endl;
        double time = MPI_Wtime() - start_time;
        // cout<<time<<endl;
    }

    MPI_Type_free(&MPI_NODE);
    MPI_Finalize();
    return 0;
}

