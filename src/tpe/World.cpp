#include <tpe/World.h>

namespace tpe
{

World::World(float depth) : depth(depth), gravity(0, 0), bodies()
{
}

void World::addBody(Body* body)
{
	body->world = this;
	this->bodies.push_back(body);
}

void World::step(float t, int iterations)
{
	float dt = t / iterations;
	for (int iter = 0; iter < iterations; iter++)
	{
		for (Body *body : this->bodies)
		{
			if (!body->isStatic)
				body->velocity += this->gravity * dt;
			body->angle += body->angular_velocity * dt;
			body->position += body->velocity * dt;
			body->updateRotation();
		}

		for (int i = 0; i < this->bodies.size(); i++)
		{
			for (int j = 0; j < this->bodies.size(); j++)
			{
				if (i != j && !(this->bodies[i]->isStatic && this->bodies[j]->isStatic))
					this->bodies[i]->collides(this->bodies[j]);
			}
		}
	}
}

}
