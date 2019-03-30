#pragma once


namespace grlx
{


namespace fsm
{

    struct Any{};
    struct None{};
    struct FsmInit{};


    struct HandleStatus
    {
        enum Type
        {
            SUCCESS = 0,
            FAILURE,
            GUARDREJECT,
            DEFERRED
        };
    };

}
}

