/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#ifndef NODE_SLOPE
#define NODE_SLOPE

#include "node.h"

namespace FT{
	class NodeSlope : public Node
    {
    	public:
    	
    		NodeSlope()
            {
                name = "slope";
    			otype = 'f';
    			arity['f'] = 0;
    			arity['b'] = 0;
    			arity['z'] = 1;
    			complexity = 4;
    		}
    		
            /// Evaluates the node and updates the stack states. 
            void evaluate(Data& data, Stacks& stack)
            {
                ArrayXd tmp(stack.z.top().first.size());
                
                for(int x = 0; x < stack.z.top().first.size(); x++)                    
                    tmp(x) = slope(limited(stack.z.top().first[x]), limited(stack.z.top().second[x]));
                    
                stack.z.pop();

                stack.f.push(tmp);
                
            }

            /// Evaluates the node symbolically
            void eval_eqn(Stacks& stack)
            {
                stack.fs.push("slope(" + stack.zs.pop() + ")");
            }

            double slope(const ArrayXd& x, const ArrayXd& y)
            {
                double varx = variance(x);
                if (varx > NEAR_ZERO)
                    return covariance(x, y)/varx;
                else
                    return 0;
            }

        protected:
            NodeSlope* clone_impl() const override { return new NodeSlope(*this); }; 
            NodeSlope* rnd_clone_impl() const override { return new NodeSlope(); }; 
    };
}	

#endif
