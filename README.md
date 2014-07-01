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
    canvas << "(" << data.x << "," << data.y << ")";
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
    auto engine = sdst::make_engine( scene , std::cout , draw_scene );

    //And now just start the simulation!
    engine.start();
}

```

The example above is so simple, but shows very whell the spirit of this library: You don't have to write raw simulation loops,
you don't have to write your own particle type. **Just write how particles behave and how they are drawed, and the library
does the rest for you!**

## The architecture

The key concept behind the Stardust design are function entities. A Stardust particle engine only expects one way to evolve
particles, one way to draw a particle, and then one way to draw the simulation. Doesn't expects an instance of a specific
`StardustParticleEvolutionPolicy` hierarchy, neither a class implementing the `StardustParticleDrawPolicy` interface, etc.
(Thanks to the Java guys for this very good naming and practices).  
**Stardust only expects function entities with the required signature**. You could use lambdas, functors, plain old C functions,
or whatever you like!

This gives many power and expresiveness to the library, because when some policy is simple you could write it using a lambda 
in a couple of lines, but when you are imaginative you could write your own functor which, behaves like a function, but could
carry state, information, etc.   

*"Ok, Alexandrescu's policy-based designs are so smart, but they lock the specification of policies at compile-time"* may you think.
Well, thats true considering how policies are specified (Via template argumments).  
Stardust takes care of these and implements type-erasure for policies allowing you to specify the policies at runtime if thats what you need:

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

//Specify the particle data, the evolution policy, but delay the specification 
//of the drawing policy until runtime
using particle = sdst::particle<particle_data,my_update_policy,sdst::erase>;

particle particle_builder( void(*)( particle_data& , std::ostream& ) drawing_policy )
{
    return { particle_data{} , my_update_policy{ 25.0f } , drawing_policy };
}

```

Also Stardust encourages the usage of builder function templates to create the different entities a particle engine needs. This combined with C++11 `auto` makes possible to avoid type specification and make the code much more clear:

``` cpp
auto particle = sdst::make_particle( particle_data{1.0f , 2.0f} , update , draw );
```

## Performance

Why a policy-based design? Why not just the so common OO polymorphism-based class hierarchy?

Well, its simple: **OO designs are not very good at performance**

When your goal is to have productivity and scalability, putting a virtual machine at the bottom of your system, and rely on polymorphism and a universal base class seems like a very good idea, even if nowadays people is realizing that OOP is not that awesome, scalable, and reusable programming paradigm that the comunity claimed at the eighties and nineties.    
Besides of the problems, everybody does OOP, frameworks are OOP, so an OOP design could sound great. **If performance is not your goal**.

Current state-of-the-art hardware architectures are not that simple Von Neumman
"Here is a CPU, here is a RAM, here is the storage" schema. Since the days of the original Intel Pentium the CPUs are some kind of superscalar processors, cappable of rearange instructions in real-time, compute most of them in parallel, and nowadays having a large amount of pipelines per core cappable of processing near to 60 micro-ops per cicle. In fact modern RISC architectures are so complicated. **We need to take care of this power and use it**.

Modern C++ compilers are just a kind optimization-monsters, things eating thousands of lines of code per second, applying global code optimizations, inlining everything, and computing everything they can at compile-time.  
But that optimizers have one more property that we (The common human programmer) don't: **They know perfectly the underlying architecture they are compiling to, and know all its features and tricks to make your code really fast**.  
**Don't try to beat the compiler**. Some of the most proficient programming folks have spent thousands of hours to write that compiler. Don't reinvent the wheel and use their work. 

### And what OOP have to do with these? 

The ability of the compiler to optimice your code comes from its knwoledge of the hole sourcecode and how its source is used. **If you delay some call decissions to runtime it cannot optimice them**. In other words, polymorphism prevents inlining. (Of course you could do some PGOs and expect the compiler to erase the vtables and inline the used calls based on the profilling results).

But thats only one of the problems that OOP has in the current programming enviroment. When talking about the hardware, the way OOP works (specifically its memory access and function call patterns) break all of the efforts which hardware engineers did to speed up the system. Of course we are talking about **the memory hierarchy and the branch predictor**.

Some people don't understand that avoiding a variable, putting a value on a register instead of the stack, or storing a function call result in a variable instead of calling it multiple times, **are just examples of stupid and simple microoptimizations that doesn't speed up your program at all, and in fact any decent compiler is cappable of perform such and other advanced optimizations.**
Center your optimization efforts on algorithms and access patterns that encourages the correct usage of the memory hierarchy (The so called "*cache friendlyness*"") and to have the CPU pipeline/s full of instructions everytime. That is, **do cache-friendly memory access patterns, and avoid processor stalls by doing predictable call/jump patterns**. 


### What does Stardust?

Stardust uses policy-based designs because two points:

 - **They are just a fancy C++ name for component-based designs**: Yes, they are the C++ equivalent of component based designs, a design approach which reduces the some common coupling related to OO class hierarchies.

 - **Policies are specified by default at compile-time**: Since by default policies are specified as template parameters, the compiler is completely cappable to inline all the function calls, speeding up the program. Stardust tries to use compile-time polymorphism instead of dynamic-binding whenever its possible. Also even using type erasure, the library tries to use compile-time type erasure instead of runtime (Cast of polymorphism based), only using the former if there is no option. 

When there is no way to avoid dynamic binding, type-erasure, and classic OO polymorphic designs Stardust provides some data structores to avoid (Or reduce) the problems inehernt to OO explained above, like the Turbo polymorphic container (See [this](http://bannalia.blogspot.com.es/2014/05/fast-polymorphic-collections.html) article ).



# License 

The Stardust library is currently licensed under the Apache License, version 2.0:

```
    Copyright 2014 Manuel Sánchez Pérez

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
```

See the [`LICENSE`](https://github.com/Manu343726/Stardust/blob/master/LICENSE) file for more information.
