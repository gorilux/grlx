#pragma once

#include <memory>
#include <unordered_map>
#include <map>
#include <functional>
#include <mutex>
#include <chrono>
#include <list>

namespace grlx
{



template<typename TKey, typename TValue,  template<class> class TStrategy>
class Cache
{
public:
    using DataContainer = std::unordered_map<TKey, TValue >;
    using DataType      = typename DataContainer::value_type;


    template<typename ... TArgs>
    Cache(TArgs&& ...args)
        : strategy(std::forward<TArgs>(args)...)
    {

    }


    template<typename ...TArgs>
    TValue& getOrDefault(TKey const& key, std::function<TValue()> createDefault, TArgs&& ...args)
    {

        if(auto itr = data.find(key); itr != data.end())
        {
            if(strategy.isValid(key, std::forward<TArgs>(args)...))
            {
                return itr->second;
            }
        }

        auto[itr, assigned] = performPut(key, createDefault(), std::forward<TArgs>(args)...);

        return itr->second;
    }


    template<typename ...TArgs>
    void put(TKey const& key, TValue const& value, TArgs&& ...args)
    {
        performPut(key, value, std::forward<TArgs>(args)...);
    }

    void remove(TKey const& key)
    {
        strategy.remove(key);
        data.erase(key);
    }

    void clear()
    {
        strategy.clear();
        data.clear();
    }

    std::size_t size() const
    {
        return data.size();
    }

protected:

    using Itr = typename DataContainer::iterator;

    template<typename ...TArgs>
    auto performPut(TKey const& key, TValue const& value, TArgs&& ...args)
    {
        strategy.put(key, std::forward<TArgs>(args)...);
        return data.insert_or_assign(key,value);
    }

private:
    TStrategy<TKey> strategy;
    DataContainer data;

};

template<typename TKey>
class LRUCacheStrategy
{

public:
    using Keys     = std::list<TKey>;
    using KeyIndex = std::unordered_map<TKey, typename Keys::iterator>;

    LRUCacheStrategy(std::size_t size = 1024)
        : size(size)
    {

    }

    bool isValid(TKey const& key)
    {
        return (keyIndex.find(key) != keyIndex.end());
    }
    void put(TKey const& key)
    {

        if(auto itr = keyIndex.find(key); itr == keyIndex.end())
        {
            if(keys.size() == size)
            {
                auto& last = keys.back();
                keys.pop_back();
                keyIndex.erase(last);
            }
        }
        else
        {
            keys.erase(itr->second);
        }

        keys.push_front(key);
        keyIndex.insert_or_assign(key, keys.begin());

    }
    void remove(TKey const& key)
    {
        if(auto itr = keyIndex.find(key); itr != keyIndex.end())
        {
            keys.erase(itr->second);
            keyIndex.erase(itr);
        }
    }

    void clear()
    {
        keys.clear();
        keyIndex.clear();
    }



private:
    std::size_t size;
    Keys keys;
    KeyIndex keyIndex;
};

template<typename TKey>
class ExpireCacheStrategy
{
public:

    using Clock        = std::chrono::steady_clock;
    using ExpireIndex = std::unordered_map<TKey, Clock::time_point>;

    ExpireCacheStrategy(Clock::duration expireTime)
        : expireTime( expireTime )
    {

    }
    bool isValid(TKey const& key)
    {
        auto now = Clock::now();

        if(auto itr = expireIndex.find(key); itr != expireIndex.end())
        {
            if( now < itr->second )
            {
                // not expired
                return true;
            }
            else
            {
                //expired
                expireIndex.erase(itr);
                return false;
            }
        }
        else
        {
            return false;
        }

    }
    void put(TKey const& key)
    {
        auto now = Clock::now();
        expireIndex.insert_or_assign( key, now + expireTime );
    }
    void remove(TKey const& key)
    {
        expireIndex.erase(key);
    }

    void clear()
    {
        expireIndex.clear();
    }

private:
    ExpireIndex expireIndex;
    Clock::duration expireTime;

};

template<typename TKey, typename TValue>
using ExpireCache = Cache<TKey, TValue, ExpireCacheStrategy >;

template<typename TKey, typename TValue>
using LRUCache = Cache<TKey, TValue, LRUCacheStrategy >;


}
