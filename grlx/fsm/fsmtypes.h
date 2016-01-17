/* 	 
*    Copyright (c) 2011 David Salvador Pinheiro
*
*    http://www.gorilux.org 
*       
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.  
*/

#ifndef GRLX_FSM_TYPES_H
#define GRLX_FSM_TYPES_H


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

#endif // GRLX_FSM_TYPES_H
