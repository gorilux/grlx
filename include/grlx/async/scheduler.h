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

#include <iostream>
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

        template<typename F>
        Task(F&& f)
            : action(std::move(f))
        {}

        void exec()
        {
            action();
        }
        void checkAndReschedule()
        {
            std::lock_guard sync(lock);
            if(rescheduleAction)
            {
                rescheduleAction();
            }

        }
        void cancel()
        {
            std::lock_guard sync(lock);
            rescheduleAction = [](){};
        }

    private:
        friend class Scheduler;
        std::mutex lock;
        std::function<void()> action;
        std::function<void()> rescheduleAction;

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
                if(!tasks.empty())
                {
                    processTaskEvent.wait();
                }
                else
                {
                    auto timeOfFirstTask = (*tasks.begin()).first;
                    processTaskEvent.wait_until(timeOfFirstTask);
                }
                processElapsedTasks();
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
    }


    template<typename F, typename ...ArgsT>
    auto every(Clock::duration time, F &&f, ArgsT&&... args)
    {
        auto task = std::make_shared<Task>( std::bind(std::forward<F>(f), std::forward<ArgsT>(args)...) );

        task->rescheduleAction = [task, time, this]()
        {
            this->addTask(Clock::now() + time, task);
        };

        this->addTask(Clock::now() + time, task);

        return task;
    }

    template<typename F, typename ...ArgsT>
    auto in(Clock::time_point time, F &&f, ArgsT&&... args)
    {
         auto task = std::make_shared<Task>( std::bind(std::forward<F>(f), std::forward<ArgsT>(args)...) );
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
        auto task = std::make_shared<Task>( std::bind(std::forward<F>(f), std::forward<ArgsT>(args)...) );
//        addTask(time, task);
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
//            auto task = std::make_shared<Task>( std::bind(std::forward<F>(f), std::forward<ArgsT>(args)...) );

//            task->rescheduleAction = [task, tp, this]()
//            {
//                // if we've already passed this time, the user will mean next day, so add a day.
//                if(Clock::now() >= tp)
//                {
//                    tp += std::chrono::hours(24);
//                }

//                this->addTask(tp, task);
//            };

//            this->addTask(tp, task);

//            return task;
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

        task->cancel();

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

        for(auto itr = tasks.begin(); itr != tasks.end(); )
        {
            if(itr->first <= Clock::now())
            {
                auto task = itr->second;
                threadPool.schedule([task]()
                {

                    task->exec();
                    task->checkAndReschedule();

                });
                itr = tasks.erase(itr);
            }


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
