/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#ifndef POPULATION_H
#define POPULATION_H

//#include "node.h" // including node.h since definition of node is in the header
#include "individual.h"
using std::vector;
using std::string;
using Eigen::Map;

namespace FT{    
    ////////////////////////////////////////////////////////////////////////////////// Declarations
    extern Rnd r;
    int last;
    /*!
     * @class Population
     * @brief Defines a population of programs and functions for constructing them. 
     */
    struct Population
    {
        vector<Individual> individuals;     ///< individual programs
        vector<size_t> open_loc;            ///< unfilled matrix positions
        vector<size_t> locs;
        

        Population(){}
        Population(int p)
        {
            individuals.resize(p); 
            locs.resize(2*p); 
            std::iota(locs.begin(),locs.end(),0);
            for (unsigned i = 0; i < individuals.size(); ++i)
                individuals[i].set_id(locs[i]);
        }
        ~Population(){}
        
        /// initialize population of programs. 
        void init(const Individual& starting_model, const Parameters& params, bool random);
        
        /// update individual vector size 
        void resize(int pop_size, bool resize_locs=false)
        {	
            individuals.resize(pop_size); 
            if (resize_locs)        // if this is an initial pop size, locs should be resized
            {
                locs.resize(2*pop_size); 
                std::iota(locs.begin(),locs.end(),0);
            }
        }
        
        /// reduce programs to the indices in survivors. 
        void update(vector<size_t> survivors);
        
        /// returns population size
        int size(){ return individuals.size(); }

        /// returns an open location 
        size_t get_open_loc(); 
        
        /// updates open locations to reflect population.
        void update_open_loc();

        /// adds a program to the population. 
        void add(Individual&);
        
        /// setting and getting from individuals vector
        const Individual operator [](size_t i) const {return individuals.at(i);}
        const Individual & operator [](size_t i) {return individuals.at(i);}

        /// return population equations. 
        string print_eqns(bool,string);

        /// return complexity-sorted Pareto front indices. 
        vector<size_t> sorted_front(unsigned);
        
        /// Sort population in increasing complexity.
        struct SortComplexity
        {
            Population& pop;
            SortComplexity(Population& p): pop(p){}
            bool operator()(size_t i, size_t j)
            { 
                return pop.individuals[i].complexity() < pop.individuals[j].complexity();
            }
        };
        /// check for same fitness and complexity to filter uniqueness. 
        struct SameFitComplexity
        {
            Population & pop;
            SameFitComplexity(Population& p): pop(p){}
            bool operator()(size_t i, size_t j)
            {
                return (pop.individuals[i].fitness == pop.individuals[j].fitness &&
                       pop.individuals[i].complexity() == pop.individuals[j].complexity());
            }
        };

    };

    /////////////////////////////////////////////////////////////////////////////////// Definitions
 
    bool is_valid_program(NodeVector& program, unsigned num_features, 
                          vector<string> longitudinalMap)
    {
        /*! checks whether program fulfills all its arities. */
        Stacks stack;
        
        std::map<string, std::pair<vector<ArrayXd>, vector<ArrayXd>>> Z;
        
        MatrixXd X = MatrixXd::Zero(num_features,2); 
        VectorXd y = VectorXd::Zero(2);
        
         for(auto key : longitudinalMap)
         {
            Z[key].first.push_back(ArrayXd::Zero(2));
            Z[key].first.push_back(ArrayXd::Zero(2));
            Z[key].second.push_back(ArrayXd::Zero(2));
            Z[key].second.push_back(ArrayXd::Zero(2));
         }
         
        Data data(X, y, Z, false);
        
        //cout<<"Enter\n";  
        unsigned i = 0; 
        for (const auto& n : program){
            //cout<<"Evaluating node "<<n->name;
            if (stack.check(n->arity))
            {
                //cout<<"\nEvaluating\n";
                n->evaluate(data, stack);
                //cout<<"Evaluation done\n";
            }
            else{
                std::cout << "Error: ";
                for (const auto& p: program) std::cout << p->name << " ";
                std::cout << "is not a valid program because ";
                std::cout << n->name << " at pos " << i << "is not satisfied\n";
                return false; 
            }
            ++i;
        }
        //cout<<"Exit\n";
        return true;
    }
   
