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

#ifndef STK_MISC_H
#define STK_MISC_H

#include <setjmp.h>
#include <string>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <functional>
#include <vector>
#include <map>
#include <mutex>
#include <set>
#include <memory>
#include <sys/time.h>
#include <array>
#include <type_traits>
#include <fstream>
#include <cinttypes>
#include <locale>
#include <csignal>

/// \file
/// \brief Various snippets too small to get own files.

#if __cplusplus>201402L
 #define constex14 constexpr
#else
 #define constex14
#endif

namespace stk
{

template<typename T,bool B=std::is_arithmetic<T>::value>
struct init_arithmetic_with_0
{
    T value;
    constexpr init_arithmetic_with_0():value(0){}
    constexpr init_arithmetic_with_0(const init_arithmetic_with_0&)=default;
    constexpr init_arithmetic_with_0(init_arithmetic_with_0&&)=default;
    constex14 init_arithmetic_with_0& operator=(const init_arithmetic_with_0&)=default;
    constex14 init_arithmetic_with_0& operator=(init_arithmetic_with_0&&)=default;
    constexpr init_arithmetic_with_0(const T& t):value(t){}
    constexpr init_arithmetic_with_0(T&& t):value(t){}
    constex14 init_arithmetic_with_0& operator=(const T& t){value=t;return *this;}
    constexpr init_arithmetic_with_0& operator=(T&& t){value=t;return *this;}
    constexpr operator T()const{return value;}
    constex14 operator T&(){return value;}
};
template<typename T>
struct init_arithmetic_with_0<T,false>
{
    T value;
    constexpr init_arithmetic_with_0(){}
    constexpr init_arithmetic_with_0(const init_arithmetic_with_0&)=default;
    constexpr init_arithmetic_with_0(init_arithmetic_with_0&&)=default;
    constex14 init_arithmetic_with_0& operator=(const init_arithmetic_with_0&)=default;
    constex14 init_arithmetic_with_0& operator=(init_arithmetic_with_0&&)=default;
    constexpr init_arithmetic_with_0(const T& t):value(t){}
    constexpr init_arithmetic_with_0(T&& t):value(t){}
    constex14 init_arithmetic_with_0& operator=(const T& t){value=t;return *this;}
    constexpr init_arithmetic_with_0& operator=(T&& t){value=t;return *this;}
    constexpr operator T()const{return value;}
    constex14 operator T&(){return value;}
};

/**
 * \brief This class can be used to wrap a value in it, and use it either like a value (like a=42;) or like a function (a(42);).
 * The values in this one are automatically initialized with 0, if possible.
 */
template<typename T>
class functor
{
public:
    init_arithmetic_with_0<T> value;

    constexpr functor()=default;
    constexpr functor(T v) : value(v){}
    constex14 void operator()(T v) {value=v;}
    constexpr operator T() const {return value;}
    constexpr T operator()() const {return value;}
    constex14 functor& operator =(const T& v){value =v;return *this;}
    constex14 functor& operator+=(const T& v){value+=v;return *this;}
    constex14 functor& operator-=(const T& v){value-=v;return *this;}
};

/**
 * \brief This class can be used to wrap a value in it, and use it either like a value (like a=42;) or like a function (a(42);).
 * This one does not try to automatically initialize with 0 and has therefore less constraints on the wrapped type.
 */
template<typename T>
struct functor_simple
{
    T value;

    constex14 void operator()(T v) {value=v;}
    constexpr operator T() const {return value;}
    constexpr T operator()() const {return value;}
    constex14 T& operator()() {return value;}
    constex14 functor_simple& operator=(const T& v){value=v;return *this;}
    constex14 T operator +=(const T& v)  {value+=v;return *this;}
    constex14 T operator -=(const T& v)  {value-=v;return *this;}
    constex14 T operator *=(const T& v)  {value*=v;return *this;}
};

/**
 * \brief This class can be used to wrap a value in it, and use it either like a value (like a=42;) or like a function (a(42);).
 * The values in this one are automatically initialized with 0, if possible.
 * In this variant a notifier lambda can be given which will be called when the value is changed.
 */
template<typename T>
struct functor_with_notifier
{
private:
    std::function<void(T)> notifier;
public:
    init_arithmetic_with_0<T> value;

