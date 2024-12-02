#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include<bits/stdc++.h>
using namespace std;

struct particle{
    int r;
    int c;
    char dir;
};

struct particle move(int r, int c, int n, int m, char dir){
    int row=r, col=c;
    char direction = dir;
    if(r==0 and c==0){
        // cout<<"C1"<<endl;
        if(dir=='L' or dir=='R'){
            row = r+1;
            col = c;
            direction = 'R';
        }
        else if(dir=='U' or dir=='D'){
            row = r;
            col = c+1;
            direction = 'U';
        }
    }
    else if(r==n-1 and c==m-1){
        // cout<<"C2"<<endl;
        if(dir=='L' or dir=='R'){
            row = r-1;
            col = c;
            direction = 'L';
        }
        else if(dir=='U' or dir=='D'){
            row = r;
            col = c-1;
            direction = 'D';
        }
    }
    else if(r==n-1 and c==0){
        // cout<<"C3"<<endl;
        if(dir=='L' or dir=='R'){
            row = r-1;
            col = c;
            direction = 'L';
        }
        else if(dir=='U' or dir=='D'){
            row = r;
            col = c+1;
            direction = 'U';
        }
    }
    else if(r==0 and c==m-1){
        // cout<<"C4"<<endl;
        if(dir=='L' or dir=='R'){
            row = r+1;
            col = c;
            direction = 'R';
        }
        else if(dir=='U' or dir=='D'){
            row = r;
            col = c-1;
            direction = 'D';
        }
    }
    else if(c==m-1){
        // cout<<"C5"<<endl;
        if(dir=='L' or dir=='R'){
            row = r+(dir=='L'?-1:1);
            col = c;
            direction = dir;
        }
        else if(dir=='U' or dir=='D'){
            row = r;
            col = c-1;
            direction = 'D';
        }
    }
    else if(r==n-1){
        // cout<<"C6"<<endl;
        if(dir=='L' or dir=='R'){
            row = r-1;
            col = c;
            direction = 'L';
        }
        else if(dir=='U' or dir=='D'){
            row = r ;
            col = c+ (dir=='U'?1:-1);
            direction = dir;
        }
    }
    else if(c==0){
        // cout<<"C7"<<endl;
        if(dir=='L' or dir=='R'){
            row = r+(dir=='L'?-1:1);
            col = c;
            direction = dir;
        }
        else if(dir=='U' or dir=='D'){
            row = r;
            col = c+1;
            direction = 'U';
        }
    }
    else if(r==0){
        // cout<<"C8"<<endl;
        if(dir=='L' or dir=='R'){
            row = r+1;
            col = c;
            direction = 'R';
        }
        else if(dir=='U' or dir=='D'){
            row = r ;
            col = c + (dir=='U'?1:-1);
            direction = dir;
        }
    }
    else{
        // cout<<"C9"<<endl;
        if(dir=='L' or dir=='R'){
            row = r+(dir=='L'?-1:1);
            col = c;
            direction = dir;
        }
        else if(dir=='U' or dir=='D'){
            row = r ;
            col = c + (dir=='U'?1:-1);
            direction = dir;
        }
    }
    
    // cout<<r<<" "<<c<<" "<<dir<<"--->"<<row<<" "<<col<<" "<<direction<<endl;
    // printf("%d  %d  %c --->  %d  %d  %c\n",r,c,dir,row,col,direction);
    struct particle p;
    p.r = row;
    p.c = col;
    p.dir = direction;
    return p;
}


struct myComparator
{
    inline bool operator() (const struct particle& p1, const struct particle& p2)
    {
        if(p1.r == p2.r){
            return p1.c < p2.c;
        }
        return p1.r < p2.r;
    }
};

