/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#ifndef NODE_RELU
#define NODE_RELU

#include "nodeDx.h"

namespace FT{
	class NodeRelu : public NodeDx
    {
    	public:
    	  	
    		NodeRelu(vector<double> W0 = vector<double>())
    		{
    			name = "relu";
    			otype = 'f';
    			arity['f'] = 1;
    			arity['b'] = 0;
    			complexity = 2;

                if (W0.empty())
                {
                    for (int i = 0; i < arity['f']; i++) {
                        W.push_back(r.rnd_dbl());
                    }
                }
                else
                    W = W0;
    		}
    		
            /// Evaluates the node and updates the stack states. 
             void evaluate(Data& data, Stacks& stack)
            {
                ArrayXd x = stack.f.pop();
                ArrayXd res = (W[0] * x > 0).select(W[0]*x, ArrayXd::Zero(x.size())+0.01); 
                stack.f.push(res);
            }

            /// Evaluates the node symbolically
             void eval_eqn(Stacks& stack)
            {
                stack.fs.push("relu("+ stack.fs.pop() +")");         	
            }

            ArrayXd getDerivative(Trace& stack, int loc) {

                ArrayXd x = stack.f[stack.f.size()-1];
                switch (loc) {
                    case 1: // d/dW
                        return (x>0).select(x,ArrayXd::Zero(x.size())+0.01);
                    case 0: // d/dx
                    default:
                        return (x>0).select(W[0],ArrayXd::Zero(x.size())+0.01);
                } 
            }

            protected:
            NodeRelu* clone_impl() const override { return new NodeRelu(*this); };  
            NodeRelu* rnd_clone_impl() const override { return new NodeRelu(); };  
    };
}	

#endif
