#include "Primitive.hpp"
#include <iostream>

double triIntersect(const Ray &ray, vec3 P0, vec3 P1, vec3 P2) {
	glm::vec3 a = vec3(ray.origin);
	glm::vec3 b = vec3(ray.origin + ray.direction);
	glm::vec3 R = vec3(a.x - P0.x, a.y - P0.y, a.z - P0.z);
	glm::mat3 D = mat3(vec3(P1.x - P0.x, P2.x - P0.x, a.x - b.x), vec3(P1.y - P0.y, P2.y - P0.y, a.y - b.y), vec3(P1.z - P0.z, P2.z - P0.z, a.z - b.z));
	glm::mat3 D1 = mat3(vec3(a.x - P0.x, P2.x - P0.x, a.x - b.x), vec3(a.y - P0.y, P2.y - P0.y, a.y - b.y), vec3(a.z - P0.z, P2.z - P0.z, a.z - b.z));
	glm::mat3 D2 = mat3(vec3(P1.x - P0.x, a.x - P0.x, a.x - b.x), vec3(P1.y - P0.y, a.y - P0.y, a.y - b.y), vec3(P1.z - P0.z, a.z - P0.z, a.z - b.z));
	glm::mat3 D3 = mat3(vec3(P1.x - P0.x, P2.x - P0.x, a.x - P0.x), vec3(P1.y - P0.y, P2.y - P0.y, a.y - P0.y), vec3(P1.z - P0.z, P2.z - P0.z, a.z - P0.z));

	double beta = determinant(D1)/determinant(D);
	double gamma = determinant(D2)/determinant(D);
	double t = determinant(D3)/determinant(D);

	if((beta >= 0) && (gamma >= 0) && (beta + gamma <= 1) && (t > 0))
		return t;
	else
		return -1;
}

Primitive::~Primitive()
{
}

Sphere::~Sphere()
{
}

double Sphere::intersect(const Ray &ray, vec3 &normal) {
	glm::vec3 a = vec3(ray.origin);
	glm::vec3 b = vec3(ray.origin + ray.direction);
	glm::vec3 c = vec3(0.0f, 0.0f, 0.0f);

	double A = dot(b - a, b - a);
	double B = 2*dot(b - a, a - c);
	double C = dot(a - c, a - c) - 1;

	double roots[2];
	size_t k = quadraticRoots(A, B, C, roots);

	if(k == 0)
		return -1;
	else if(k == 1) {
		normal = a + float(roots[0])*(b - a);
		return roots[0];
	}
	else {
		if(roots[0] > 0.1 && roots[1] > 0.1) {
			normal = a + float(std::min(roots[0], roots[1]))*(b - a);
			return std::min(roots[0], roots[1]);
		}
		else {
			normal = a + float(std::max(roots[0], roots[1]))*(b - a);
			return std::max(roots[0], roots[1]);
		}
	}
}

Cube::~Cube()
{
}

double Cube::intersect(const Ray &ray, vec3 &normal) {
	  double ans = -1;
	  double k;
	  glm::vec3 v1 = vec3(0.0f, 0.0f, 0.0f);
	  glm::vec3 v2 = vec3(1.0f, 0.0f, 0.0f);
	  glm::vec3 v3 = vec3(0.0f, 0.0f, 1.0f);
	  glm::vec3 v4 = vec3(1.0f, 0.0f, 1.0f);
	  glm::vec3 v5 = vec3(0.0f, 1.0f, 0.0f);
	  glm::vec3 v6 = vec3(1.0f, 1.0f, 0.0f);
	  glm::vec3 v7 = vec3(0.0f, 1.0f, 1.0f);
	  glm::vec3 v8 = vec3(1.0f, 1.0f, 1.0f);

	  //bottom
	  if((k = triIntersect(ray, v1, v2, v3)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, -1, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, -1, 0);
		  }
	  }
	  if((k = triIntersect(ray, v4, v2, v3)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, -1, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, -1, 0);
		  }
	  }

	  //left
	  if((k = triIntersect(ray, v1, v3, v5)) > 0) {
		  if(ans < 0) {
			  normal = vec3(-1, 0, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(-1, 0, 0);
		  }
	  }
	  if((k = triIntersect(ray, v7, v3, v5)) > 0) {
		  if(ans < 0) {
			  normal = vec3(-1, 0, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(-1, 0, 0);
		  }
	  }

	  //back
	  if((k = triIntersect(ray, v1, v2, v5)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, 0, -1);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, 0, -1);
		  }
	  }
	  if((k = triIntersect(ray, v6, v2, v5)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, 0, -1);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, 0, -1);
		  }
	  }

	  //top
	  if((k = triIntersect(ray, v5, v6, v7)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, 1, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, 1, 0);
		  }
	  }
	  if((k = triIntersect(ray, v8, v6, v7)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, 1, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, 1, 0);
		  }
	  }

	  //right
	  if((k = triIntersect(ray, v2, v4, v6)) > 0) {
		  if(ans < 0) {
			  normal = vec3(1, 0, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(1, 0, 0);
		  }
	  }
	  if((k = triIntersect(ray, v8, v4, v6)) > 0) {
		  if(ans < 0) {
			  normal = vec3(1, 0, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(1, 0, 0);
		  }
	  }

	  //front
	  if((k = triIntersect(ray, v3, v4, v7)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, 0, 1);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, 0, 1);
		  }
	  }
	  if((k = triIntersect(ray, v8, v4, v7)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, 0, 1);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, 0, 1);
		  }
	  }

	  return ans;
}