    void make_tree(NodeVector& program, 
                   const NodeVector& functions, 
                   const NodeVector& terminals, int max_d,  
                   const vector<double>& term_weights, char otype, const vector<char>& term_types)
    {  
                
        /*!
         * recursively builds a program with complete arguments.
         */
        
        if (max_d == 0 || r.rnd_flt() < terminals.size()/(terminals.size()+functions.size())) 
        {
            // append terminal 
            vector<size_t> ti;  // indices of valid terminals 
            vector<double> tw;  // weights of valid terminals
            for (size_t i = 0; i<terminals.size(); ++i)
            {
                if (terminals[i]->otype == otype) // grab terminals matching output type
                {
                    ti.push_back(i);
                    tw.push_back(term_weights[i]);                    
                }
            }
            
            if(ti.size() > 0 && tw.size() > 0)
            {
                auto t = terminals[r.random_choice(ti,tw)]->clone();
                //std::cout << t->name << " ";
                program.push_back(t->rnd_clone());
            }
            else
                HANDLE_ERROR_THROW("Error: make_tree couldn't find properly typed terminals");
        }
        else
        {
            // let fi be indices of functions whose output type matches otype and, if max_d==1,
            // with no boolean inputs (assuming all input data is floating point) 
            vector<size_t> fi;
            bool bterms = in(term_types,'b');   // are there boolean terminals?
            bool zterms = in(term_types,'z');   // are there boolean terminals?
            for (size_t i = 0; i<functions.size(); ++i)
                if (functions[i]->otype==otype && (max_d>1 || functions[i]->arity['b']==0 || bterms) 
                    && (max_d>1 || functions[i]->arity['z']==0 || zterms))
                    fi.push_back(i);
            
            if (fi.size()==0){
                if(otype == 'z')
                {
                    make_tree(program, functions, terminals, 0, term_weights, 'z', term_types);
                    return;
                }
                else{            
                    std::cout << "---\n";
                    std::cout << "f1.size()=0. current program: ";
                    for (const auto& p : program) std::cout << p->name << " ";
                    std::cout << "\n";
                    std::cout << "otype: " << otype << "\n";
                    std::cout << "max_d: " << max_d << "\n";
                    std::cout << "functions: ";
                    for (const auto& f: functions) std::cout << f->name << " ";
                    std::cout << "\n";
                    std::cout << "---\n";
                }
            }
            
            assert(fi.size() > 0 && "The operator set specified results in incomplete programs.");
            
            // append a random choice from fs            
            /* auto t = functions[r.random_choice(fi)]->rnd_clone(); */
            //std::cout << t->name << " ";
            program.push_back(functions[r.random_choice(fi)]->rnd_clone());
            
            std::unique_ptr<Node> chosen(program.back()->clone());
            // recurse to fulfill the arity of the chosen function
            for (size_t i = 0; i < chosen->arity['f']; ++i)
                make_tree(program, functions, terminals, max_d-1, term_weights,'f', term_types);
            for (size_t i = 0; i < chosen->arity['b']; ++i)
                make_tree(program, functions, terminals, max_d-1, term_weights, 'b', term_types);
            for (size_t i = 0; i < chosen->arity['z']; ++i)
                make_tree(program, functions, terminals, max_d-1, term_weights, 'z', term_types);
        }

    }

    void make_program(NodeVector& program, 
                      const NodeVector& functions, 
                      const NodeVector& terminals, int max_d, 
                      const vector<double>& term_weights, int dim, char otype, 
                      vector<string> longitudinalMap, const vector<char>& term_types)
    {
        /* cout << "ttypes: \n"; */
        /* for (const auto& t : term_types) */
        /*     cout << t << " "; */ 
        /* cout << "\n"; */
        for (unsigned i = 0; i<dim; ++i)    // build trees
            make_tree(program, functions, terminals, max_d, term_weights, otype, term_types);
        
        // reverse program so that it is post-fix notation
        std::reverse(program.begin(),program.end());
        assert(is_valid_program(program,terminals.size(), longitudinalMap));
    }


