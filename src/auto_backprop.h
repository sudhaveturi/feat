#ifndef AUTO_BACKPROP_H
#define AUTO_BACKPROP_H

#include <Eigen/Dense>
#include <iostream>
#include <map>
#include <vector>
#include "nodevector.h"
#include "stack.h"
#include "node/node.h"
#include "node/nodeDx.h"
#include "metrics.h"
#include "individual.h"
#include "ml.h"
#include "init.h"
#include "params.h"

#include <cmath>
#include <shogun/labels/Labels.h>

using shogun::CLabels;
using Eigen::MatrixXd;
using Eigen::VectorXd;
typedef Eigen::Array<bool,Eigen::Dynamic,1> ArrayXb;
using std::cout;
/**
TODO
------------------------
Integrate vectorList
Integrate pointers?
TODO Make it so stops traversing once it hits a non-differentiable node and then goes upstream and finds another branch to traverse
**/

namespace FT {
	class AutoBackProp 
    {
        /* @class AutoBackProp
         * @brief performs back propagation on programs to adapt weights.
         */
	public:
	
        typedef VectorXd (*callback)(const VectorXd&, shared_ptr<CLabels>&, const vector<float>&);
        
        std::map<string, callback> d_score_hash;
        std::map<string, callback> score_hash;
        
        AutoBackProp(string scorer, int iters=1000, double n=0.1, double a=0.9) 
        {
			/* this->program = program.get_data(); */
            score_hash["mse"] = & metrics::squared_difference;
            score_hash["log"] =  & metrics::log_loss; 
            score_hash["multi_log"] =  & metrics::multi_log_loss;
	        d_score_hash["mse"] = & metrics::d_squared_difference;
            d_score_hash["log"] =  & metrics::d_log_loss; 
            d_score_hash["multi_log"] =  & metrics::d_multi_log_loss;
			
            this->d_cost_func = d_score_hash[scorer]; 
            this->cost_func = score_hash[scorer]; 
			/* this->X = X; */
			/* this->labels = labels; */
			this->iters = iters;
			this->n = n;
            this->epT = 0.01*this->n;   // min learning rate
			this->a = a;
		}
        /// adapt weights
		void run(Individual& ind, Data d,
                 const Parameters& params);

        /* ~AutoBackProp() */
        /* { */
        /*     /1* for (const auto& p: program) *1/ */
        /*         /1* p = nullptr; *1/ */
        /* } */


	private:
		double n;                   //< learning rate
        double a;                   //< momentum
        callback d_cost_func;       //< derivative of cost function pointer
        callback cost_func;         //< cost function pointer
        int iters;                  //< iterations
        double epk;                 //< current learning rate 
        double epT;                  //< min learning rate

		struct BP_NODE
		{
			NodeDx* n;
			vector<ArrayXd> deriv_list;
		};

		void print_weights(NodeVector& program) {
			for (const auto& p : program) 
            {
				cout << "( " << p->name;
				if (p->isNodeDx()) {
                    
					NodeDx* dNode = dynamic_cast<NodeDx*>(p.get());
					for (int i = 0; i < dNode->arity['f']; i++) {
						cout << "," << dNode->W.at(i);
					}
                    dNode = nullptr;
				}

				cout << " ) ";
			}
            /* cout << "\n"; */
		}
		/// Return the f_stack
		vector<Trace> forward_prop(Individual& ind, Data d,
                               MatrixXd& Phi, const Parameters& params);

		/// Updates stacks to have proper value on top
		void next_branch(vector<BP_NODE>& executing, vector<Node*>& bp_program, 
                         vector<ArrayXd>& derivatives);

        /// Compute gradients and update weights 
        void backprop(Trace& f_stack, NodeVector& program, int start, int end, 
                                double Beta, shared_ptr<CLabels>& yhat, 
                                Data d,
                               vector<float> sw);

        /// select random subset of data for training weights.
        void get_batch(Data d, Data db, int batch_size);
       
		template <class T>
		T pop(vector<T>* v) {
			T value = v->back();
			v->pop_back();
			return value;
		}

		template <class T>
		T pop_front(vector<T>* v) {
			T value = v->front();
			v->erase(v->begin());
			return value;
		}


	};

/////////////////////////////////////////////////////////////////////////////////////// Definitions

    void AutoBackProp::get_batch(Data d, Data db, int batch_size)
    {

        batch_size =  std::min(batch_size,int(d.y.size()));
        vector<size_t> idx(d.y.size());
        std::iota(idx.begin(), idx.end(), 0);
        r.shuffle(idx.begin(), idx.end());
        db.X.resize(d.X.rows(),batch_size);
        db.y.resize(batch_size);
        for (const auto& val: d.Z )
        {
            db.Z[val.first].first.resize(batch_size);
            db.Z[val.first].second.resize(batch_size);
        }
        for (unsigned i = 0; i<batch_size; ++i)
        {
           
           db.X.col(i) = d.X.col(idx.at(i)); 
           db.y(i) = d.y(idx.at(i)); 

           for (const auto& val: d.Z )
           {
                db.Z[val.first].first.at(i) = d.Z.at(val.first).first.at(idx.at(i));
                db.Z[val.first].second.at(i) = d.Z.at(val.first).second.at(idx.at(i));
           }
        }
        /* std::cout << "exiting batch\n"; */
    }

