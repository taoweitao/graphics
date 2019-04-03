#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	std::ifstream ifs( fname.c_str() );
	if(!ifs.is_open()) {
		ifs.open( string("Assets/") + fname.c_str() );
		if(!ifs.is_open())
			std::cout << "Please make sure that the mesh file is in the same path with A4 or in the Assets directory." << std::endl;
	}

	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*
  
  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}

glm::vec3 Mesh::getPos() {
	return vec3(0.0f, 0.0f, 0.0f);
}

int Mesh::getType() {
	return 5;
}

double Mesh::intersect(const Ray &ray, vec3 &normal) {
	double minx = 1e9, miny = 1e9, minz = 1e9;
	double maxx = -1e9, maxy = -1e9, maxz = -1e9;
	for(glm::vec3 point: m_vertices) {
		if(point.x < minx)
			minx = point.x;
		if(point.x > maxx)
			maxx = point.x;

		if(point.y < miny)
			miny = point.y;
		if(point.y > maxy)
			maxy = point.y;

		if(point.z < minz)
			minz = point.z;
		if(point.z > maxz)
			maxz = point.z;
	}
	
	  double ans = -1;
	  double k;
	  glm::vec3 v1 = vec3(minx, miny, minz);
	  glm::vec3 v2 = vec3(maxx, miny, minz);
	  glm::vec3 v3 = vec3(minx, miny, maxz);
	  glm::vec3 v4 = vec3(maxx, miny, maxz);
	  glm::vec3 v5 = vec3(minx, maxy, minz);
	  glm::vec3 v6 = vec3(maxx, maxy, minz);
	  glm::vec3 v7 = vec3(minx, maxy, maxz);
	  glm::vec3 v8 = vec3(maxx, maxy, maxz);

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

	  //return ans;
	if(ans < 0)
		return ans;

	ans = -1;
	for(Triangle triangle: m_faces) {
		double k;
		if((k = triIntersect(ray, m_vertices[triangle.v1], m_vertices[triangle.v2], m_vertices[triangle.v3])) > 0) {
			if(ans < 0) {
				ans = k;
				normal = triangleNormal(m_vertices[triangle.v1], m_vertices[triangle.v2], m_vertices[triangle.v3]);
			}
			else if(k < ans && k > 0.1) {
				ans = k;
				normal = triangleNormal(m_vertices[triangle.v1], m_vertices[triangle.v2], m_vertices[triangle.v3]);
			}	
		}
	}

	return ans;
}
