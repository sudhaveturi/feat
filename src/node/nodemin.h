/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#ifndef NODE_MIN
#define NODE_MIN

#include "node.h"

namespace FT{
	class NodeMin : public Node
    {
    	public:
    	
    		NodeMin()
            {
                name = "min";
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
                    tmp(x) = limited(stack.z.top().first[x]).minCoeff();
                    
                stack.z.pop();

                stack.f.push(tmp);
                
            }

            /// Evaluates the node symbolically
            void eval_eqn(Stacks& stack)
            {
                stack.fs.push("min(" + stack.zs.pop() + ")");
            }
        protected:
            NodeMin* clone_impl() const override { return new NodeMin(*this); }; 
            NodeMin* rnd_clone_impl() const override { return new NodeMin(); }; 
    };
}	

#endif
