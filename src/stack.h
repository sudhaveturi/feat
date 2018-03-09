/* FEWTWO
copyright 2017 William La Cava
license: GNU/GPL v3
*/

#ifndef STACK_H
#define STACK_H
//external includes

namespace FT
{
    template<typename type>
    class Stack
    {
        private:
            std::vector<type> st;
            
        public:
        
            Stack()
            {
                st = std::vector<type>();
            }
            
            void push(type element)
            {
                st.push_back(element);
            }
            
            type pop()
            {
                type ret = st.back();
                st.pop_back();
                return ret;
            }
            
            bool empty()
            {
                return st.empty();
            }
            
            unsigned int size()
            {
                return st.size();
            }
            
            type& top()
            {
                return st.back();
            }
            
            type& at(int i)
            {
                return st.at(i);
            }
            
            void clear()
            {
                st.clear();
            }
            
            typename vector<type>::iterator begin()
            {
                return st.begin();
            }
            
            typename vector<type>::iterator end()
            {
                return st.end();
            }
            
            typename vector<type>::const_iterator begin() const
            {
                return st.begin();
            }
            
            typename vector<type>::const_iterator end() const
            {
                return st.end();
            }
    };
    
    struct Stacks
    {
        Stack<ArrayXd> f;
        Stack<ArrayXb> b;
        Stack<vector<ArrayXd> > l;
        Stack<string> fs;
        Stack<string> bs;
        Stack<string> ls;
        
    };
}



#endif