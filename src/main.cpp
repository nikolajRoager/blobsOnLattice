/*
LICENCE GOES HERE
*/

#include<string>
#include<iostream>
#include<exception>
#include<algorithm>

#include<cstdint>
#include<random>


using uint   = uint32_t;

using std::stoi, std::stof, std::cout, std::endl, std::default_random_engine, std::vector, std::cerr, std::pair, std::make_pair, std::uniform_int_distribution, std::uniform_real_distribution;

//The main loop, I try to keep anything graphics related out of this.
int main(int argc, char* argv[])
{

    std::random_device r;
    default_random_engine generator(r());

    //cout << "Running with "<<argc<<" arguments, command executed: " << endl;//Some say endl is bad style, but I want to flush in case we crash during startup.

    //for (int i = 0; i < argc; ++i)
    //{
    //    cout << argv[i] << " ";
    //}
    //cout << endl;

    uint sqrtHomes =0;
    uint Homes =0;
    uint Trees =0;

    //rewards
    double any_vs_none     = (argc >= 4) ? stof(argv[3]) : 2.0;
    double friend_vs_friend= (argc >= 5) ? stof(argv[4]) : 1.75;
    double friend_vs_foe   = (argc >= 6) ? stof(argv[5]) : 0.5;
    double foe_vs_friend   = (argc >= 7) ? stof(argv[6]) : 1.5;
    double foe_vs_foe      = (argc >= 8) ? stof(argv[7]) : 0.75;

    if (argc >= 3)
    {
        sqrtHomes =stoi(argv[1]);
        Homes =sqrtHomes*sqrtHomes;
        Trees =stoi(argv[2]);
    }
    else
    {
        //cout<<"Command needed: "<<argv[0]<<" homes trees\n   homes = blob homes per side length\n   trees = trees per blob home"<<endl;
        return 1;
    }

    struct blob_home
    {
        uint blobs=0;//how many blobs are here in total, at most 64, but if I use 8 bit it gets printed as a char
        uint64_t Friendly;//Is this blob friendly or not (each bit is a blob)


        uint new_blobs=0;
        uint64_t new_Friendly;

        uint trees;
        //For display only, where is this home to be displaye
        double display_x;
        double display_y;

        uint free_trees;
        uint half_trees;//trees occupied by a single blob
        vector< pair< pair<uint,uint > , pair<uint,uint> > > Occupied_trees;//The ID of the home and blobs standing around each tree


        blob_home(double _X=0,double _Y=0, uint Trees=1, uint _blobs=0,uint64_t start_friendlies = -1): display_x(_X), display_y(_Y), trees(Trees), blobs(_blobs), Friendly(start_friendlies)
        {
            new_blobs=0;
            new_Friendly=0;

            free_trees=trees;
            half_trees=0;
            Occupied_trees=vector< pair< pair<uint,uint > , pair<uint,uint> > >(Trees);
            for (uint i = 0; i <trees; ++i)
                Occupied_trees[i] = make_pair(make_pair((uint)-1,(uint)-1),make_pair((uint)-1,(uint)-1));
        }

        double get_friendliness()
        {
            int out =0;
            for (uint i = 0; i < blobs; ++i)
            {

                if ((Friendly>> i) & 1)
                    ++out;
                else
                    --out;

            }

            return out;//blobs==0 ? 0 : out/double(blobs);

        }

        void reset(uint& good_people,uint& bad_people)
        {
            bad_people+=0;
            blobs   =new_blobs;
            Friendly=new_Friendly;

            new_blobs=0;
            new_Friendly=0;

            free_trees=trees;
            half_trees=0;


            //cout<<endl;
            //cout<<"Numbers     : "<<blobs<<" -> "<< new_blobs<<endl;
            //cout<<"Friendliness: "<<Friendly<<" -> "<<new_Friendly<<endl;

            for (uint i = 0; i < blobs; ++i)
            {
                if ((Friendly>> i) & 1)
                    ++good_people;
                else
                    ++bad_people;

            }


            for (uint i = 0; i <trees; ++i)
                Occupied_trees[i] = make_pair(make_pair((uint)-1,(uint)-1),make_pair((uint)-1,(uint)-1));
        }

        void visit(pair<uint,uint > blob)
        {
            //Prefer visiting free trees
            if (free_trees!=0)
            {
                Occupied_trees[half_trees].first=blob;
                --free_trees;
                ++half_trees;
            }
            else if (half_trees!=0)
            {
                --half_trees;
                Occupied_trees[half_trees].second=blob;
            }
        }

    };

    //cout<<"Generating lattice with  "<<Homes <<" blob homes, with "<<Trees<<" trees for every home; Each blob will randomly forrage food from the trees at its home, or in one of the 4 neighbouring homes."<<endl;

    vector<blob_home> home_list(sqrtHomes*sqrtHomes);
    vector < pair<uint,uint> > action_priority(sqrtHomes*sqrtHomes*64);//What order do the blobs get to pick their tree in (home_ID,blob_ID in that home), yes most of these are empty, we will skip those

    double dx_i = 1/double(sqrtHomes-1);


    uniform_int_distribution<uint> position_start_dist(0,sqrtHomes);
    uint x0 = sqrtHomes/4;
    uint y0 = sqrtHomes/2;
    uint x1 = (3*sqrtHomes)/4;
    uint y1 = sqrtHomes/2;

    for (uint i = 0; i < sqrtHomes; ++i)// right
        for (uint j = 0; j < sqrtHomes; ++j) // up
        {

            uint ID       = i*(sqrtHomes) + j;
            home_list[ID] = blob_home(dx_i*i,dx_i*j,Trees, 0);

            if (i == x0 && j == y0)
            {
                home_list[ID].Friendly=0;
                ++home_list[ID].blobs;
            }
            else if (i == x1 && j == y1)
            {
                home_list[ID].Friendly=-1;
                ++home_list[ID].blobs;
            }

            for (uint k = 0; k < 64; ++k)
                action_priority[ID*64+k]=make_pair(ID,k);
        }



    for (uint turn = 0; turn<256; ++turn)
    {


        //Update what trees each blob goes to, start by distributing the blobs among the trees; The order the blobs find trees in is randomized, such that no-one gets an unfair priority
        shuffle(action_priority.begin(), action_priority.end(), generator);

        for (pair<uint,uint > blob : action_priority)
        {
            if (home_list[blob.first].blobs>blob.second)//If this blob even exists
            {
                //cout<<"Blob "<<blob.second<<" at "<<blob.first<<" looks for food"<<endl;

                //Everyone prefers their own tree
                uint free_trees_home = home_list[blob.first].free_trees;
                uint free_trees_l =(blob.first/sqrtHomes != 0)            ? home_list[blob.first-sqrtHomes].free_trees : 0;
                uint free_trees_r =(blob.first/sqrtHomes != sqrtHomes -1) ? home_list[blob.first+sqrtHomes].free_trees: 0;
                uint free_trees_u = (blob.first%sqrtHomes != 0)           ? home_list[blob.first-1].free_trees : 0;
                uint free_trees_d = (blob.first%sqrtHomes != sqrtHomes -1)? home_list[blob.first+1].free_trees: 0;

                uint total_free_trees=free_trees_home +free_trees_l +free_trees_r+free_trees_u+free_trees_d;
                if (total_free_trees != 0)
                {
                    //cout<<" Can check out "<<total_free_trees<<" free trees"<<endl;

                    uniform_int_distribution<uint> dist(0,total_free_trees-1);
                    uint target_tree = dist(generator);
                    if (target_tree<free_trees_home)
                    {
                        //cout<<"Visiting "<<blob.first<<endl;
                        home_list[blob.first].visit(blob);
                    }
                    else
                    {
                        target_tree-=free_trees_home;

                        if (target_tree<free_trees_l)
                        {
                            //cout<<"Visiting <- "<<(blob.first-sqrtHomes)<<endl;
                            home_list[blob.first-sqrtHomes].visit(blob);
                        }
                        else
                        {
                            target_tree-=free_trees_l;

                            if (target_tree<free_trees_r)
                            {
                                //cout<<"Visiting -> "<<(blob.first+sqrtHomes)<<endl;
                                home_list[blob.first+sqrtHomes].visit(blob);
                            }
                            else
                            {
                                target_tree-=free_trees_r;

                                if (target_tree<free_trees_u)
                                {
                                    //cout<<"Visiting v "<<(blob.first-1)<<endl;
                                    home_list[blob.first-1].visit(blob);
                                }
                                else
                                {
                                    target_tree-=free_trees_u;
                                    //cout<<"Visiting ^ "<<(blob.first+1)<<endl;
                                    home_list[blob.first+1].visit(blob);
                                }
                            }
                        }
                    }
                }
                else
                {


    uint half_trees_home = home_list[blob.first].half_trees;
    uint half_trees_l =(blob.first/sqrtHomes != 0)            ? home_list[blob.first-sqrtHomes].half_trees : 0;
    uint half_trees_r =(blob.first/sqrtHomes != sqrtHomes -1) ? home_list[blob.first+sqrtHomes].half_trees: 0;
    uint half_trees_u = (blob.first%sqrtHomes != 0)           ? home_list[blob.first-1].half_trees : 0;
    uint half_trees_d = (blob.first%sqrtHomes != sqrtHomes -1)? home_list[blob.first+1].half_trees: 0;

    uint total_half_trees=half_trees_home +half_trees_l +half_trees_r+half_trees_u+half_trees_d;


                    if (total_half_trees != 0)
                    {
                        //cout<<" Can check out "<<total_half_trees<<" half trees"<<endl;

                        uniform_int_distribution<uint> dist(0,total_half_trees-1);
                        uint target_tree = dist(generator);
                        if (target_tree<half_trees_home)
                        {
                            //cout<<"Visiting "<<blob.first<<endl;
                            home_list[blob.first].visit(blob);
                        }
                        else
                        {
                            target_tree-=half_trees_home;

                            if (target_tree<half_trees_l)
                            {
                                //cout<<"Visiting <- "<<(blob.first-sqrtHomes)<<endl;
                                home_list[blob.first-sqrtHomes].visit(blob);
                            }
                            else
                            {
                                target_tree-=half_trees_l;

                                if (target_tree<half_trees_r)
                                {
                                    //cout<<"Visiting -> "<<(blob.first+sqrtHomes)<<endl;
                                    home_list[blob.first+sqrtHomes].visit(blob);
                                }
                                else
                                {
                                    target_tree-=half_trees_r;

                                    if (target_tree<half_trees_u)
                                    {
                                        //cout<<"Visiting v "<<(blob.first-1)<<endl;
                                        home_list[blob.first-1].visit(blob);
                                    }
                                    else
                                    {
                                        target_tree-=half_trees_u;
                                        //cout<<"Visiting ^ "<<(blob.first+1)<<endl;
                                        home_list[blob.first+1].visit(blob);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        //cout<<" No free trees, starved to death "<<endl;
                    }

                }
            }

        }


        auto reproduce =[&home_list,sqrtHomes,&generator](pair<uint,uint>& blob) -> void
        {
            uint neighbour_l =(blob.first/sqrtHomes != 0)            ? 1 : 0;
            uint neighbour_r =(blob.first/sqrtHomes != sqrtHomes -1) ? 1 : 0;
            uint neighbour_u =(blob.first%sqrtHomes != 0)            ? 1 : 0;
            uint neighbour_d =(blob.first%sqrtHomes != sqrtHomes -1) ? 1 : 0;

            uniform_int_distribution<uint> dist(0,neighbour_l+neighbour_r+neighbour_u+neighbour_d);

            //Get where the next one spawns
            uint target_spawn = 0;

            uint target_spawn_id = dist(generator);
            if (target_spawn_id ==0)
            {
                target_spawn =blob.first;
            }
            else
            {
                --target_spawn_id;

                if (target_spawn_id<neighbour_l )
                {
                    target_spawn =blob.first-sqrtHomes;
                }
                else
                {
                    target_spawn_id-=neighbour_l;

                    if (target_spawn_id<neighbour_r)
                    {
                        target_spawn =blob.first+sqrtHomes;
                    }
                    else
                    {
                        target_spawn_id-=neighbour_r;

                        if (target_spawn_id<neighbour_u)
                        {
                            target_spawn =blob.first-1;
                        }
                        else
                        {
                            target_spawn =blob.first+1;
                        }
                    }
                }
            }





            if (home_list[target_spawn].new_blobs<64)
            {



                home_list[target_spawn].new_Friendly = home_list[target_spawn].new_Friendly | ((home_list[blob.first].Friendly>> blob.second) & 1)<<home_list[target_spawn].new_blobs;


                ++home_list[target_spawn].new_blobs;
            }
        };


        //Now, check out all the trees where blobs live, and have them eat and reproduce
        for (blob_home& h : home_list)
        {

            for (uint i = 0; i <h.trees; ++i)
            {

                pair<uint,uint>& blob0 = h.Occupied_trees[i].first;
                pair<uint,uint>& blob1 = h.Occupied_trees[i].second;

                uniform_real_distribution<double> range_01(0.0,1.0);

                if(blob0.first!=(uint)-1)//-1: there are noone there
                {
                    if(blob1.first==(uint)-1)//I am Alone
                    {
                        //We gain this reward, so we make this many copies of myself, can only go in my home
                        double my_reward = any_vs_none;


                        //For every one above or equal to 1, reproduce guaranteed
                        for (; my_reward>=1; --my_reward)
                        {
                            reproduce(blob0);
                        }
                        if (range_01(generator)<my_reward)
                        {
                            reproduce(blob0);
                        }
                    }
                    else
                    {


                        bool friend0 = (home_list[blob0.first].Friendly >>blob0.second & 1);
                        bool friend1 = (home_list[blob1.first].Friendly >>blob1.second & 1);

                        double   my_reward = 0.0;
                        double your_reward = 0.0;

                        if (friend0 && friend1)//Lets help
                        {
                              my_reward = friend_vs_friend;
                            your_reward = friend_vs_friend;
                        }
                        else if (friend0 && !friend1)//Hey ... stop, what are you doing!
                        {
                              my_reward = friend_vs_foe;
                            your_reward = foe_vs_friend;
                        }
                        else if (!friend0 && friend1)//Ha ha idiot
                        {
                              my_reward = foe_vs_friend;
                            your_reward = friend_vs_foe;

                        }
                        else//Grr
                        {
                              my_reward = foe_vs_foe;
                            your_reward = foe_vs_foe;
                        }


                        for (; my_reward>=1; --my_reward)
                        {
                            reproduce(blob0);
                        }
                        if (range_01(generator)<my_reward)
                        {
                            reproduce(blob0);
                        }

                        for (; your_reward>=1; --your_reward)
                        {
                            reproduce(blob1);
                        }
                        if (range_01(generator)<your_reward)
                        {
                            reproduce(blob1);
                        }

                    }
                }

            }

            //Reset all trees here
        }


        uint good_people=0;
        uint bad_people=0;
        for (blob_home& h : home_list)
            h.reset(good_people,bad_people);


        cout<<turn<<'\t'<<good_people<<'\t'<<bad_people<<endl;



        cerr<<sqrtHomes;
        for (uint i = 0; i < sqrtHomes; ++i)
            cerr<<'\t'<<i;
        cerr<<endl;
        for (uint j = 0; j < sqrtHomes; ++j)
        {
            cerr<<j;
            for (uint i = 0; i < sqrtHomes; ++i)
            {
                uint Id       = i*(sqrtHomes) + j;
                //Print the number of blobs on each site, in a format gnuplot can plot
                cerr<<'\t'<<home_list[Id].blobs;
            }
            cerr<<endl;
        }
        cerr<<endl;
        cerr<<endl;



        cerr<<sqrtHomes;
        for (uint i = 0; i < sqrtHomes; ++i)
            cerr<<'\t'<<i;
        cerr<<endl;
        for (uint j = 0; j < sqrtHomes; ++j)
        {
            cerr<<j;
            for (uint i = 0; i < sqrtHomes; ++i)
            {
                uint Id       = i*(sqrtHomes) + j;
                //Print the number of blobs on each site, in a format gnuplot can plot
                cerr<<'\t'<<home_list[Id].get_friendliness();
            }
            cerr<<endl;
        }
        cerr<<endl;
        cerr<<endl;




    }

    return 0;
}
