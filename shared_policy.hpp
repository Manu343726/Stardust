/***************************************************************************
*               Stardust, a C++11 particle engine generator                *
*               -------------------------------------------                *
*                                                                          *
*  Copyright 2014  Manuel Sánchez Pérez                                    *
*                                                                          *
* Licensed under the Apache License, Version 2.0 (the "License");          *
* you may not use this file except in compliance with the License.         *
* You may obtain a copy of the License at                                  *
*                                                                          *
*     http://www.apache.org/licenses/LICENSE-2.0                           *
*                                                                          *
* Unless required by applicable law or agreed to in writing, software      *
* distributed under the License is distributed on an "AS IS" BASIS,        *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
* See the License for the specific language governing permissions and      *
* limitations under the License.                                           *
***************************************************************************/

#ifndef SHARED_POLICY_HPP
#define	SHARED_POLICY_HPP

#include <memory>
#include <utility>

namespace sdst
{
    /*
     * The type sdst::shared_policy<P> encapsulates a std::shared_ptr<P> where P is a 
     * function entity that acts as a policy. This allows the library to perform 
     * static type-erasure and make shared policies behave like other policies, with 
     * exactly the same function-like interface.
     */
    template<typename POLICY>
    struct shared_policy
    {
    public:
        /*
         * Initializes a shared policy given an underlying policy.
         */
        template<typename... ARGS>
        shared_policy( ARGS&&... args ) :
            _ptr{ std::make_shared<POLICY>( std::forward<ARGS>( args )... ) }
        {}
            
        /*
         * Call operator for the shared policy. Calls the underlying policy.
         */
        template<typename... ARGS>
        auto operator()( ARGS&&... args ) -> decltype( (*_ptr)( std::forward<ARGS>( args )... ) )
        {
            return (*_ptr)( std::forward<ARGS>( args )... );
        }
        
        /*
         * (const) Call operator for the shared policy. Calls the underlying policy.
         */
        template<typename... ARGS>
        auto operator()( ARGS&&... args ) const -> decltype( (*_ptr)( std::forward<ARGS>( args )... ) )
        {
            return (*_ptr)( std::forward<ARGS>( args )... );
        }
            
    private:
        std::shared_ptr<POLICY> _ptr;
    };
    
    /*
     * Builder for shared policies.
     */
    template<typename POLICY  , typename... ARGS>
    sdst::shared_policy<POLICY> make_shared_policy( ARGS&&... args )
    {
        return { std::forward<ARGS>( args )... };
    }
    
    /*
     * Builder for shared policies (No ctor forwarding, but full type-deductive).
     */
    template<typename POLICY>
    sdst::shared_policy<typename std::decay<POLICY>::type> make_shared_policy( POLICY&& policy )
    {
        return { std::forward<POLICY>( policy ) };
    }
}

#endif	/* SHARED_POLICY_HPP */

