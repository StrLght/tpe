#include <tpe/Polygon.h>

Polygon::Polygon(Body *body, std::vector<glm::vec2> vertices)
{
	this->body = body;

	for (glm::vec2 vert : vertices)
	{
		this->vertices.push_back(vert);
		this->base_vertices.push_back(vert);
	}

	for (int i = 0; i < this->vertices.size(); i++)
	{
		Edge edge;
		glm::vec2 first = this->vertices[i % this->vertices.size()], second = this->vertices[(i + 1) % this->vertices.size()];
		glm::vec2 result = second - first;
		edge.n = glm::normalize(glm::vec2(-result.y, result.x));
		edge.d = glm::dot(edge.n, first);
		this->base_edges.push_back(edge);
	}
	
	this->body->shapes.push_back(*this);

	this->updateRotation();

	this->updateAABB();
}

float Polygon::IMoment()
{
	float ti = 0, j = 0;
	glm::vec2 first, second;

	for (int i = 0; i < this->vertices.size(); i++)
	{
		first = this->base_vertices[i];
		second = this->base_vertices[(i + 1) % this->vertices.size()];

		float perpdot = (second.x * first.y) - (first.x * second.y);
		float sumdot = glm::dot(first, second) + glm::dot(first, first) + glm::dot(second, second);

		ti += perpdot * sumdot;
		j += perpdot + sumdot;
	}

	return (body->mass * ti) / (6.f * j);
}

float Polygon::edgeDistance(glm::vec2 n, float d)
{
	float m = glm::dot(n, this->vertices[0]);

	for (int i = 1; i < this->vertices.size(); i++)
		if (glm::dot(n, this->vertices[i]) < m)
			m = glm::dot(n, this->vertices[i]);

	return m - d;
}

int Polygon::minEdgeDistanceTo(Polygon *polygon, float &dist)
{
	int index = -1;
	float min, d;

	for (int i = 0; i < polygon->edges.size(); i++)
	{
		d = this->edgeDistance(polygon->edges[i].n, polygon->edges[i].d);

		if (min > 0.f)
			return -1;
		else if (min < d || i == 0)
		{
			min = d;
			index = i;
		}
	}
	dist = d;
	return index;
}

bool Polygon::containsPoint(glm::vec2 point)
{
	for (int i = 0; i < this->vertices.size(); i++)
		if (glm::dot(this->edges[i].n, point) - this->edges[i].d > 0.f)
			return false;
	return true;
}

void Polygon::checkIntersection(Polygon *polygon, glm::vec2 n, float d)
{
	float k;

	d = abs(d);

	for (int i = 0; i < this->vertices.size(); i++)
	{
		if (polygon->containsPoint(this->vertices[i]))
		{
			Collision collision;
			collision.position = this->vertices[i];
			collision.n = n;
			collision.depth = d;
			collision.r1 = collision.position - this->body->position;
			collision.r2 = collision.position - polygon->body->position;

			if (this->body->isStatic)
				k = 0.f;
			else if (polygon->body->isStatic)
				k = 1.f;
			else
				k = .5f;

			this->body->position -= collision.n * (k * collision.depth);
			polygon->body->position += collision.n * ((1 - k) * collision.depth);

			collision.solve(this->body, polygon->body);
		}
	}
}

bool Polygon::collides(Polygon *polygon)
{
	float min_dist1 = 0.f, min_dist2 = 0.f;

	int min_index1 = polygon->minEdgeDistanceTo(this, min_dist1);
	if (min_index1 == -1)
		return false;

	int min_index2 = this->minEdgeDistanceTo(polygon, min_dist2);
	if (min_index2 == -1)
		return false;

	if (min_dist1 > min_dist2)
		this->checkIntersection(polygon, polygon->edges[min_index1].n, min_dist1);
	else
		polygon->checkIntersection(this, this->edges[min_index2].n, min_dist2);

	return true;
}

void Polygon::updateAABB()
{
	if (this->aabb != NULL)
		delete this->aabb;

	this->aabb = new AABB();
	this->aabb->x1 = this->vertices[0].x;
	this->aabb->x2 = this->vertices[0].x;
	this->aabb->y1 = this->vertices[0].y;
	this->aabb->y2 = this->vertices[0].y;

	for (glm::vec2 vertex : this->vertices)
	{
		if (vertex.x < this->aabb->x1) this->aabb->x1 = vertex.x;
		if (vertex.x > this->aabb->x2) this->aabb->x2 = vertex.x;
		if (vertex.y < this->aabb->y1) this->aabb->y1 = vertex.y;
		if (vertex.y > this->aabb->y2) this->aabb->y2 = vertex.y;
	}
}

void Polygon::updateRotation()
{
	this->rotation = glm::vec2((float)cos(this->body->angle), (float)sin(this->body->angle));

	for (int i = 0; i < this->vertices.size(); i++)
	{
		this->vertices[i] = this->body->position + glm::vec2(this->base_vertices[i].x * this->rotation.x - this->base_vertices[i].y * this->rotation.y, this->base_vertices[i].x * this->rotation.y + this->base_vertices[i].y * this->rotation.x); //todo: rotation
		this->edges[i].n = glm::vec2(this->base_edges[i].n.x * this->rotation.x - this->base_edges[i].n.y * this->rotation.y, this->base_edges[i].n.y * this->rotation.x + this->base_edges[i].n.y * this->rotation.y); //todo: rotation
		this->edges[i].d = glm::dot(this->body->position, this->edges[i].n) + this->base_edges[i].d;
	}
}