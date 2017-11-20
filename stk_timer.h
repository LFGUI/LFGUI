/*
 * Copyright (C) 2012-2016 damu/gawag
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
*/

#ifndef STK_TIMER_H
#define STK_TIMER_H

#include <iostream>
#include <vector>
#include <chrono>

/// \file
/// \brief Various timer classes to measure real time, used processor time and used cycles.

namespace stk
{

/// \brief The timer class can be used to measure times and optionaly output them automatically on destruction.
/// It supports multiple measurepoints and a sumarized output at the end.
///
/// Example:
/// \code
/// {
///     stk::timer t("test 1");
///     sleep(0.1);
///     cout<<"until now: "<<t.until_now()<<endl;
///     sleep(0.1);
/// }                        // the timer is destructed here as it goes out of scope and prints something like "0.100132 <- test"
/// {
///     stk::timer time("test 2");
///     sleep(0.1);
///     time.add("first");
///     sleep(0.2);
///     time.add("second");
///     sleep(0.3);
/// }
/// \endcode
/// Output:
/// \code
/// until now: 0.100005
/// 0.200011 	<- test 1
/// 0.599034 	<- test 2
///   0.100005 	<- 0 first
///   0.200011 	<- 1 second
///   0.299017 to end
/// \endcode
class timer
{
    class measure_point
    {
    public:
        std::string text;
        std::chrono::steady_clock::time_point start;
        measure_point(std::string text) : text(text),start(std::chrono::steady_clock::now()) {}
    };

    std::string text;
    std::vector<measure_point> points;
    bool output;            ///< if the timer should print something on destruction
public:
    std::chrono::steady_clock::time_point start;
    /// \brief The text is the general name for this timer. Setting output to false disables printing anything on destruction.
    timer(std::string text="",bool output=true) : text(text),output(output) {start=std::chrono::steady_clock::now();}
    /// \brief The text is the general name for this timer. Setting output to false disables printing anything on destruction.
    timer(const char* text,   bool output=true) : text(text),output(output) {start=std::chrono::steady_clock::now();}
    timer(const timer&)=default;
    timer(timer&&)=default;
    timer& operator=(const timer&)=default;
    timer& operator=(timer&&)=default;

    ~timer()
    {
        if(!output)
            return;
        auto start=this->start;
        auto diff=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count()/1000000.0;
        if(text.size())
            std::cout<<diff<<" \t<- "<<text<<std::endl;
        else
            std::cout<<"timer time: "<<diff<<std::endl;

        if(points.empty())
            return;

        for(auto p:points)
        {
            auto diff=std::chrono::duration_cast<std::chrono::microseconds>(p.start-start).count()/1000000.0;
            std::cout<<"  "<<diff<<" \t<- "<<p.text<<std::endl;
            start=p.start;
        }
        diff=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count()/1000000.0;
        std::cout<<"  "<<diff<<" to end"<<std::endl;
    }

    /// \brief Returns the time passed since the timer was started or reset in seconds.
    double until_now() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count()/1000000.0;
    }

    void add(const std::string& name="")
    {
        if(!name.size())
            points.emplace_back(std::to_string(points.size()));
        else
            points.emplace_back(std::to_string(points.size())+" "+name);
    }

    /// \brief Resets the starting time to the current time. Measure points added before this will display a negative time in the summary.
    void reset(){start=std::chrono::steady_clock::now();}
};

/**
 * @brief Same as timer but with the clock() function (measures only the time consumed by the program).
 */
class timer_clock
{
    class measure_point
    {
    public:
        std::string text;
        clock_t tv;
        measure_point(std::string text) : text(text),tv(::clock()) {}
    };

    std::string text;
    std::vector<measure_point> points;
    clock_t tv;
    bool output;            ///< if the timer should print something on destruction
public:
    /// \brief The text is the general name for this timer. Setting output to false disables printing anything on destruction.
    timer_clock(std::string text="",bool output=true) : text(text),tv(::clock()),output(output) {}
    /// \brief The text is the general name for this timer. Setting output to false disables printing anything on destruction.
    timer_clock(const char* text,   bool output=true) : text(text),tv(::clock()),output(output) {}
    timer_clock(const timer_clock&)=default;
    timer_clock(timer_clock&&)=default;
    timer_clock& operator=(const timer_clock&)=default;
    timer_clock& operator=(timer_clock&&)=default;

    ~timer_clock()
    {
        if(!output)
            return;
        clock_t start=tv;
        clock_t end=::clock();
        if(text.size())
            std::cout<<end-tv<<" \t<- "<<text<<std::endl;
        else
            std::cout<<"timer time: "<<end-tv<<std::endl;

        if(points.empty())
            return;

        for(auto p:points)
        {
            std::cout<<"  "<<p.tv-start<<" \t<- "<<p.text<<std::endl;
            start=p.tv;
        }
        std::cout<<"  "<<end-start<<" to end"<<std::endl;
    }