    void Population::init(const Individual& starting_model, const Parameters& params,
                          bool random=false)
    {
        /*!
         *create random programs in the population, seeded by initial model weights 
         */
        individuals[0] = starting_model;
        individuals[0].loc = 0;
        
        #pragma omp parallel for
        for (unsigned i = 1; i< individuals.size(); ++i)
        {           
            // pick a dimensionality for this individual
            int dim = r.rnd_int(1,params.max_dim);      
            // pick depth from [params.min_depth, params.max_depth]
            /* unsigned init_max = std::min(params.max_depth, unsigned int(3)); */
            int depth;
            if (random)
                depth = r.rnd_int(1, params.max_depth);
            else
                depth =  r.rnd_int(1, std::min(params.max_depth,unsigned(3)));
            // make a program for each individual
            char ot = r.random_choice(params.otypes);
            make_program(individuals[i].program, params.functions, params.terminals, depth,
                         params.term_weights,dim,ot, params.longitudinalMap, params.ttypes);
            
            /* std::cout << individuals[i].program_str() + " = "; */
            /* std::cout << individuals[i].get_eqn() + "\n"; */
           
            // set location of individual and increment counter             
            individuals[i].loc = i;   
        }
        // define open locations
        update_open_loc(); 
    }
   
   void Population::update(vector<size_t> survivors)
   {

       /*!
        * cull population down to survivor indices.
        */
       vector<size_t> pop_idx(individuals.size());
       std::iota(pop_idx.begin(),pop_idx.end(),0);
       std::reverse(pop_idx.begin(),pop_idx.end());
       for (const auto& i : pop_idx)
           if (!in(survivors,i))
               individuals.erase(individuals.begin()+i);                         
          
       //individuals.erase(std::remove_if(individuals.begin(), individuals.end(), 
       //                  [&survivors](const Individual& ind){ return !in(survivors,ind.loc);}),
       //                  individuals.end());

       // reset the open locations in F matrix 
       update_open_loc();
   
   }

   size_t Population::get_open_loc()
   {
       /*!
        * grabs an open location and removes it from the vector.
        */
       size_t loc = open_loc.back(); open_loc.pop_back();
       return loc;
   }

   void Population::update_open_loc()
   {
       /*!
        * updates open_loc to any locations in [0, 2*popsize-1] not in individuals.loc
        */
       vector<size_t> current_locs, new_open_locs;
      
       for (const auto& ind : individuals)  // get vector of current locations
           current_locs.push_back(ind.loc);

       for (const auto& i : locs)           // find open locations       
        if (!in(current_locs,i))
               new_open_locs.push_back(i); 
       
        open_loc = new_open_locs;      // re-assign open locations             
        //std::cout << "updating open_loc to ";
        //for (auto o: open_loc) std::cout << o << " "; std::cout << "\n";
   }

   void Population::add(Individual& ind)
   {
       /*!
        * adds ind to individuals, giving it an open location and bookeeping.
        */

       ind.loc = get_open_loc();
       individuals.push_back(ind);
   }

   string Population::print_eqns(bool just_offspring=false, string sep="\n")
   {
       string output = "";
       int start = 0;
       
       if (just_offspring)
           start = individuals.size()/2;

       for (unsigned int i=start; i< individuals.size(); ++i)
           output += individuals[i].get_eqn() + sep;
       
       return output;
   }

    vector<size_t> Population::sorted_front(unsigned rank=1)
    {
        /* Returns individuals on the Pareto front, sorted by increasign complexity. */
        vector<size_t> pf;
        for (unsigned int i =0; i<individuals.size(); ++i)
        {
            if (individuals[i].rank == rank)
                pf.push_back(i);
        }
        std::sort(pf.begin(),pf.end(),SortComplexity(*this)); 
        auto it = std::unique(pf.begin(),pf.end(),SameFitComplexity(*this));
        pf.resize(std::distance(pf.begin(),it));
        return pf;
    }
    
}//FT    
#endif
