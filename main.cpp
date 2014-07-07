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
#include <deque>
#include <algorithm>
#include <functional>

#include "basic_engines.hpp"
#include "particle.hpp"

#include "Turbo/overloaded_function.hpp"


#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <random>


#ifndef SCENE_SIZE
#define SCENE_SIZE 10000
#endif


using particle_data = sf::Vertex;

struct evolution
{
public:
    
private:
    void restart( particle_data& data )
    {
        data.position.x = _x_dist( _prng() );
        data.position.y = _y_dist( _prng() );
        
        data.color.a = 255;
        data.color.r = _rgb_dist( _prng() );
        data.color.g = _rgb_dist( _prng() );
        data.color.b = _rgb_dist( _prng() );
    }
    
    std::mt19937                                 _prng;
    std::uniform_real_distribution<float>        _x_dist;
    std::uniform_real_distribution<float>        _y_dist;
    std::uniform_int_distribution<std::uint8_t>  _rgb_dist;
    sf::Vector2f                                 _blackhole;
    float                                        _vertical_gravity;
};

/* F...ng Standard Library... */
template<typename T>
struct fixed_size_queue
{
public:
    fixed_size_queue( std::size_t size ) :
        _remaining{ size }
    {}
    
    template<typename... ARGS>
    void push( ARGS&&... args )
    {
        if( _remaining > 0 )
        {
            _queue.emplace_back( std::forward<ARGS>( args )... );
            _remaining--;
        }
    }
    
    void pop()
    {
        if( _queue.size() > 0 )
        {
            _queue.pop_front();
            _remaining++;
        }
    }
    
    auto begin() const -> decltype( _queue.begin() )
    {
        return _queue.begin();
    }
    
    auto end() const -> decltype( _queue.end() )
    {
        return _queue.end();
    }
    
    std::size_t size() const
    {
        return _queue.size();
    }
    
private:
    std::deque<T> _queue;
    std::size_t   _remaining;
};

struct particle_drawing
{
public:
    particle_drawing( std::size_t tail_length , sf::RenderTarget& canvas ) :
        _tail{ tail_length },
        _canvas{ canvas }
    {}
    
    void operator()( particle_data& data )
    {
        _tail.push( data );
        std::vector<particle_data> buffer{ _tail.size() };
        
        std::copy( std::begin( _tail ) , std::end( _tail ) , std::begin( buffer ) );
        
        _canvas.get().draw( buffer.data() , _tail.size() , sf::Points );
    }
    
private:
    std::reference_wrapper<sf::RenderTarget> _canvas;
    fixed_size_queue<sf::Vertex> _tail;
};

int main()
{
    sf::RenderWindow window;
    auto engine = sdst::make_basic_automatic_engine( generate_scene() , [&]( const std::vector<particle_data>& scene )
    {
        for( auto& particle : particles )
            particle.draw( window );
        
        window.display();
    });
    
    
}
