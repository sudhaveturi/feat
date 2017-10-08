/* FEWTWO
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#ifndef NODE_DIVIDE
#define NODE_DIVIDE

#include "node.h"

namespace FT{
	class NodeDivide : public Node
    {
    	public:
    	
    		NodeDivide()
    		{
    			std::cerr << "error in node.h : invalid constructor called";
				throw;
    		}
    	
    		NodeDivide(string n) : name(n),
    							 otype('f'),
    							 arity['f'](2),
    							 arity['b'](0),
    							 complexity(2) {}
    		
    		/*!
             * @brief Evaluates the node and updates the stack states. 
             */
            void evaluate(const MatrixXd& X, const VectorXd& y, vector<ArrayXd>& stack_f, vector<ArrayXi>& stack_b)
            {
            	if (stack_f.size() >= arity['f'] && stack_b.size() >= arity['b'])
            	{
            		ArrayXd x = stack_f.back(); stack_f.pop_back();
                    ArrayXd y = stack_f.back(); stack_f.pop_back();
                    stack_f.push_back(x / y);
            	}
            }

            /*!
             * @brief evaluates the node symbolically
             */
            void eval_eqn(vector<string>& stack_f, vector<string>& stack_b)
            {
            	if (stack_f.size() >= arity['f'] && stack_b.size() >= arity['b'])
            	{
            		string x = stack_f.back(); stack_f.pop_back();
                    string y = stack_f.back(); stack_f.pop_back();
                    stack_f.push_back("(" + x + "/" + y + ")");            	
            	}
            }
    };
}	