    void AutoBackProp::run(Individual& ind, Data d,
                            const Parameters& params)
    {
        vector<size_t> roots = ind.program.roots();
        double min_loss;
        double current_loss, current_val_loss;
        vector<vector<double>> best_weights;
        // batch data
        MatrixXd Xb, Xb_v;
        VectorXd yb, yb_v;
        std::map<string, std::pair<vector<ArrayXd>, vector<ArrayXd> > > Zb, Zb_v;
        /* cout << "y: " << d.y.transpose() << "\n"; */ 
        Data db(Xb, yb, Zb, params.classification);
        Data db_val(Xb_v, yb_v, Zb_v, params.classification);
        db_val.set_validation();    // make this a validation set
        get_batch(d, db_val, params.bp.batch_size);     // draw a batch for the validation data
        
        int patience = 3;   // number of iterations to allow validation fitness to not improve
        int missteps = 0;

        this->epk = n;  // starting learning rate
        /* params.msg("running backprop on " + ind.get_eqn(), 2); */
        params.msg("=========================",2);
        params.msg("Iteration,Train Loss,Val Loss,Weights",2);
        params.msg("=========================",2);
        for (int x = 0; x < this->iters; x++)
        {
            /* cout << "get batch\n"; */
            // get batch data for training
            get_batch(d, db, params.bp.batch_size); 
            /* cout << "db.y: " << db.y.transpose() << "\n"; */ 
            // Evaluate forward pass
            MatrixXd Phi; 
            /* cout << "forward pass\n"; */
            vector<Trace> stack_trace = forward_prop(ind, db, Phi, params);
            // Evaluate ML model on Phi
            bool pass = true;
            auto ml = std::make_shared<ML>(params, true);

            /* cout << "ml fit\n"; */
            shared_ptr<CLabels> yhat = ml->fit(Phi,db.y,params,pass,ind.dtypes);
            vector<double> Beta = ml->get_weights();
            /* cout << "cost func\n"; */
            current_loss = this->cost_func(db.y,yhat, params.class_weights).mean();
            
            
            if (!pass || stack_trace.size() ==0 )
                break;

            // Evaluate backward pass
            size_t s = 0;
            for (int i = 0; i < stack_trace.size(); ++i)
            {
                while (!ind.program.at(roots[s])->isNodeDx()) ++s;
                /* cout << "running backprop on " << ind.program_str() << " from " */
                /*      << roots.at(s) << " to " */ 
                /*     << ind.program.subtree(roots.at(s)) << "\n"; */
                
                backprop(stack_trace.at(i), ind.program, ind.program.subtree(roots.at(s)), 
                        roots.at(s), Beta.at(s)/ml->N.scale.at(s), yhat, db, params.class_weights);
            }

            // check validation fitness for early stopping
            MatrixXd Phival = ind.out(db_val,params);
            shared_ptr<CLabels> y_val = ml->predict(Phival);
            current_val_loss = this->cost_func(db_val.y, y_val, params.class_weights).mean();
            
            if (x==0 || current_val_loss < min_loss)
            {
                min_loss = current_val_loss;
                best_weights = ind.program.get_weights();
                params.msg("new min loss: " + std::to_string(min_loss), 2);
            }
            else
            {
                ++missteps;
                /* cout << "missteps: " << missteps << "\n"; */
                params.msg("",2);           // update learning rate
            }
            if (missteps == patience || std::isnan(min_loss) || std::isinf(min_loss)
                    || min_loss <= NEAR_ZERO)       // early stopping trigger
                break;

            double alpha = double(x)/double(iters);
            this->epk = (1 - alpha)*this->epk + alpha*this->epT;  
            /* this->epk = this->epk + this->epT; */ 
            /* cout << "epk: " << this->epk << "\n"; */
            if (params.verbosity>1)
            {
                cout << x << "," 
                 << current_loss << "," 
                 << current_val_loss << ",";
                 print_weights(ind.program);
            }
        }
        params.msg("",2);
        params.msg("=========================",2);
        params.msg("done=====================",2);
        params.msg("=========================",2);
        ind.program.set_weights(best_weights);
    }