    functor_with_notifier(std::function<void(T)> notifier) : notifier(notifier){}
    functor_with_notifier(T v,std::function<void(T)> notifier) : notifier(notifier),value(v){}
    constexpr functor_with_notifier(T v) : value(v){}

    void operator()(const T& v) {value=v;notifier(value);}
    functor_with_notifier& operator=(const T& v){value=v;notifier(value);return *this;}
    constexpr operator T() const {return value;}
    constexpr T operator()() const {return value;}
    //T operator +=(const T& v)  {value+=v;return *this;}
    //T operator -=(const T& v)  {value+=v;return *this;}
};

struct stacktrace
{
    struct entry
    {
        const char* file;
        int line;
        const char* function;

        entry(const char* file,int line,const char* function):file(file),line(line),function(function)
        {
            stacktrace::instance().entries.push_back(this);
        }

        ~entry()
        {
            stacktrace::instance().entries.pop_back();
        }

        void describe(std::string& ret)
        {
            ret.append(file);
            ret.append(":");
            ret.append(std::to_string(line));
            ret.append(": ");
            ret.append(function);
            ret.append("\n");
        }
    };

    std::vector<entry*> entries;
    std::function<void(int,const char*)> signal_handler=[](int,const char* signal_name)
    {
        stacktrace::instance().print(signal_name);
        exit(-1);
    };

    static stacktrace& instance()
    {
        static stacktrace st;
        return st;
    }

    void print(const char* signal_name="")
    {
        std::cerr<<describe(signal_name)<<std::endl;
    }

    std::string describe(const char* signal_name=0)
    {
        std::string ret;
        if(signal_name)
        {
            ret.append("ERROR: received signal ");
            ret.append(signal_name);
            ret.append("\n");
        }
        if(!entries.size())
        {
            ret.append("Stacktrace is empty\n");
            return ret;
        }
        ret.append("Printing stacktrace with size ");
        ret.append(std::to_string(entries.size()));
        ret.append(":\n");
        for(int i=entries.size()-1;i>=0;i--)
        {
            ret.append(std::to_string(i));
            ret.append(": ");
            entries[i]->describe(ret);
        }
        return ret;
    }

    static void signal_handler_wrapper(int signal)
    {
        static const char* signal_names[]=
            {
                "UNKNOWN",          // 0    these names are from signal.h
                "SIGHUP",           // 1
                "SIGINT",           // 2
                "SIGQUIT",          // 3
                "SIGILL",           // 4
                "SIGTRAP",          // 5
                "SIGABRT_COMPAT",   // 6
                "SIGEMT",           // 7
                "SIGFPE",           // 8
                "SIGKILL",          // 9
                "SIGBUS",           // 10
                "SIGSEGV",          // 11
                "SIGSYS",           // 12
                "SIGPIPE",          // 13
                "SIGALRM",          // 14
                "SIGTERM",          // 15
                "UNKNOWN",          // 16
                "UNKNOWN",          // 17
                "UNKNOWN",          // 18
                "UNKNOWN",          // 19
                "UNKNOWN",          // 20
                "SIGBREAK",         // 21
                "SIGABRT",          // 22
            };
        if(signal>(int)sizeof(signal_names))
            signal=0;

        instance().signal_handler(signal,signal_names[signal]);
    }

    stacktrace(stacktrace&&)=delete;
    stacktrace(const stacktrace&)=delete;
    stacktrace& operator=(stacktrace&&)=delete;
    stacktrace& operator=(const stacktrace&)=delete;

