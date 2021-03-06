/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#ifndef NODE_GEQ
#define NODE_GEQ

#include "node.h"

namespace FT{
	class NodeGEQ : public Node
    {
    	public:
    	
   		    NodeGEQ()
    		{
    			name = ">=";
    			otype = 'b';
    			arity['f'] = 2;
    			arity['b'] = 0;
    			complexity = 2;
    		}
    		
            /// Evaluates the node and updates the stack states. 
            void evaluate(Data& data, Stacks& stack)
            {
            	ArrayXd x1 = stack.f.pop();
                ArrayXd x2 = stack.f.pop();
                stack.b.push(x1 >= x2);
            }

            /// Evaluates the node symbolically
            void eval_eqn(Stacks& stack)
            {
                stack.bs.push("(" + stack.fs.pop() + ">=" + stack.fs.pop() + ")");
            }
        protected:
            NodeGEQ* clone_impl() const override { return new NodeGEQ(*this); };  
            NodeGEQ* rnd_clone_impl() const override { return new NodeGEQ(); };  
    };
}	

#endif