NonhierSphere::~NonhierSphere()
{
}

double NonhierSphere::intersect(const Ray &ray, vec3 &normal) {
	glm::vec3 a = vec3(ray.origin);
	glm::vec3 b = vec3(ray.origin + ray.direction);
	glm::vec3 c = m_pos;

	double A = dot(b - a, b - a);
	double B = 2*dot(b - a, a - c);
	double C = dot(a - c, a - c) - m_radius*m_radius;

	double roots[2];
	size_t k = quadraticRoots(A, B, C, roots);
	
	if(k == 0)
		return -1;
	else if(k == 1) {
		normal = normalize(a + float(roots[0])*(b - a) - m_pos);
		return roots[0];
	}
	else {
		if(roots[0] > 0.1 && roots[1] > 0.1) {
			normal = normalize(a + float(std::min(roots[0], roots[1]))*(b - a) - m_pos);
			return std::min(roots[0], roots[1]);
		}
		else {
			normal = normalize(a + float(std::max(roots[0], roots[1]))*(b - a) - m_pos);
			return std::max(roots[0], roots[1]);
		}
	}
}

NonhierBox::~NonhierBox()
{
}

double NonhierBox::intersect(const Ray &ray, vec3 &normal) {
	  double ans = -1;
	  double k;
	  glm::vec3 v1 = m_pos;
	  glm::vec3 v2 = m_pos + vec3(m_size, 0.0f, 0.0f);
	  glm::vec3 v3 = m_pos + vec3(0.0f, 0.0f, m_size);
	  glm::vec3 v4 = m_pos + vec3(m_size, 0.0f, m_size);
	  glm::vec3 v5 = m_pos + vec3(0.0f, m_size, 0.0f);
	  glm::vec3 v6 = m_pos + vec3(m_size, m_size, 0.0f);
	  glm::vec3 v7 = m_pos + vec3(0.0f, m_size, m_size);
	  glm::vec3 v8 = m_pos + vec3(m_size, m_size, m_size);

	  //bottom
	  if((k = triIntersect(ray, v1, v2, v3)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, -1, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, -1, 0);
		  }
	  }
	  if((k = triIntersect(ray, v4, v2, v3)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, -1, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, -1, 0);
		  }
	  }

	  //left
	  if((k = triIntersect(ray, v1, v3, v5)) > 0) {
		  if(ans < 0) {
			  normal = vec3(-1, 0, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(-1, 0, 0);
		  }
	  }
	  if((k = triIntersect(ray, v7, v3, v5)) > 0) {
		  if(ans < 0) {
			  normal = vec3(-1, 0, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(-1, 0, 0);
		  }
	  }

	  //back
	  if((k = triIntersect(ray, v1, v2, v5)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, 0, -1);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, 0, -1);
		  }
	  }
	  if((k = triIntersect(ray, v6, v2, v5)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, 0, -1);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, 0, -1);
		  }
	  }

	  //top
	  if((k = triIntersect(ray, v5, v6, v7)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, 1, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, 1, 0);
		  }
	  }
	  if((k = triIntersect(ray, v8, v6, v7)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, 1, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, 1, 0);
		  }
	  }

	  //right
	  if((k = triIntersect(ray, v2, v4, v6)) > 0) {
		  if(ans < 0) {
			  normal = vec3(1, 0, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(1, 0, 0);
		  }
	  }
	  if((k = triIntersect(ray, v8, v4, v6)) > 0) {
		  if(ans < 0) {
			  normal = vec3(1, 0, 0);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(1, 0, 0);
		  }
	  }

	  //front
	  if((k = triIntersect(ray, v3, v4, v7)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, 0, 1);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, 0, 1);
		  }
	  }
	  if((k = triIntersect(ray, v8, v4, v7)) > 0) {
		  if(ans < 0) {
			  normal = vec3(0, 0, 1);
			  ans = k;
		  }
		  else if(k < ans && k > 0.1) {
			  ans = k;
			  normal = vec3(0, 0, 1);
		  }
	  }

	  return ans;
}