    void subscribe()
    {
        std::signal(SIGABRT         ,signal_handler_wrapper);
#ifdef SIGABRT2
        std::signal(SIGABRT2        ,signal_handler_wrapper);
#endif
#ifdef SIGABRT_COMPAT
        std::signal(SIGABRT_COMPAT  ,signal_handler_wrapper);
#endif
#ifdef SIGBREAK
        std::signal(SIGBREAK        ,signal_handler_wrapper);
#endif
        std::signal(SIGFPE          ,signal_handler_wrapper);
        std::signal(SIGILL          ,signal_handler_wrapper);
        std::signal(SIGINT          ,signal_handler_wrapper);
        std::signal(SIGSEGV         ,signal_handler_wrapper);
        std::signal(SIGTERM         ,signal_handler_wrapper);
    }

private:
    stacktrace()
    {
        entries.reserve(50);

        subscribe();
    }
};

struct assert_handler
{
    static jmp_buf& handler()
    {
        static jmp_buf h;
        return h;
    }
    static void jmp_or_default()
    {
        bool jmp_buf_is_set=false;
#if defined(__gnu_linux__)
        jmp_buf_is_set=stk::assert_handler::handler()->__jmpbuf[0];
#else
        // I don't know if this works on all platforms, but in MinGW this seems to be an int64_t[2]
 #ifdef __x86_64__
        jmp_buf_is_set=stk::assert_handler::handler()->Part[0];
 #else
        jmp_buf_is_set=stk::assert_handler::handler()[0];
 #endif
#endif
        if(!jmp_buf_is_set)
        {
            std::cerr<<stk::assert_handler::text()<<std::endl<<std::flush;
            exit(-1);
        }
        else
            longjmp(stk::assert_handler::handler(),1);
    }
    static std::string& text()
    {
        static std::string s;
        return s;
    }
};

/// @brief stk::exception is deriving from std::exception and it's constructor takes a string as parameter to give a textual error message.
class exception : public std::exception
{
public:
    std::string str;
    exception(std::string str) : str(str)
    {
        str.append("\n");
        str.append(stk::stacktrace::instance().describe());
        str.append("\n");

        std::cerr<<str<<std::endl;
        stk::assert_handler::text()=str;
        stk::assert_handler::jmp_or_default();
    }
    virtual ~exception() {}
    virtual const char* what() const noexcept override {return str.c_str();}
};

/** \brief Converts dates into timestamps. Can handle "2014-12-31", "14-12-31", "31.12.2014", "31.12.14", "12/31/2014" and "12/31/14". Years with two numbers are interpreted as 1970 to 2069.
 * \code
 * Code:
 *   cout<<PRINT(stk::timestamp_conversion("01.01.1970"))<<endl;
 *   cout<<PRINT(stk::timestamp_conversion("11.02.14"))<<endl;
 *   cout<<PRINT(stk::timestamp_conversion("14-02-11"))<<endl;
 *   cout<<PRINT(stk::timestamp_conversion("02/11/14"))<<endl;
 *   cout<<PRINT(stk::timestamp_conversion("11.02.2014"))<<endl;
 *   cout<<PRINT(stk::timestamp_conversion("2014-02-11"))<<endl;
 *   cout<<PRINT(stk::timestamp_conversion("02/11/2014"))<<endl;
 *   cout<<PRINT(stk::timestamp_conversion())<<endl;
 *   cout<<PRINT(stk::timestamp_conversion(1392076800))<<endl;
 *   cout<<PRINT(stk::timestamp_conversion(stk::timestamp_conversion("2014-02-11")))<<endl;
 * Output:
 *   stk::timestamp_conversion("01.01.1970"): 0
 *   stk::timestamp_conversion("11.02.14"): 1392076800
 *   stk::timestamp_conversion("14-02-11"): 1392076800
 *   stk::timestamp_conversion("02/11/14"): 1392076800
 *   stk::timestamp_conversion("11.02.2014"): 1392076800
 *   stk::timestamp_conversion("2014-02-11"): 1392076800
 *   stk::timestamp_conversion("02/11/2014"): 1392076800
 *   stk::timestamp_conversion(): 2014-02-11
 *   stk::timestamp_conversion(1392076800): 2014-02-11
 *   stk::timestamp_conversion(stk::timestamp_conversion("2014-02-11")): 2014-02-11
 * \endcode
 **/
long timestamp_conversion(const std::string& str);
/// @brief Converts `timestamp` into dates. `format` can look like "%a %Y-%m-%d %H:%M:%S %Z" which would result in "ddd yyyy-mm-dd hh:mm:ss zzz".
std::string timestamp_conversion(const long& timestamp=0,const std::string& format="%Y-%m-%d");

/// @brief The `std::make_unique` function from C++14 is still missing (they forgot it in C++11).
template<typename T,typename... Args>
constexpr std::unique_ptr<T> make_unique(Args... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/// \brief Turns a pointer into a std::unique_ptr<T>.
template<typename T>
constexpr std::unique_ptr<T> make_unique_from_ptr(T* t)
{
    return std::unique_ptr<T>(t);
}

/// \brief There's no std::make_pair??? Ah come on!
template<typename T1,typename T2>
constexpr std::pair<T1,T2> make_pair(const T1& t1,const T2& t2)
{
    return std::pair<T1,T2>(t1,t2);
}

/// \brief This class takes a lambda and calls it in its destructor. This is thought to be used in a RAII manner.
class lambda_on_destruct
{
public:
    std::function<void()> lambda;
    lambda_on_destruct(std::function<void()> lambda) : lambda(lambda){}
    ~lambda_on_destruct()
    {
        lambda();
    }
};

/// \brief This class provides a thread-safe global storage for data. This can be used to set global flags, to control
/// parts of the application, that you can't reach by parameters.
/// Can be used to get flags around the Qt slot/signal thing.
template<typename Key=std::string,typename Value=bool,typename Storage=std::map<Key,Value>>
class global_storage
{
public:
    static Storage& data()
    {
        static Storage d;
        return d;
    }
    static std::mutex& lock()
    {
        static std::mutex d;
        return d;
    }

    static Value get(const Key& key,Value default_=Value())
    {
        std::lock_guard<std::mutex> _(lock());
        if(data().count(key))
            return data()[key];
        return default_;
    }

    static void set(Key k,Value v)
    {
        std::lock_guard<std::mutex> _(lock());
        data()[k]=v;
    }

    static void insert(Key k,Value v)
    {
        std::lock_guard<std::mutex> _(lock());
        data().emplace(k,v);
    }

    static void remove(Key k)
    {
        std::lock_guard<std::mutex> _(lock());
        data().erase(k);
    }
};

/// \brief Returned by read_dir.
struct direntry
{
    std::string filename;
    std::string filepath;  ///< filename with path
    bool is_dir;
};

/// \brief Reads the specified directory and returns its content in a std::vector. Uses dirent.h.
std::vector<direntry> read_dir(std::string dirname);

/// \brief Inserts an element into a vector if it's not already in there (operator==).
template<typename T>
constexpr void push_back_unique(std::vector<T>& vec,const T& v)
{
    for(auto e:vec)
        if(e==v)
            return;
    vec.push_back(v);
}

/// \brief Inserts an element into a vector if it's not already in there (operator==).
template<typename T>
constexpr typename std::vector<T>::iterator find(std::vector<T>& vec,const T& v)
{
    for(typename std::vector<T>::iterator it=vec.begin();it!=vec.end();it++)
        if(*it==v)
            return it;
    return vec.end();
}

/// Takes a void pointer and returns the data there as the given type. Like a dereference but without UB. Copies the data.
template<typename T>
constexpr T unaligned_cast(const void* const ptr)
{
    T ret;
    char* input =(char*)ptr;
    char* output=(char*)&ret.value;
    for(size_t i=0;i<sizeof(T);i++)
        output[i]=input[i];
    return ret;
}

} // namespace stk

#define STK_CONCAT_(x,y) x##y
#define STK_CONCAT(x,y) STK_CONCAT_(x,y)
#define STK_STACKTRACE stk::stacktrace::entry STK_CONCAT(_STK_STACKTRACE_ENTRY_NAME_,__LINE__)(__FILE__,__LINE__,__PRETTY_FUNCTION__);

inline void __stk_assert(bool condition,const char* file,int line,const char* function,const char* expression)
{
    if(!condition)
    {
        std::string str;
        str.append("assert \"");
        str.append(expression);
        str.append("\" failed at ");
        str.append(file);
        str.append(":");
        str.append(std::to_string(line));
        str.append(" in ");
        str.append(function);
        str.append("\n");
        str.append(stk::stacktrace::instance().describe());
        str.append("\n");

        stk::assert_handler::text()=str;
        stk::assert_handler::jmp_or_default();
    }
}
#define STK_STRINGIFY(X) #X
#define stk_assert(EXPRESSION) __stk_assert(EXPRESSION,__FILE__,__LINE__,__PRETTY_FUNCTION__,STK_STRINGIFY(EXPRESSION));

#endif // STK_MISC_H
