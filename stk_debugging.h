/*
This file is part of the STK

The code in this file is normally released under the GPLv3+ (see GPL
header below), unless you or your company have got an exception
agreement with us. Contact us at contact@shin-project.org.
By making changes to this file, you give us the right to reproduce,
adapt, translate, modify, and prepare derivative works of, publicly
display, publicly perform, sublicense, make available and distribute
your changes under either the GPLv3+ or under the terms of another
license to, for example, make your changes also available for users who
can't or don't want to put their software under the GPLv3+, like
proprietary software. Though you are not required to send us your
changes. So if you don't give us access to your modifications, this
requirement will not affect you.

--- GPL 3 HEADER START ---
The STK is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The STK is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the STK. If not, see <http://www.gnu.org/licenses/>.
--- GPL 3 HEADER END ---
*/

#ifndef STK_DEBUGGING_H
#define STK_DEBUGGING_H

#include <string>
#include <iostream>
#include <functional>
#include <vector>
#include <map>
#include <set>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

#include "stk_misc.h"

namespace stk
{

/// @brief This function is fed with a type, like `type_name<int>()`, and it returns a `std::string` containing the name of the type, like "int".
template<typename T>
std::string type_name()
{
    char* name=(char*)typeid(T).name();
#ifdef __GNUC__
    int status;
    name=abi::__cxa_demangle(name,0,0,&status);
#endif

    std::string ret(name);

#ifdef __GNUC__
    free(name);
    name=0;
#endif
    return ret;
}

/// \brief Assures that the given string is only given once to the construcor of this class. Will throw/abort() otherwise. Mostly for debugging.
class unique_string
{
public:
    unique_string(std::string key)
    {
        static std::set<std::string> keys;
        if(keys.count(key))
        {
            throw stk::exception("stk::unique violation with '"+key+"'");
            std::cerr<<"stk::unique violation with '"+key+"'"<<std::endl;
            abort();
        }
        keys.insert(key);
    }
};

} // namespace stk

/*inline std::ostream& operator<<(std::ostream& os,const std::vector<std::string>& s)
{
    os<<"{"<<stk::join<std::vector<std::string>,std::string>(s,',')<<"}";
    return os;
}*/

template <typename T>
inline std::ostream& operator<<(std::ostream& os,const std::vector<T>& s)
{
    os<<"{";
    for(auto it=s.begin();it!=s.end();it++)
    {
        os<<"'"<<*it<<"'";
        it++;   // look at the next element
        if(it!=s.end())
            os<<", ";
        it--;
    }
    os<<"}";
    return os;
}

template <typename T1,typename T2>
inline std::ostream& operator<<(std::ostream& os,const std::map<T1,T2>& map)
{
    os<<"{";
    for(auto it=map.begin();it!=map.end();it++)
    {
        os<<"{'"<<(*it).first<<"','"<<(*it).second<<"'}";
        it++;   // look at the next element
        if(it!=map.end())
            os<<", "<<std::endl;
        it--;
    }
    os<<"}";
    return os;
}

/// @brief This macro is useful for debug output and can be used like `std::cout<<PRINT(name)<<PRINT(i)<<std::endl;` and outputs "name: Dave i: 42"
#define PRINT(x) #x<<": "<<x<<" "

/// @brief This macro is like PRINT but enquotes the output in single quotes, e.g. `std::cout<<PRINT(name)<<PRINT(i)<<std::endl;` outputs "name: 'Dave' i: '42'"
#define PRINTQ(x) #x<<": '"<<x<<"' "

/// @brief STK_TRACE is another macro for debugging and will print informations about the file, line and function it is in. Uses std::cout.
/// Example output:
/// `###TRACE ..\..\..\stk\src\tests.cpp:94	person::person(const string&)`
#define STK_TRACE std::cout<<"###TRACE "<<__FILE__<<":"<<__LINE__<<"\t"<<__PRETTY_FUNCTION__<<std::endl;

/// @brief STK_TRACE_CERR is another macro for debugging and will print informations about the file, line and function it is in. Uses std::cerr.
/// Example output:
/// `###TRACE ..\..\..\stk\src\tests.cpp:94	person::person(const string&)`
#define STK_TRACE_CERR std::cerr<<"###TRACE "<<__FILE__<<":"<<__LINE__<<"\t"<<__PRETTY_FUNCTION__<<std::endl;

/// @brief Tired of writing loops like `for(int i=0;i<10;i++)`?? Use `STK_LOOP(10)`!! The counting variable is called `loop` and is an unsigned int.
#define STK_LOOP(x) for(unsigned int loop=0;loop<x;loop++)

#endif // STK_DEBUGGING_H
