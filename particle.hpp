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

#ifndef PARTICLE_HPP
#define	PARTICLE_HPP

namespace sdst
{
    template<typename DATA , typename EVOLUTION_POLICY , typename DRAW_POLICY>
    struct particle
    {
    public:
        /*
         * The type which holds the data of a particle
         */
        using data_t = DATA;
        
        /*
         * The type of the particle evolution policy
         */
        using evolution_policy_t = EVOLUTION_POLICY;
        
        /*
         * The type of the particle drawing policy
         */
        using drawing_policy_t = DRAW_POLICY;
        
        /*
         * The type of the particle
         */
        using particle_t = particle;
        
        
        /*
         * Initializes the particle given the particle data and its evolution and drawing policies.
         */
        particle( const data_t& data , const evolution_policy_t& evolution_policy , const drawing_policy_t& drawing_policy ) :
            _data{ data },
            _evolution_policy{ evolution_policy },
            _draw_policy{ drawing_policy }
        {}
            
        /*
         * Draws the particle
         */    
        void draw()
        {
            _draw_policy( _data );
        }
        
        /*
         * Updates the particle
         */
        void update()
        {
            _evolution_policy( _data );
        }
        
        /*
         * Draws the particle
         */    
        void draw() const
        {
            _draw_policy( _data );
        }
        
        /*
         * Updates the particle
         */
        void update() const
        {
            _evolution_policy( _data );
        }
        
        /*
         * Gives readonly access to the particle data
         */
        const data_t& data() const
        {
            return _data;
        }
        
    private:
        data_t             _data;
        evolution_policy_t _evolution_policy;
        drawing_policy_t   _draw_policy;
    };
    
    /*
     * Type-deduction-based builder for particles:
     */
    template<typename DATA , typename EVOLUTION_POLICY , typename DRAW_POLICY>
    sdst::particle<typename std::decay<DATA>::type,typename std::decay<EVOLUTION_POLICY>::type,typename std::decay<DRAW_POLICY>::type> 
    make_particle( DATA&& data , EVOLUTION_POLICY&& evolution_policy , DRAW_POLICY&& draw_policy )
    {
        return { std::forward<DATA>( data ) , std::forward<EVOLUTION_POLICY>( evolution_policy ) , std::forward<DRAW_POLICY>( draw_policy ) };
    }
}

#endif /* PARTICLE_HPP */