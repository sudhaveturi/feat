/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#ifndef NODE_KURTOSIS
#define NODE_KURTOSIS

#include "node.h"

namespace FT{
	class NodeKurtosis : public Node
    {
    	public:
    	
    		NodeKurtosis()
            {
                name = "kurtosis";
    			otype = 'f';
    			arity['f'] = 0;
    			arity['b'] = 0;
    			arity['z'] = 1;
    			complexity = 1;
    		}
    		
            /// Evaluates the node and updates the stack states. 
            void evaluate(Data& data, Stacks& stack)
            {
                ArrayXd tmp(stack.z.top().first.size());
                
                int x;
                
                for(x = 0; x < stack.z.top().first.size(); x++)
                    tmp(x) = kurtosis(limited(stack.z.top().first[x]));
                    
                stack.z.pop();
                stack.f.push(tmp);
                
            }

            /// Evaluates the node symbolically
            void eval_eqn(Stacks& stack)
            {
                stack.fs.push("kurtosis(" + stack.zs.pop() + ")");
            }
        protected:
            NodeKurtosis* clone_impl() const override { return new NodeKurtosis(*this); }; 
            NodeKurtosis* rnd_clone_impl() const override { return new NodeKurtosis(); }; 
    };
}	

#endif
