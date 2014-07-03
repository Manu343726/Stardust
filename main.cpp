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

#ifndef SCENE_SIZE
#define SCENE_SIZE 100
#endif

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
    data.x += 5.0f;
}

void draw( const particle_data& data )
{
    std::cout << "(" << data.x << "," << data.y << ")";
}

using particle_t = decltype( sdst::make_particle( particle_data{} , update , draw ) );
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
        scene.emplace_back( particle_data{ static_cast<float>( i ) , 1.0f } , update , draw );
        
    auto engine = sdst::make_basic_automatic_engine( std::move(scene) , draw_scene );
    
    engine.before_draw( []( decltype(engine)& e ){ std::cout << "Drawing scene...\n"; } )
    
          .before_next( []( decltype(engine)& e )
                        {
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
                              return particle.data().x > 300.0f;
                          }
                        );
}