    /// \brief Returns the time passed since the timer was started or reset in seconds.
    double until_now() const
    {
        clock_t end=::clock();
        return end-tv;
    }

    void add(const std::string& name="")
    {
        if(!name.size())
            points.emplace_back(std::to_string(points.size()));
        else
            points.emplace_back(std::to_string(points.size())+" "+name);
    }

    /// \brief Resets the starting time to the current time. Measure points added before this will display a negative time in the summary.
    void reset(){tv=::clock();}
};

/// \brief This is a class to hierarchically profile code with manual set points in code. It uses the stk::timer class to measure times.
///
/// Example:
/// \code
/// {
///     stk::profiler::measure_point _("test1");         // points can be given names
///     sleep(0.1);
///     {
///         stk::profiler::measure_point _("test2");     // this point will be indent to show that it's a su-point of test1
///         sleep(0.1);
///         {
///             stk::profiler::measure_point _("test3");
///             sleep(0.1);
///         }
///     }
///     {
///         stk::profiler::measure_point _("test4");
///         sleep(0.1);
///     }
/// }                                           // the test1 point measures the time until it's destruction here
/// {
///     STK_PROFILER                            // there is also a macro that will autogenerate names with the file
///     sleep(0.1);                             // name, line number and function name of its position, like ..\src\tests.cpp:107 main
///     {
///         STK_PROFILER
///         sleep(0.1);
///         {
///             STK_PROFILER
///             sleep(0.1);
///         }
///     }
///     {
///         STK_PROFILER
///         sleep(0.1);
///     }
/// }
/// \code
/// Output:
/// \code
/// 0.400022 	test1
/// 0.200011 	  test2
/// 0.100005 	    test3
/// 0.100005 	  test4
/// 0.400022 	..\src\tests.cpp:107 main
/// 0.200011 	  ..\src\tests.cpp:110 main
/// 0.100005 	    ..\src\tests.cpp:113 main
/// 0.100005 	  ..\src\tests.cpp:118 main
/// \endcode
class profiler
{
public:
    class measure_point
    {
    public:
        std::string name;
        stk::timer t=stk::timer("",false);

        measure_point(const std::string& name) : name(name)
        {
            profiler::instance().open_points.push_back(this);
            std::string str;
            for(size_t i=0;i<profiler::instance().open_points.size()-1;i++)
                str=str+"  ";
            str=str+profiler::instance().open_points.back()->name;
            profiler::instance().find_or_create_time(str);
        }
        measure_point(const std::string& name,profiler*) : name(name)
        {
        }
        ~measure_point()
        {
            if(name=="STK Profiler")    // crashing weirdly
                return;

            std::string str;
            for(size_t i=0;i<profiler::instance().open_points.size()-1;i++)
                str=str+"    ";
            str=str+profiler::instance().open_points.back()->name;
            profiler::instance().find_or_create_time(str)+=t.until_now();
            profiler::instance().open_points.resize(profiler::instance().open_points.size()-1);
        }
    };
private:
    profiler() : main_point("STK Profiler",this) {}
    measure_point main_point;
public:
    std::vector<measure_point*> open_points;
    std::vector<std::pair<std::string,double>> sub_point_times; // <name,time>

    static profiler& instance()
    {
        static profiler p;
        return p;
    }
    ~profiler()
    {
        std::cout<<"Profiler: "<<std::endl;
        for(auto e:sub_point_times)
            std::cout<<e.second<<" \t"<<e.first<<std::endl;
    }

    double& find_or_create_time(const std::string& name)
    {
        for(std::pair<std::string,double>& e:sub_point_times)
            if(e.first==name)
                return e.second;
        sub_point_times.emplace_back(name,0);
        return sub_point_times.back().second;
    }

    static std::string to_string()
    {
        std::string ret;
        ret.append("Profiler: \n");
        for(auto e:instance().sub_point_times)
        {
            if(e.second<=0.0)
                continue;
            ret.append(std::to_string(e.second));
            ret.append(" \t");
            ret.append(e.first);
            ret.append("\n");
        }
        return ret;
    }

    static void clear()
    {
        instance().sub_point_times.clear();
    }
};

//#define STK_PROFILER stk::profiler_point STK_PROFILER_POINT(std::string(__FILE__)+":"+std::to_string(__LINE__)+" "+__PRETTY_FUNCTION__);
#define STK_PROFILER stk::profiler::measure_point __STK_PROFILER_POINT__(std::string(__FILE__)+":"+std::to_string(__LINE__)+" "+__FUNCTION__);
//#define STK_PROFILER stk::profiler_point STK_PROFILER_POINT(std::string(__FILE__)+":"+std::to_string(__LINE__));

#define STK_PROFILER_POINT(X) stk::profiler::measure_point __STK_PROFILER_POINT__(X);

}       // namespace STK

#endif  // STK_TIMER_H