int main(int argc, char **argv) {
    int size, rank;
    // ifstream cin("./i.txt");
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    //MPI datatype
    MPI_Datatype MPI_PARTICLE,oldtypes[2];
    MPI_Aint offsets[2], lb, extent;
    int blockcounts[2];
    oldtypes[0] = MPI_INT;
    offsets[0] = 0;
    blockcounts[0] = 3;
    MPI_Type_get_extent(MPI_INT, &lb, &extent);
    oldtypes[1] = MPI_CHAR;
    offsets[1] = 2 * extent;
    blockcounts[1] = 1;
    MPI_Type_create_struct(1, blockcounts, offsets, oldtypes, &MPI_PARTICLE);
    MPI_Type_commit(&MPI_PARTICLE);
    //MPI datatype
    int N,M,K,T;
    vector<struct particle> pym_particles;
    double starttime = 0.0;
    if(rank==0){
        // cout<<"Enter the value of N,M,K,T with space separated: ";
        cin>>N>>M>>K>>T;
        // cout<<"Enter K particles configuration:\n";
        int r,c;
        char dir;
        for(int i=0;i<K;i++){
            cin>>r>>c>>dir;
            struct particle pym;
            pym.r = r;
            pym.c = c;
            pym.dir = dir;
            pym_particles.push_back(pym); 
        }
        starttime = MPI_Wtime();
    }
    
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&K, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&T, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //bcAST t
    int np = ceill((double)(N*1.0/size));
    
    if(size>1 and rank==0){
        // printf("np= %d\n",np);
        for(int ran=0;ran<size;ran++){
            int start = max(0,ran*np);
            int end = min(N-1,(ran+1)*np-1);
            vector<struct particle> lp;
            for(int i=0;i<K;i++){
                struct particle p = pym_particles[i];
                if(start<= p.r and p.r<=end){
                    lp.push_back(p);
                }
                
            }
            int nps=lp.size();
            // printf("rank= %d, nps= %d, start= %d, end= %d\n",ran,nps,start,end);
           
            MPI_Send(&start,1,MPI_INT,ran,0,MPI_COMM_WORLD);
            
            MPI_Send(&end,1,MPI_INT,ran,0,MPI_COMM_WORLD);
            
            MPI_Send(&nps,1,MPI_INT,ran,0,MPI_COMM_WORLD);
            
            MPI_Send(lp.data(),nps,MPI_PARTICLE,ran,0,MPI_COMM_WORLD);
            // MPI_Recv();

            // printf("rank= %d, nps= %d, start= %d, end= %d\n",ran,nps,start,end);
        }
    }
    int start,end,nps;
    vector<struct particle> local_pym_particles;
    if(size>1){
        MPI_Recv(&start,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Recv(&end,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Recv(&nps,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        local_pym_particles = vector<struct particle>(nps);
        MPI_Recv(local_pym_particles.data(),nps,MPI_PARTICLE,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    }
    else{
        start = 0;
        end = N-1;
        nps = K;
        local_pym_particles = pym_particles;
    }

    unordered_map<int,unordered_map<int,vector<struct particle>>> mp;

    for(int i=0;i<nps;i++){
        struct particle p = local_pym_particles[i];
        mp[p.r][p.c].push_back(p);
    }
    while(T--){
        //movement
        // if(start>end){
        //     continue;
        // }
        vector<struct particle> left,right;
        unordered_map<int,unordered_map<int,vector<struct particle>>>next_mp;
        for(auto pr: mp){
            for(auto pp: pr.second){
                for(struct particle p: pp.second){
                    struct particle ret = move(p.r,p.c,N,M,p.dir);
                    if(ret.r<start){
                        left.push_back(ret);
                    }
                    else if(ret.r>end){
                        right.push_back(ret);
                    }
                    else{
                        next_mp[ret.r][ret.c].push_back(ret);
                    }
                }
            }
        }
        mp = next_mp;
        next_mp.clear();
        
        //send points to other process if any
        int ls = left.size();
        int rs = right.size();
        if(rank != 0){
            MPI_Send(&ls,1,MPI_INT,rank-1,0,MPI_COMM_WORLD);
            if(ls>0){
                MPI_Send(left.data(),ls,MPI_PARTICLE,rank-1,0,MPI_COMM_WORLD);
            }
        }
        if(rank != size-1){
            MPI_Send(&rs,1,MPI_INT,rank+1,0,MPI_COMM_WORLD);
            if(rs>0) {           
                MPI_Send(right.data(),rs,MPI_PARTICLE,rank+1,0,MPI_COMM_WORLD);
            }
        }

        //Recieve points from other process if any
        int lss,rss;
        if(rank != 0){
            MPI_Recv(&lss,1,MPI_INT,rank-1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            vector<struct particle> left_pros(lss);
            if(lss>0){
                MPI_Recv(left_pros.data(),lss,MPI_PARTICLE,rank-1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                for(struct particle p: left_pros){
                    mp[p.r][p.c].push_back(p);
                }
            }
            //store the points to the local map
            
        }
        if(rank != size-1){
            MPI_Recv(&rss,1,MPI_INT,rank+1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            vector<struct particle> right_pros(rss);
            if(rss>0){
                MPI_Recv(right_pros.data(),rss,MPI_PARTICLE,rank+1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                    for(struct particle p: right_pros){
                    mp[p.r][p.c].push_back(p);
                }
            }
            //store the points to the local map
            
        }
        
        // collision detection just change the direction
        for(auto &pr: mp){
            for(auto &pp: pr.second){
                if(pp.second.size()==2){
                    struct particle &p1 = pp.second[0];
                    struct particle &p2 = pp.second[1];
                    if(p1.dir == 'L' and p2.dir == 'R'){
                        p1.dir = 'U';
                        p2.dir = 'D';
                    }
                    else if(p1.dir == 'R' and p2.dir == 'L'){
                        p1.dir = 'D';
                        p2.dir = 'U';
                    }
                    else if(p1.dir == 'U' and p2.dir == 'D'){
                        p1.dir = 'R';
                        p2.dir = 'L';
                    }
                    else if(p1.dir == 'D' and p2.dir == 'U'){
                        p1.dir = 'L';
                        p2.dir = 'R';
                    }
                }
            }
        }

        // printf("Iteartion = %d for rank = %d \n",T,rank);
        

    }


    nps=0;
    local_pym_particles.clear();
    for(auto pr: mp){
        for(auto pp: pr.second){
            nps += pp.second.size();
            for (struct particle p: pp.second){
                local_pym_particles.push_back(p);
            }
        }
    }
    // store the result into the new local_pym_particles and send to the parent
    if(size>1){
        MPI_Send(&nps,1,MPI_INT,0,0,MPI_COMM_WORLD);
        if(nps>0)
            MPI_Send(local_pym_particles.data(),nps,MPI_PARTICLE,0,0,MPI_COMM_WORLD);
    }
    // MPI_Barrier(MPI_COMM_WORLD);
    if(rank==0){ // accumulate the result here
        // printf("ACcumulation of all the process  at rank = %d \n",rank);
        vector<struct particle> result_pym_particles;
        if(size>1){
            for(int ran=0;ran<size;ran++){
                int nps;
                MPI_Recv(&nps,1,MPI_INT,ran,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                vector<struct particle> local_pym_particles(nps);
                if(nps>0){
                    MPI_Recv(local_pym_particles.data(),nps,MPI_PARTICLE,ran,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                    for(int i=0;i<nps;i++){
                        struct particle p = local_pym_particles[i];
                        result_pym_particles.push_back(p);
                        
                    }
                }
            }
        }
        else{
            result_pym_particles = local_pym_particles;
        }
        sort(result_pym_particles.begin(),result_pym_particles.end(),myComparator());
        // cout<<"************** OUTPUT *************************"<<endl;
        for(struct particle p: result_pym_particles){
            // cout<<p.r<<" "<<p.c<<" "<<p.dir<<endl;
            printf("%d %d %c\n",p.r,p.c,p.dir);
        }
        // double time=MPI_Wtime() -starttime;
        // cout<<time<<endl;
    }

    // local_pym_particles.clear();
    MPI_Type_free(&MPI_PARTICLE);
    MPI_Finalize();
    return 0;
}