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

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

#include "basic_engines.hpp"
#include "particle.hpp"

#include "Turbo/overloaded_function.hpp"



#ifndef SCENE_SIZE
#define SCENE_SIZE 10000
#endif

float acc = 1.0f; //Particles acceleration

struct particle_data
{
    float x , y;
    
    particle_data( float _x = 0.0f , float _y = 0.0f ) :
        x{ _x },
        y{ _y }
    {}
};

void update( particle_data& data )
{
    data.x += acc;
}

void draw( const particle_data& data )
{
    std::cout << "(" << data.x << "," << data.y << ")";
}

auto evolution_policy = tml::runtime::make_overloaded_function( update , 
                                                                [&]( sdst::state_change change ) 
                                                                {
                                                                   if( change == sdst::state_change::global )
                                                                       acc += 1.001f;
                                                                });

using particle_t = sdst::particle<particle_data,decltype(evolution_policy),decltype(draw)>;
using scene_t    = std::vector<particle_t>;

void draw_scene( const scene_t& scene )
{
    for( auto& particle : scene )
    {
        particle.draw();
        std::cout << "\n";
    }
}

int main() 
{
    scene_t scene;
    
    for( std::size_t i = 0 ; i < SCENE_SIZE ; ++i )
        scene.emplace_back( particle_data{ static_cast<float>( i ) , 1.0f } , evolution_policy , draw );
        
    auto engine = sdst::make_basic_automatic_engine( std::move(scene) , draw_scene );
    
    engine.before_draw( []( decltype(engine)& e ){ std::cout << "Drawing scene...\n"; } )
    
          .before_next( [&]( decltype(engine)& e )
                        {
                            evolution_policy( sdst::state_change::global ); //Update the evolution policy
              
                            if( e.scene().size() > 0 )
                            {
                                e.scene().pop_back();
                                std::cout << "Oh, we have lost one particle!\n";
                            }
                            else
                            {
                                std::cout << "We are out of particles! Shutting down...\n";
                                e.stop();
                            }
                        })
          
          .run_until_any( []( const particle_t& particle )
                          {
                              return particle.data().x > 30000.0f;
                          }
                        );
}

