#pragma once

#include <glm/glm.hpp>
#include <algorithm>
#include "polyroots.hpp"
using namespace glm;
using namespace std;

struct Ray {
	glm::vec4 origin;
	glm::vec4 direction;
};

double triIntersect(const Ray &ray, vec3 P0, vec3 P1, vec3 P2);

class Primitive {
public:
  virtual ~Primitive();
  virtual double intersect(const Ray &ray, vec3 &normal) = 0;
};

class Sphere : public Primitive {
public:
  virtual ~Sphere();
  virtual double intersect(const Ray &ray, vec3 &normal);
};

class Cube : public Primitive {
public:
  virtual ~Cube();
  virtual double intersect(const Ray &ray, vec3 &normal);
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual ~NonhierSphere();
  virtual double intersect(const Ray &ray, vec3 &normal);

private:
  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
  }
  
  virtual ~NonhierBox();
  virtual double intersect(const Ray &ray, vec3 &normal);

private:
  glm::vec3 m_pos;
  double m_size;
};
