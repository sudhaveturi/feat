/* FEWTWO
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#ifndef NODE_ROOT
#define NODE_ROOT

#include "nodeDx.h"

namespace FT{
	class NodeRoot : public NodeDx
    {
    	public:
    	
    		NodeRoot()
    		{
    			std::cerr << "error in noderoot.h : invalid constructor called";
				throw;
    		}
    	
    		NodeRoot(string n)
    		{
    			name = n;
    			otype = 'f';
    			arity['f'] = 1;
    			arity['b'] = 0;
    			complexity = 2;
                for (int i = 0; i < arity['f']; i++) {
                    W.push_back(1);
                }
    		}
    		
            /// Evaluates the node and updates the stack states. 
            void evaluate(const MatrixXd& X, const VectorXd& y,
                          const std::map<string, std::pair<vector<ArrayXd>, vector<ArrayXd> > > &Z, 
			              Stacks& stack)
            {
                    stack.f.push(sqrt(abs(W[0]*stack.f.pop())));
            }

            /// Evaluates the node symbolically
            void eval_eqn(Stacks& stack)
            {
                    stack.fs.push("sqrt(|" + stack.fs.pop() + "|)");
            }

            ArrayXd getDerivative(vector<ArrayXd>& stack_f, int loc) {
                switch (loc) {
                    case 1:
                        return stack_f[stack_f.size() - 1] / (2 * sqrt(abs(this->W[0] * stack_f[stack_f.size() - 1])));
                    case 0:
                    default:
                       return W[0] / (2 * sqrt(abs(this->W[0] * stack_f[stack_f.size() - 1])));
                } 
            }

        protected:
            NodeRoot* clone_impl() const override { return new NodeRoot(*this); };  
    };
}	

#endif
