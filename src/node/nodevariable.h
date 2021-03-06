/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#ifndef NODE_VARIABLE
#define NODE_VARIABLE

#include "node.h"

namespace FT{
	class NodeVariable : public Node
	{
		public:
			size_t loc;             ///< column location in X, for x types
			
			NodeVariable(const size_t& l, char ntype = 'f', std::string n="")
			{
                if (n.empty())
    			    name = "x_" + std::to_string(l);
                else
                    name = n;
    			otype = ntype;
    			arity['f'] = 0;
    			arity['b'] = 0;
    			complexity = 1;
    			loc = l;
    		}
    		
    		/// Evaluates the node and updates the stack states. 		
			void evaluate(Data& data, Stacks& stack)
		    {
	    		if (otype == 'b')
	                stack.b.push(data.X.row(loc).cast<bool>());
	            else
	                stack.f.push(data.X.row(loc));
		    }

		    /// Evaluates the node symbolically
		    void eval_eqn(Stacks& stack)
		    {
	    		if (otype == 'b')
	                stack.bs.push(name);
	            else
	                stack.fs.push(name);
		    }
	    protected:
            NodeVariable* clone_impl() const override { return new NodeVariable(*this); };  
            // rnd_clone is just clone_impl() for variable, since rand vars not supported
            NodeVariable* rnd_clone_impl() const override { return new NodeVariable(*this); };  
    };
}

#endif
