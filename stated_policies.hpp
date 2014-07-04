/* 
 * File:   stated_policies.hpp
 * Author: manu343726
 *
 * Created on 4 de julio de 2014, 19:49
 */

#ifndef STATED_POLICIES_HPP
#define	STATED_POLICIES_HPP

#include "Turbo/enable_if.hpp"
#include "Turbo/type_traits.hpp"

/*
 * A policy could have an internal state that should be updated along the simulation.
 * Such stated evolution policies are updated via a function-like call specially designed for
 * that purpose. The type 'sdst::state_change' is an enumeration specifying the kind of state
 * update to be performed:
 *  - A local update is performed when a particle is updated, once per particle in each simulation frame.
 *  - A global update occurs once per simulation frame. 
 * 
 * Such different update events help to update correctly both policies shared between
 * different particles (Using the global update to avoid more than one update per frame), and normal
 * policies not shared at all (Using the local update).
 * 
 * Note that a local update is designed to be requested automatically via the 'update()' method of a particle,
 * and a simple particle engine could easily handle it traversing the set of particles and calling 'update()' 
 * for each one. A global update, on the other hand, can't be handled correctly using that pattern, and should 
 * be requested manually by the user or a more complex engine which takes care and stores policies shared between 
 * particles.
 * 
 * The signature of the update function-like call is:
 *     
 *     void(sdst::state_change)
 * 
 * Any policy where such signature is valid is regarded to have state and candidate for update. 
 * A 'sdst::erase_state<P>' template is provided to perform static type erasure and treat both types of policies 
 * (Stated and non-stated) in the same homogeneous way. 
 */

namespace sdst
{
    /*
     * Specifies the type of update to be performed on a stated policy.
     */
    enum class state_change
    {
        local, //Per particle
        global //Per simulation frame
    };
    
    /*
     * Specifies the signature of a policy update request function
     */
    using update_request_t = void(sdst::state_change);
    
    
    /*
     * Performs static type erasure to manage homogenoeusly both stated
     * and non-stated policies.
     */
    template<typename POLICY>
    struct erase_state
    {
    private:
        POLICY _policy; //The underlying policy
    public:
        /*
         * Initializes the policy bypassing the constructor argumments to the underlying
         * type-erased policy.
         */
        template<typename... ARGS>
        erase_state( ARGS&&... args ) : 
            _policy{ std::forward<ARGS>( args )... }
        {}
            
        /*
         * Call operator for the policy. Calls the underlying policy.
         */
        template<typename... ARGS>
        auto operator()( ARGS&&... args ) -> decltype( _policy( std::forward<ARGS>( args )... ) )
        {
            return _policy( std::forward<ARGS>( args )... );
        }
        
        /*
         * (const) Call operator for the policy. Calls the underlying policy.
         */
        template<typename... ARGS>
        auto operator()( ARGS&&... args ) const -> decltype( _policy( std::forward<ARGS>( args )... ) )
        {
            return _policy( std::forward<ARGS>( args )... );
        }
        
        /*
         * Call overload for update request.
         */
        void operator()( sdst::state_change change )
        {
            update_request<POLICY>::execute( _policy , change );
        }
        
        
        /*
         * Gives const access to the underlying policy.
         */
        const POLICY& get() const
        {
            return _policy;
        }
        
        /*
         * Gives mutable access to the underlying policy.
         */
        POLICY& get()
        {
            return _policy;
        }
        
        /*
         * The wrapper is implicitly convertible to the underlying policy type
         * to make it work in any policy-related context. Several conversion ops
         * are provided:
         * 
         * NOTE: The to rvalue conversion is not provided implicitly, could be dangerous 
         * since it could stole the policy representation silently.
         */
        
        
        operator POLICY() const
        {
            return _policy;
        }
        
        operator const POLICY() const
        {
            return _policy;
        }
        
        operator POLICY&()
        {
            return _policy;
        }
        
        operator const POLICY&() const
        {
            return _policy;
        }
        
        explicit operator POLICY&&()
        {
            return std::move( _policy );
        }
        
    private:
        /*
         * Update request. This specialization is rejected if the underlying
         * policy doesn't support updates (Is non-stated).
         */
        template<typename P , typename HAS_STATE = tml::is_valid_call<P,sdst::state_change>>
        struct update_request
        {
            static void execute( P& policy , sdst::state_change change )
            {
                policy( change );
            }
        };
        
        /*
         * Update request. This specialization is rejected if the underlying
         * policy supports updates (Is stated).
         */
        template<typename P>
        struct update_request<P,tml::false_type>
        {
            static void execute( P& policy , sdst::state_change change )
            {
                /* does nothing */
            }
        };
    };
}

#endif	/* STATED_POLICIES_HPP */

