Stardust
========

A C++11 particle engine generator

# Description

Stardust is a modern C++ library which uses a [policy-based]() design to build up and customize particle engines.
Stardust provides you a complete working particle engine, where you specify what a particle is, how a particle works, 
and how a particle and the scene in general is renderized.

``` cpp

//Define a type which specifies the data of a particle
struct particle_data
{
    float x , y;

    particle_data(float _x = 0.0f , float _y = 0.0f ) : 
        x{_x}, 
        y{_y}
    {}
};

//Define how a particle works (evolves) along the simulation:
void update( particle_data& data )
{
    data.x += 1.0f; //Move the particle one unit to the right
}

//Define how a particle is renderized on a particular canvas:
void draw_particle( std::ostream& canvas , particle_data& data )
{
    std::cout << "(" << data.x << "," << data.y << ")";
}

//Define how the scene (The set of particles) is renderized on a particular canvas:
template<typename SCENE>
void draw_scene( const SCENE& scene , std::ostream& canvas )
{
    for( const auto& particle : scene )
        particle.draw( canvas );
}

//Define the particle type:
using particle = decltype( sdst::make_particle( particle_data{} , update , draw_particle ) );

int main()
{
    //Define the scene (Two particles in this case):
    std::vector<particle> scene = { {{.0f , 1.0f}} , {{2.0f , 2.0f}} };
    //Define the engine:
    auto engine = sdst::make_engine( scene , draw_scene );

    //And now just start the simulation!
    engine.start();
}

 ```

The example above is so simple, but shows very whell the spirit of this library: You don't have to write raw simulation loops,
you don't have to write your own particle type. **Just write how particles behave and how they are drawed, and the library
does the rest for you!**

### The architecture

The key concept behind the Stardust design are function entities. A Stardust particle engine only expects one way to evolve
particles, one way to draw a particle, and then one way to draw the simulation. **Doesn't expects an instance of a specific
`StardustParticleEvolutionPolicy` hierarchy, neither a class implementing the `StardustParticleDrawPolicy` interface, etc.
(Thanks to the Java guys for this very good naming and practices).  
**Stardust only expects function entities with the required signature**. You could use lambdas, functors, plain old C functions,
or whatever you like!

This gives many power and expresiveness to the library, because when some policy is simple you could write it using a lambda 
in a couple of lines, but when you are imaginative you could write your own functor which, it behaves like a function, but could
carry state, information, etc.   

*Ok, Alexandrescu's policy-based design are so smart, but they locks the specification of policies to the compile-time* may you think.
Well, thats true considering how policies are specified (Via template argumments).  
Stardust takes care of these and implements type-erasure for policies allowing you to specify the policies at runtime if thats 
what you need:

``` cpp

class my_update_policy
{
public:
    my_update_policy( float offset = 0.0f ) :
        _offset{ offset }
    {}

    void operator()( particle_data& data ) const
    {
        data.x += offset;
    }

private:
    float _offset;
};

//Specify the particle data, the evolution policy, but delay the specification of the 
//drawing policy until runtime
using particle = sdst::particle<particle_data,my_update_policy,sdst::erase>;

particle particle_builder( void(*)( particle_data& , std::ostream& ) drawing_policy )
{
    return { particle_data{} , my_update_policy{ 25.0f } , drawing_policy };
}

```


