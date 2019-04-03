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

glm::vec3 Sphere::getPos() {
	return vec3(0.0f, 0.0f, 0.0f);
}

int Sphere::getType() {
	return 1;
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
		if(roots[0] > 0.1)
			return roots[0];
		else
			return -1;
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

glm::vec3 Cube::getPos() {
	return vec3(0.5f, 0.5f, 0.5f);
}

int Cube::getType() {
	return 2;
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

glm::vec3 NonhierSphere::getPos() {
	return m_pos;
}

int NonhierSphere::getType() {
	return 1;
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
		if(roots[0] > 0.1)
			return roots[0];
		else
			return -1;
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

glm::vec3 NonhierBox::getPos() {
	return m_pos + vec3(m_size/2, m_size/2, m_size/2);
}

int NonhierBox::getType() {
	return 2;
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

Cylinder::~Cylinder()
{
}

glm::vec3 Cylinder::getPos() {
	return vec3(0.0f, 0.0f, 0.0f);
}

int Cylinder::getType() {
	return 3;
}

double Cylinder::intersect(const Ray &ray, vec3 &normal) {
	glm::vec3 E = vec3(ray.origin);
	glm::vec3 D = vec3(ray.direction);
	
	double A = D.x*D.x + D.y*D.y;
	double B = 2*E.x*D.x + 2*E.y*D.y;
	double C = E.x*E.x + E.y*E.y - 1;

	double roots[2];
	size_t k = quadraticRoots(A, B, C, roots);
			
	if(k == 0)
		return -1;
	else if(k == 1) {
		glm::vec3 P = E + float(roots[0])*D;
		normal = normalize(vec3(P.x, P.y, 0.0f));
		if(roots[0] > 0.1)
			return roots[0];
		else
			return -1;
	}
	else {
		double zmin = -1;
		double zmax = 1;
		double z1 = E.z + roots[0]*D.z;
		double z2 = E.z + roots[1]*D.z;

		if((z1 < zmin && z2 < zmin) || (z1 > zmax && z2 > zmax))
			return -1;
		else if(z1 >= zmin && z2 >= zmin && z1 <= zmax && z2 <= zmax) {
			if(roots[0] > 0.1 && roots[1] > 0.1) {
				glm::vec3 P = E + float(std::min(roots[0], roots[1]))*D;
				normal = normalize(vec3(P.x, P.y, 0.0f));
				return std::min(roots[0], roots[1]);
			}
			else {
				glm::vec3 P = E + float(std::max(roots[0], roots[1]))*D;
				normal = normalize(vec3(P.x, P.y, 0.0f));
				return std::max(roots[0], roots[1]);
			}
		}
		else if(z1 >= zmin && z1 <= zmax && z2 < zmin) {
			roots[1] = (zmin - E.z)/D.z;
			if(roots[0] > 0.1 && roots[1] > 0.1) {
				if(roots[0] < roots[1]) {
					glm::vec3 P = E + float(roots[0])*D;
					normal = normalize(vec3(P.x, P.y, 0.0f));
					return roots[0];
				}
				else {
					normal = vec3(0.0f, 0.0f, -1.0f);
					return roots[1];
				}
			}
			else {
				if(roots[0] > roots[1]) {
					glm::vec3 P = E + float(roots[0])*D;
					normal = normalize(vec3(P.x, P.y, 0.0f));
					return roots[0];
				}
				else {
					normal = vec3(0.0f, 0.0f, -1.0f);
					return roots[1];
				}
			}
		}
		else if(z2 >= zmin && z2 <= zmax && z1 < zmin) {
			roots[0] = (zmin - E.z)/D.z;
			if(roots[0] > 0.1 && roots[1] > 0.1) {
				if(roots[0] < roots[1]) {
					normal = vec3(0.0f, 0.0f, -1.0f);
					return roots[0];
				}
				else {
					glm::vec3 P = E + float(roots[1])*D;
					normal = normalize(vec3(P.x, P.y, 0.0f));
					return roots[1];
				}
			}
			else {
				if(roots[0] > roots[1]) {
					normal = vec3(0.0f, 0.0f, -1.0f);
					return roots[0];
				}
				else {
					glm::vec3 P = E + float(roots[1])*D;
					normal = normalize(vec3(P.x, P.y, 0.0f));
					return roots[1];
				}
			}
		}
		else if(z1 >= zmin && z1 <= zmax && z2 > zmax) {
			roots[1] = (zmax - E.z)/D.z;
			if(roots[0] > 0.1 && roots[1] > 0.1) {
				if(roots[0] < roots[1]) {
					glm::vec3 P = E + float(roots[0])*D;
					normal = normalize(vec3(P.x, P.y, 0.0f));
					return roots[0];
				}
				else {
					normal = vec3(0.0f, 0.0f, 1.0f);
					return roots[1];
				}
			}
			else {
				if(roots[0] > roots[1]) {
					glm::vec3 P = E + float(roots[0])*D;
					normal = normalize(vec3(P.x, P.y, 0.0f));
					return roots[0];
				}
				else {
					normal = vec3(0.0f, 0.0f, 1.0f);
					return roots[1];
				}
			}
		}
		else if(z2 >= zmin && z2 <= zmax && z1 > zmax) {
			roots[0] = (zmax - E.z)/D.z;
			if(roots[0] > 0.1 && roots[1] > 0.1) {
				if(roots[0] < roots[1]) {
					normal = vec3(0.0f, 0.0f, 1.0f);
					return roots[0];
				}
				else {
					glm::vec3 P = E + float(roots[1])*D;
					normal = normalize(vec3(P.x, P.y, 0.0f));
					return roots[1];
				}
			}
			else {
				if(roots[0] > roots[1]) {
					normal = vec3(0.0f, 0.0f, 1.0f);
					return roots[0];
				}
				else {
					glm::vec3 P = E + float(roots[1])*D;
					normal = normalize(vec3(P.x, P.y, 0.0f));
					return roots[1];
				}
			}
		}
		else {
			roots[0] = (zmin - E.z)/D.z;
			roots[1] = (zmax - E.z)/D.z;
			if(roots[0] > 0.1 && roots[1] > 0.1) {
				if(roots[0] < roots[1]) {
					normal = vec3(0.0f, 0.0f, -1.0f);
					return roots[0];
				}
				else {
					normal = vec3(0.0f, 0.0f, 1.0f);
					return roots[1];
				}
			}
			else {
				if(roots[0] > roots[1]) {
					normal = vec3(0.0f, 0.0f, -1.0f);
					return roots[0];
				}
				else {
					normal = vec3(0.0f, 0.0f, 1.0f);
					return roots[1];
				}
			}
		}
	}
}

Cone::~Cone()
{
}

glm::vec3 Cone::getPos() {
	return vec3(0.0f, 0.0f, 0.0f);
}

int Cone::getType() {
	return 4;
}

double Cone::intersect(const Ray &ray, vec3 &normal) {
	glm::vec3 E = vec3(ray.origin);
	glm::vec3 D = vec3(ray.direction);
	
	double A = D.x*D.x + D.y*D.y - D.z*D.z;
	double B = 2*E.x*D.x + 2*E.y*D.y - 2*E.z*D.z;
	double C = E.x*E.x + E.y*E.y - E.z*E.z;

	double roots[2];
	size_t k = quadraticRoots(A, B, C, roots);
			
	if(k == 0)
		return -1;
	else if(k == 1) {
		glm::vec3 P = E + float(roots[0])*D;
		normal = normalize(vec3(P.x, P.y, -P.z));
		if(roots[0] > 0.1)
			return roots[0];
		else
			return -1;
	}
	else {
		double zmin = 0;
		double zmax = 1;
		double t = (zmax - E.z)/D.z;
		double z1 = E.z + roots[0]*D.z;
		double z2 = E.z + roots[1]*D.z;
		glm::vec3 Q = E + float(t)*D;

		if(Q.x*Q.x + Q.y*Q.y <= zmax*zmax) {
			if(t > roots[0] || t > roots[1]) {
				double ans = -1;
				if(z1 > zmin && z1 < zmax && roots[0] > 0.1) {
					glm::vec3 P = E + float(roots[0])*D;
					normal = normalize(vec3(P.x, P.y, -P.z));
					ans = roots[0];
				}

				if(z2 > zmin && z2 < zmax && roots[1] > 0.1 && roots[1] < roots[0]) {
					glm::vec3 P = E +float(roots[1])*D;
					normal = normalize(vec3(P.x, P.y, -P.z));
					ans = roots[1];
				}
				return ans;
			}
			else if(t > 0.1) {
				normal = vec3(0.0f, 0.0f, 1.0f);
				return t;
			}
			else
				return -1;
		}
		else {
			double ans = -1;
			if(z1 > zmin && z1 < zmax && roots[0] > 0.1) {
				glm::vec3 P = E + float(roots[0])*D;
				normal = normalize(vec3(P.x, P.y, -P.z));
				ans = roots[0];
			}

			if(z2 > zmin && z2 < zmax && roots[1] > 0.1 && roots[1] < roots[0]) {
				glm::vec3 P = E +float(roots[1])*D;
				normal = normalize(vec3(P.x, P.y, -P.z));
				ans = roots[1];
			}
			return ans;
		}
	}
}
