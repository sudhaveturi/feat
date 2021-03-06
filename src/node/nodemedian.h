/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#ifndef NODE_MEDIAN
#define NODE_MEDIAN

#include "node.h"

namespace FT{
	class NodeMedian : public Node
    {
    	public:
    	
    		NodeMedian()
            {
                name = "median";
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
                    tmp(x) = median(limited(stack.z.top().first[x]));
                    
                stack.z.pop();

                stack.f.push(tmp);
                
            }

            /// Evaluates the node symbolically
            void eval_eqn(Stacks& stack)
            {
                stack.fs.push("median(" + stack.zs.pop() + ")");
            }
        protected:
            NodeMedian* clone_impl() const override { return new NodeMedian(*this); }; 
            NodeMedian* rnd_clone_impl() const override { return new NodeMedian(); }; 
    };
}	

#endif
