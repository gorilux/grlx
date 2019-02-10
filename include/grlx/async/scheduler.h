#pragma once

////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2015 David Salvador Pinheiro
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is David Salvador Pinheiro
///
/// @author David Salvador Pinheiro
/// @author Copyright 2015, David Salvador Pinheiro
////////////////////////////////////////////////////////////////////////////////


#include <chrono>
#include <memory>
#include <map>
#include <mutex>
#include <atomic>
#include <iomanip>
#include <exception>
#include <iterator>
#include <grlx/utility/cron.h>

#include "resetevent.h"
#include "threadpool.h"

namespace grlx
{

namespace async
{

using Clock = std::chrono::system_clock;



class Scheduler
{

public:    

    class Task
    {
    public:

        Task(std::function<void()>&& f)
            : action(std::move(f))
        {}
        virtual ~Task(){}

        virtual bool recurrent() = 0;
        virtual bool interval() = 0;

        virtual Clock::time_point getNextTime() = 0;

        void exec()
        {
            action();
        }



    private:
        friend class Scheduler;
        std::mutex lock;
        std::function<void()> action;
    };

    class CronTask : public Task
    {
    public:
        template<typename F>
        CronTask(std::string const& expression, F&& f)
            : Task(std::move(f))
            , cronexpr(grlx::cron::make_cron(std::string_view(expression)))
        {

        }

        bool recurrent() override
        {
            return true;
        }
        bool interval() override
        {
            return true;
        }
        Clock::time_point getNextTime() override
        {            
            return grlx::cron::cronToNext<Clock>(cronexpr, Clock::now());
        }
    private:
        grlx::cron::cronexpr cronexpr;
    };
    class EveryTask : public Task
    {
    public:
        template<typename F>
        EveryTask(Clock::duration time, F&& f, bool interval = true)
            : Task(std::move(f))
            , time(time)
            , interval_(interval)
        {

        }

        bool recurrent() override
        {
            return true;
        }
        bool interval() override
        {
            return interval_;
        }
        Clock::time_point getNextTime() override
        {
            return Clock::now() + time;
        }
    private:
        Clock::duration time;
        bool interval_;

    };
    class InTask : public Task
    {
    public:
        template<typename F>
        InTask(F&& f)
            : Task(std::move(f))
        {

        }

        bool recurrent() override
        {
            return false;
        }
        bool interval() override
        {
            return false;
        }
        Clock::time_point getNextTime() override
        {
            return Clock::time_point(Clock::duration(0));
        }

    };
    using TaskCollection = std::multimap<Clock::time_point, std::shared_ptr<Task>> ;
    using Iterator = TaskCollection::iterator;

    Scheduler( std::size_t threadCount = std::thread::hardware_concurrency() )
        : threadPool( threadCount + 1),
          processTaskEvent(true)

    {

        threadPool.schedule([this]()
        {
            while(!done)
            {
                if(tasks.empty())
                {
                    processTaskEvent.wait();
                }
                else
                {
                    auto timeOfFirstTask = (*tasks.begin()).first;
                    processTaskEvent.wait_until(timeOfFirstTask);
                }

                if(!done){
                    processElapsedTasks();
                }
            }
        });

    }

    Scheduler(const Scheduler &) = delete;

    Scheduler(Scheduler &&) noexcept = delete;

    Scheduler &operator=(const Scheduler &) = delete;

    Scheduler &operator=(Scheduler &&) noexcept = delete;

    ~Scheduler()
    {
        done = true;
        processTaskEvent.set();
    }


    template<typename F, typename ...ArgsT>
    auto every(Clock::duration time, F &&f, ArgsT&&... args)
    {
        auto task = std::make_shared<EveryTask>(time, std::bind(std::forward<F>(f), std::forward<ArgsT>(args)...) );

        this->addTask(task->getNextTime(), task);

        return task;
    }

    template<typename F, typename ...ArgsT>
    auto in(Clock::time_point time, F &&f, ArgsT&&... args)
    {
         auto task = std::make_shared<InTask>( std::bind(std::forward<F>(f), std::forward<ArgsT>(args)...) );
         addTask(time, task);
         return task;
    }

    template<typename F, typename ...ArgsT>
    auto in(Clock::duration time, F &&f, ArgsT&&... args)
    {
        return in(Clock::now() + time, std::forward<F>(f), std::forward<ArgsT>(args)...);
    }

    template<typename F, typename ...ArgsT>
    auto cron(std::string const& cronExpr, F &&f, ArgsT&&... args)
    {
        auto task = std::make_shared<CronTask>(cronExpr, std::bind(std::forward<F>(f), std::forward<ArgsT>(args)...) );

        this->addTask(task->getNextTime(), task);

        return task;
    }

    template<typename F, typename ...ArgsT>
    auto at(std::string const& time, F &&f, ArgsT&&... args)
    {

        auto tryParse = [](std::tm& tm, const std::string &expression, const std::string &format) -> bool
        {
            std::stringstream ss(expression);
            bool parseOk =(ss >> std::get_time(&tm, format.c_str())).fail();
            return parseOk;
        };

        // get current time as a tm object
        auto time_now = Clock::to_time_t(Clock::now());
        std::tm tm = *std::localtime(&time_now);

        // our final time as a time_point
        Clock::time_point tp;

        if(tryParse(tm, time, "%H:%M:%S"))
        {
            // convert tm back to time_t, then to a time_point and assign to final
            tp = Clock::from_time_t(std::mktime(&tm));

            // if we've already passed this time, the user will mean next day, so add a day.
            if(Clock::now() >= tp)
            {
                tp += std::chrono::hours(24);
            }
        }
        else if(tryParse(tm, time, "%Y-%m-%d %H:%M:%S"))
        {
            tp = Clock::from_time_t(std::mktime(&tm));
        }
        else if(tryParse(tm, time, "%Y/%m/%d %H:%M:%S"))
        {
            tp = Clock::from_time_t(std::mktime(&tm));
        }
        else
        {
            // could not parse time
            throw std::runtime_error("Cannot parse time string: " + time);
        }

        return in(tp, std::forward<F>(f), std::forward<ArgsT>(args)...);
    }


    void remove( std::shared_ptr<Task> task)
    {
        std::lock_guard sync(lock);

        for(auto itr = tasks.begin(), endItr = tasks.end(); itr != endItr;)
        {
            if(itr->second== task)
            {
                itr = tasks.erase(itr);
            }
            else
            {
                ++itr;
            }
        }
    }

private:

    void addTask(Clock::time_point time, std::shared_ptr<Task> t)
    {
        {
            std::lock_guard sync(lock);
            tasks.emplace(time, std::move(t));
        }
        processTaskEvent.set();
    }

    void processElapsedTasks()
    {
        std::lock_guard sync(lock);

        decltype (tasks) recurringTasks;

        for(auto itr = tasks.begin(); itr != tasks.end(); )
        {
            if(itr->first <= Clock::now())
            {
                auto task = itr->second;
                threadPool.schedule([task]()
                {
                    task->exec();                    
                });                

                if(task->recurrent())
                {
                    recurringTasks.emplace(task->getNextTime(), std::move(task));
                }

                itr = tasks.erase(itr);
            }
            else
            {
                itr++;
            }
        }

        if(!recurringTasks.empty())
        {
            tasks.insert(recurringTasks.begin(), recurringTasks.end());
        }


    }
private:
    FifoThreadPool threadPool;
    ResetEvent processTaskEvent;
    std::multimap<Clock::time_point, std::shared_ptr<Task>> tasks;
    std::mutex lock;
    std::atomic<bool> done = false;

};

}
}