    // forward pass
    vector<Trace> AutoBackProp::forward_prop(Individual& ind, Data d,
                               MatrixXd& Phi, const Parameters& params) 
    {
        /* cout << "Forward pass\n"; */
        // Iterate through all the nodes evaluating and tracking ouputs
        vector<Trace> stack_trace;
        Phi = ind.out_trace(d, params, stack_trace);
        // Use stack_f and execution stack to avoid issue of branches affecting what elements 
        // appear before a node 
        /* cout << "Returning forward pass.\n"; */
        return stack_trace;
    }   
    // Updates stacks to have proper value on top
    void AutoBackProp::next_branch(vector<BP_NODE>& executing, vector<Node*>& bp_program, 
                                   vector<ArrayXd>& derivatives) 
    {
        // While there are still nodes with branches to explore
        if(!executing.empty()) {
            // Declare variable to hold node and its associated derivatives
            BP_NODE bp_node = pop<BP_NODE>(&executing); // Check first element
            // Loop until branch to explore is found
            while (bp_node.deriv_list.empty() && !executing.empty()) {
                bp_node = pop<BP_NODE>(&executing); // Get node and its derivatves

                // For some reason this function is not removing element from the stack
                pop<ArrayXd>(&derivatives); // Remove associated gradients from stack
                if (executing.empty()) {
                    return;
                }
            }
            
            // Should now have the next parent node and derivatves (stored in bp_node)
            if (!bp_node.deriv_list.empty()) 
            {
                bp_program.push_back(bp_node.n);
                // Pull derivative from front of list due to how we stored them earlier
                derivatives.push_back(pop_front<ArrayXd>(&(bp_node.deriv_list)));                 
                // Push it back on the stack in order to sync all the stacks
                executing.push_back(bp_node);             
            }
        }
    }

    // Compute gradients and update weights 
    void AutoBackProp::backprop(Trace& stack, NodeVector& program, int start, int end, 
                                double Beta, shared_ptr<CLabels>& yhat, 
                                Data d,
                                vector<float> sw)    
    {
        /* cout << "Backward pass \n"; */
        vector<ArrayXd> derivatives;
        // start with derivative of cost function wrt ML output times dyhat/dprogram output, which
        // is equal to the weight the model assigned to this subprogram (Beta)
        // push back derivative of cost function wrt ML output
        /* cout << "Beta: " << Beta << "\n"; */ 
        derivatives.push_back(this->d_cost_func(d.y, yhat, sw).array() * Beta); //*phi.array()); 
        /* cout << "Cost derivative: " << derivatives[derivatives.size() -1 ]<< "\n"; */ 
        // Working according to test program */
        /* pop<ArrayXd>(&f_stack); // Get rid of input to cost function */
        vector<BP_NODE> executing; // Stores node and its associated derivatves
        // Currently I don't think updates will be saved, might want a pointer of nodes so don't 
        // have to restock the list
        // Program we loop through and edit during algorithm (is this a shallow or deep copy?)
        /* cout << "copy program \n"; */
        vector<Node*> bp_program = program.get_data(start, end);         
        /* cout << "Initializing backprop systems.\n"; */
        while (bp_program.size() > 0) {
            /* cout << "Size of program: " << bp_program.size() << "\n"; */
            Node* node = pop<Node*>(&bp_program);
            /* cout << "Evaluating: " << node->name << "\n"; */
            /* cout << "executing stack: " ; */ 
            /* for (const auto& bpe : executing) cout << bpe.n->name << " " ; cout << "\n"; */
            /* cout << "bp_program: " ; */ 
            /* for (const auto& bpe : bp_program) cout << bpe->name << " " ; cout << "\n"; */
            /* cout << "derivatives size: " << derivatives.size() << "\n"; */ 
            vector<ArrayXd> n_derivatives;

            if (node->isNodeDx() && node->visits == 0 && node->arity['f'] > 0) {
                NodeDx* dNode = dynamic_cast<NodeDx*>(node); // Could probably put this up one and have the if condition check if null
                /* cout << "evaluating derivative\n"; */
                // Calculate all the derivatives and store them, then update all the weights and throw away the node
                for (int i = 0; i < node->arity['f']; i++) {
                    dNode->derivative(n_derivatives, stack, i);
                }
                /* cout << "updating derivatives\n"; */
                dNode->update(derivatives, stack, this->epk, this->a);
                // dNode->print_weight();
                /* cout << "popping input arguments\n"; */
                // Get rid of the input arguments for the node
                for (int i = 0; i < dNode->arity['f']; i++) {
                    pop<ArrayXd>(&stack.f);
                }
                for (int i = 0; i < dNode->arity['b']; i++) {
                    pop<ArrayXb>(&stack.b);
                }
                if (!n_derivatives.empty()) {
                    derivatives.push_back(pop_front<ArrayXd>(&n_derivatives));
                }

                executing.push_back({dNode, n_derivatives});
            }
            /* else */
            /*     cout << "not NodeDx or visits reached or no floating arity\n"; */
            /* cout << "next branch\n"; */
            // Choosing how to move through tree
            if (node->arity['f'] == 0 || !node->isNodeDx()) {
        
                // Clean up gradients and find the parent node
                /* cout << "popping derivatives\n"; */
                if (!derivatives.empty())
                    pop<ArrayXd>(&derivatives);	// TODO check if this fixed
                next_branch(executing, bp_program, derivatives);
            } 
            else 
            {
                node->visits += 1;
                if (node->visits > node->arity['f']) 
                {
                    next_branch(executing, bp_program, derivatives);
                }
            }
        }

        // point bp_program to null
        for (unsigned i = 0; i < bp_program.size(); ++i)
            bp_program[i] = nullptr;

        /* cout << "Backprop terminated\n"; */
        //print_weights(program);
    }
}

#endif
