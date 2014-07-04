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

#ifndef ENGINE_HPP
#define	ENGINE_HPP

#include <functional>
#include <utility>

#include "stated_policies.hpp"

namespace sdst
{ 
    /*
     * A manual engine doesn't perform a simulation loop, it only provides the basic
     * functionality for the different simulation steps. The loop should be done manually
     * by the user.
     * 
     * Since its a basic engine, it only manages a scene and the drawing policy for it,
     * doesn't care about global evolution policies and other things shared between particles.
     */
    template<typename SCENE , typename DRAW_POLICY>
    struct basic_manual_engine
    {
        /*
         * The type of the scene.
         */
        using scene_t = SCENE;
        
        /*
         * The type of the scene draw policy used.
         */
        using draw_policy_t = DRAW_POLICY;
        
        
        /*
         * Initializes the engine passign the values to initialize the scene and the drawing policy
         */
        basic_manual_engine( const scene_t& scene , const draw_policy_t& draw_policy ) :
            _scene{ scene } ,
            _drawing_policy{ draw_policy }
        {}
            
        /*
         * Executes one step of the simulation.
         */
        void step()
        {
            /*
             * Yes, just a raw loop, but its extremelly powerfull:
             * It works for any SCENE type which has defined iterator getters (begin() and end()),
             * and the engine shouldn't have to take care of the type of the particles, can just
             * rely on duck typing.
             */
            for( auto& particle : _scene )
                particle.update();
            
            //Update the drawing policy:
            _drawing_policy( sdst::state_change::global );
        }
        
        /*
         * Draws the current state of the simulation.
         */
        void draw()
        {
            _drawing_policy( _scene );
        }
        
        
        /*
         * Provides full (Read/Write) access to the underlying scene of an engine.
         */
        SCENE& scene()
        {
            return _scene;
        }

        /*
         * Provides readonly access to the underlying scene of an engine
         */
        const SCENE& scene() const
        {
            return _scene;
        }
    private:
        scene_t                          _scene;
        sdst::erase_state<draw_policy_t> _drawing_policy;
    };
    
    template<typename SCENE , typename DRAW_POLICY>
    sdst::basic_manual_engine<typename std::decay<SCENE>::type,typename std::decay<DRAW_POLICY>::type> make_basic_manual_engine( SCENE&& scene , DRAW_POLICY&& draw_policy )
    {
        return { std::forward<SCENE>( scene ) , std::forward<DRAW_POLICY>( draw_policy ) };
    }
    
    /*
     * An automatic engine encapsulates a simulation loop which could be controlled
     * specifying the different stages of a simulation frame, and a running condition.
     */
    template<typename SCENE , typename DRAW_POLICY>
    struct basic_automatic_engine
    {
        /*
         * The type of the scene.
         */
        using scene_t = SCENE;
        
        /*
         * The type of the scene draw policy used.
         */
        using draw_policy_t = DRAW_POLICY;
        
        /*
         * An automatic engine manages a manual engine. This is the type of such engine.
         */
        using underlying_engine_t = sdst::basic_manual_engine<scene_t,draw_policy_t>;
        
        /*
         * The type of the engine
         */
        using engine_t = basic_automatic_engine;
        
        
        /*
         * The engine runs the simulation in a loop controlled with a running condition.
         * The running condition is a boolean predicate which should be specified by the user.
         * Its default value is [](){ return true; }, i.e. performs an infinite loop.
         */
        using running_condition_t = std::function<bool(const engine_t&)>;
        
        /*
         * The engine speficifies actions for different simulation stages.
         * This is the type of a mutable action.
         */
        using mutable_action_t = std::function<void(engine_t&)>;
        
        /*
         * The engine speficifies actions for different simulation stages.
         * This is the type of a inmutable action.
         */
        using inmutable_action_t = std::function<void(const engine_t&)>;
        
        
        /*
         * Result of the simulation. Could contain profilling data, etc.
         * Currently void.
         */
        using simulation_result_t = void;
        
        
        /*
         * Initializes the engine passign the values to initialize the scene and the drawing policy
         */
        basic_automatic_engine( const scene_t& scene , const draw_policy_t& draw_policy ) :
            _engine{ scene , draw_policy },
            _run_condition{ []( const engine_t& ){ return true; } },
            _before_update{ []( engine_t& ){} },
            _before_draw{ []( engine_t& ){} },
            _before_next{ []( engine_t& ){} }
        {}
            
        
        /*
         * Specifies the action to be performed before the engine updates the state of
         * the simulation.
         */
        engine_t& before_update( const mutable_action_t& action )
        {
            _before_update = action;

            return *this;
        }
        
