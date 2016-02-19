#ifndef LFGUI_SIGNAL_H
#define LFGUI_SIGNAL_H

namespace lfgui
{

/// \brief Used by signal to store std::functions.
template <typename T>
struct event_function
{
    std::function<void()> void_void;
    std::function<void(T)> void_em;
    std::function<void(T,bool&)> bool_em;

    event_function(){}
    event_function(std::function<void()> f) : void_void(f) {}
    event_function(std::function<void(T)> f) : void_em(f) {}
    event_function(std::function<void(T,bool&)> f) : bool_em(f) {}
    /// \brief Returns true if this even_function has a std::function set.
    operator bool()const{return void_void||void_em||bool_em;}
};

/// \brief This class is the signal slot system of LFGUI. Functions can be set to be called by using the = or () operator.
/// The functions are called by using the call() command. Functions can have an optional argument that is given by the
/// T template parameter. Functions have a priority, lower priority numbers are called first. (default priority is 0)
template <typename T>
class signal
{
public:
    std::multimap<int,event_function<T>> functions;

    signal(){}
    signal(const event_function<T>& f)
    {
        functions.emplace(0,f);
    }
    signal(int priority,const event_function<T>& f)
    {
        functions.emplace(priority,f);
    }
    template<typename T2>
    signal(const T2& f)
    {
        functions.emplace(0,event_function<T>(f));
    }
    template<typename T2>
    signal(int priority,const T2& f)
    {
        functions.emplace(priority,event_function<T>(f));
    }

    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(const event_function<T>& f){functions.emplace(0,f);}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(int priority,const event_function<T>& f){functions.emplace(priority,f);}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(std::function<void()> f){functions.emplace(0,event_function<T>(f));}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(std::function<void(T)> f){functions.emplace(0,event_function<T>(f));}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(std::function<void(T,bool&)> f){functions.emplace(0,event_function<T>(f));}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(int priority,std::function<void()> f){functions.emplace(priority,event_function<T>(f));}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(int priority,std::function<void(T)> f){functions.emplace(priority,event_function<T>(f));}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(int priority,std::function<void(T,bool&)> f){functions.emplace(priority,event_function<T>(f));}
    template <typename T2>
    void operator=(const T2& o){(*this)(o);}

    /// \brief Returns true if the signal has any callbacks set.
    operator bool() const
    {
        return !functions.empty();
    }

    /// \brief Calls each appended function/lambda.
    bool call(const T& me)
    {
        for(auto& e:functions)
        {
            event_function<T>& s=e.second;
            bool stop;
            if(s.bool_em)
                s.bool_em(me,stop);
            else if(s.void_em)
                s.void_em(me);
            else if(s.void_void)
                s.void_void();
            if(stop)
                return true;
        }
        return !functions.empty();
    }
};

}   // namespace lfgui

#endif // LFGUI_SIGNAL_H