        /*
         * Specifies the action to be performed before the engine draws the current
         * state of the scene.
         */
        engine_t& before_draw( const mutable_action_t& action )
        {
            _before_draw = action;

            return *this;
        }
        
        /*
         * Specifies the action to be performed before the engine goes to the next
         * step of the simulation.
         */
        engine_t& before_next( const mutable_action_t& action )
        {
            _before_next = action;

            return *this;
        }
        
        engine_t& run_condition( const running_condition_t& condition )
        {
            _run_condition = condition;
            
            return *this;
        }
        
        /*
         * Starts the simulation.
         */
        simulation_result_t start()
        {
            do
            {
                _before_update( *this );
                _engine.step();
                _before_draw( *this );
                _engine.draw();
                _before_next( *this );  
            }while( _run_condition( *this ) );
        }
        
        /*
         * Stops the simulation setting the running condition to false.
         */
        void stop()
        {
            _run_condition = []( const engine_t& ){ return false; };
        }
        
        /*
         * Starts and runs the simulation while the passed condition is true.
         */
        simulation_result_t run_while( const running_condition_t& condition )
        {
            return run_condition( condition ).start();
        }
        
        /*
         * Starts and runs the simulation until the passed condition is true.
         */
        simulation_result_t run_until( const running_condition_t& condition )
        {
            return run_while( [&]( const engine_t& e ){ return !condition(e); } );
        }
        
        /*
         * Starts and runs the simulation while some property is met by all the particles
         * of the scene.
         */
        template<typename PROPERTY>
        simulation_result_t run_while_all( PROPERTY property )
        {
            run_while( [&]( const engine_t& e )
            {
                return std::all_of( std::begin( e.scene() ) , std::end( e.scene() ) , property );
            });
        }
        
        /*
         * Starts and runs the simulation while some property is met by at least one particle
         * of the scene.
         */
        template<typename PROPERTY>
        simulation_result_t run_while_any( PROPERTY property )
        {
            run_while( [&]( const engine_t& e )
            {
                return std::any_of( std::begin( e.scene() ) , std::end( e.scene() ) , property );
            });
        }
        
        /*
         * Starts and runs the simulation until some property is met by all the particles
         * of the scene.
         */
        template<typename PROPERTY>
        simulation_result_t run_until_all( PROPERTY property )
        {
            run_until( [&]( const engine_t& e )
            {
                return std::all_of( std::begin( e.scene() ) , std::end( e.scene() ) , property );
            });
        }
        
        /*
         * Starts and runs the simulation until some property is met by at least one particle
         * of the scene.
         */
        template<typename PROPERTY>
        simulation_result_t run_until_any( PROPERTY property )
        {
            run_until( [&]( const engine_t& e )
            {
                return std::any_of( std::begin( e.scene() ) , std::end( e.scene() ) , property );
            });
        }
         
        
        /*
         * Provides full (Read/Write) access to the underlying scene of an engine.
         */
        SCENE& scene()
        {
            return _engine.scene();
        }

        /*
         * Provides readonly access to the underlying scene of an engine
         */
        const SCENE& scene() const
        {
            return _engine.scene();
        }
    private:
        underlying_engine_t _engine;
        
        running_condition_t _run_condition;
        mutable_action_t    _before_update;
        mutable_action_t    _before_draw; //Note that after update is before draw too.
        mutable_action_t    _before_next; //After draw is before next iteration.
    };
    
    template<typename SCENE , typename DRAW_POLICY>
    sdst::basic_automatic_engine<typename std::decay<SCENE>::type,typename std::decay<DRAW_POLICY>::type> make_basic_automatic_engine( SCENE&& scene , DRAW_POLICY&& draw_policy )
    {
        return { std::forward<SCENE>( scene ) , std::forward<DRAW_POLICY>( draw_policy ) };
    }
}

#endif	/* ENGINE_HPP */

