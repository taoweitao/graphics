#include <glm/ext.hpp>
#include <stack>
#include <iomanip>
#include <random>

#include "A4.hpp"
#include "GeometryNode.hpp"
using namespace std;
using namespace glm;

#define MAXHIT 4
#define REFLECTION 0
#define NUM_OF_REFLECTIONS 5
#define GLOSS 10000
#define REFRACTION false
#define ALIASE false
#define ANTI_RATIO 0.3
#define NUM_OF_LIGHTS 1
#define LIGHT_DISTANCE 100
#define NUM_OF_RAYS 1
#define LENS_RADIUS 40

#define PI 3.1415926
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(-1.0, 1.0);

void printMat4(const glm::mat4 matrix) {
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++)
			std::cout << matrix[i][j] << " ";
		std::cout << "\n";
	}
}

void printVec4(const glm::vec4 vec) {
	std::cout << vec[0] << "," << vec[1] << "," << vec[2] << "," << vec[3] << std::endl;
}

SceneNode* search(SceneNode& root, unsigned int id) {
	if(root.m_nodeId == id)
		return &root;
	
	SceneNode *ans;
	for(SceneNode* node: root.children) {
		if(ans = search(*node, id))
			return ans;
	}
	
	return NULL;
}

double getIntersect(const Ray &ray, const SceneNode &node, vec3 &normal) {
	double ans = -1;
	if(node.m_nodeType == NodeType::GeometryNode) {
		const GeometryNode *geometry;
		if(node.copy == NULL)
			geometry = static_cast<const GeometryNode*>(&node);
		else
			geometry = static_cast<const GeometryNode*>(node.copy);

		ans = geometry->m_primitive->intersect(ray, normal);
	}

	return ans;
}

void transTree(SceneNode *root) {
	for(SceneNode *node: root->children) {
		node->trans = root->trans*node->trans;
		node->invtrans = glm::inverse(node->trans);
		transTree(node);
	}
}

float fresnel(const glm::vec3 I, const glm::vec3 N, const float ior) {
	float cosi = dot(normalize(I), normalize(N));
	float ni = 1, nt = ior;
	if(cosi > 0) {
		std::swap(ni, nt);
	}

	float sint = ni/nt*sqrtf(std::max(0.0f, 1 - cosi*cosi));
	if(sint >= 1) {
		return 1;
	}
	else {
		float cost = sqrtf(std::max(0.0f, 1 - sint*sint));
		cosi = fabsf(cosi);
		float Rs = (nt*cosi - ni*cost)/(nt*cosi + ni*cost);
		float Rp = (ni*cosi - nt*cost)/(ni*cosi + nt*cost);

		return (Rs*Rs + Rp*Rp)/2;
	}
}

double uvInterpolate(const Image &image, int bit, double u, double v) {
	double di = (image.width() - 1)*u;
	double dj = (image.height() - 1)*v;
	int i = (int)di;
	int j = (int)dj;
	double up = di - i;
	double vp = dj - j;
	return image(i, j, bit)*(1 - up)*(1 - vp) + image(i, j + 1, bit)*(1 - up)*vp + image(i + 1, j, bit)*up*(1 - vp) + image(i + 1, j + 1, bit)*up*vp;
}

double fval(const Image &image, double u, double v) {
	double di = (image.width() - 1)*u;
	double dj = (image.height() - 1)*v;
	int i = (int)di;
	int j = (int)dj;
	double up = di - i;
	double vp = dj - j;
	
	double f00 = image(i + 1, j + 1, 0);
	double f10 = image(i + 2, j + 1, 0);
	double f01 = image(i + 1, j + 2, 0);
	double f11 = image(i + 2, j + 2, 0);
	double fu0 = f00 + up*(f10 - f00);
	double fu1 = f01 + up*(f11 - f01);
	return fu0 + vp*(fu1 - fu0);
}

glm::vec3 ggColor(SceneNode *root, glm::vec3 eye, const glm::vec3 ambient, const std::list<Light*> &lights, Ray &ray, int maxhit) {
	glm::vec3 normal;
	glm::vec3 origin_normal;
	const SceneNode *node = NULL;
	double t = 1e9;
	
	for(int i = 0; i < root->totalSceneNodes(); i++) {
		SceneNode *temp = search(*root, i);
		if(temp == NULL)
			continue;
				
		Ray rray;
		rray.origin = temp->invtrans*ray.origin;
		rray.direction = normalize(temp->invtrans*(ray.origin + ray.direction) - temp->invtrans*ray.origin);

		glm::vec3 old_normal = normal;
		glm::vec3 oold_normal = origin_normal;
		double s = getIntersect(rray, *temp, normal);
		origin_normal = normal;
		normal = normalize(transpose(mat3(temp->invtrans))*normal);

		if(s > 0) {
			s = glm::distance(vec3(temp->trans*(rray.origin + s*rray.direction)), vec3(ray.origin));
		}
				
		if(s > 0.1 && s < t) {
			t = s;
			node = temp;
		}
		else {
			normal = old_normal;
			origin_normal = oold_normal;
		}
	}
	
	if((node != NULL) && (t > 0.1)) {
		const GeometryNode *geometry;
		if(node->copy == NULL)
			geometry = static_cast<const GeometryNode*>(node);
		else
			geometry = static_cast<const GeometryNode*>(node->copy);
				
		glm::vec3 col = vec3(0.0f, 0.0f, 0.0f); 
		glm::vec3 P = vec3(ray.origin + t*ray.direction);
		
		if(!geometry->texture)
			col += geometry->m_material->get_kd()*ambient;
		else {
			int type = geometry->m_primitive->getType();
			double u = 0, v = 0;
			if(type == 1) {
				glm::vec3 d = normalize(vec3(node->trans*vec4(geometry->m_primitive->getPos(), 1.0f)) - P);

				u = 0.5 - atan2(d.z, d.x)/2/PI;
				v = 0.5 + asin(d.y)/PI;
			}
			else {
				glm::vec4 pos = node->invtrans*vec4(P, 1.0f);
				if(distance(origin_normal, vec3(0.0f, 0.0f, 1.0f)) < 1e-3) {
					u = 1 - pos.x;
				    v = 1 - pos.y;	
				}
				else if(distance(origin_normal, vec3(0.0f, 0.0f, -1.0f)) < 1e-3) {
					u = pos.x;
					v = pos.y;
				}
				else if(distance(origin_normal, vec3(0.0f, 1.0f, 0.0f)) < 1e-3) {
					u = pos.x;
					v = pos.z;
				}
				else if(distance(origin_normal, vec3(0.0f, -1.0f, 0.0f)) < 1e-3) {
					u = 1 - pos.x;
					v = 1 - pos.z;
				}
				else if(distance(origin_normal, vec3(1.0f, 0.0f, 0.0f)) < 1e-3) {
					u = pos.y;
					v = pos.z;
				}
				else {
					u = 1 - pos.y;
					v = 1 - pos.z;
				}
			}

			glm::vec3 kd;
			kd.r = uvInterpolate(geometry->image, 0, u, v);
			kd.g = uvInterpolate(geometry->image, 1, u, v);
			kd.b = uvInterpolate(geometry->image, 2, u, v);
			col += kd*ambient;
		}

		for(Light *light: lights) {
			for(int i = 0; i < NUM_OF_LIGHTS; i++) {
				glm::vec3 position = light->position;

				if(NUM_OF_LIGHTS > 1) {
					position.x += dis(gen)*LIGHT_DISTANCE;
					position.y += dis(gen)*LIGHT_DISTANCE;
					position.z += dis(gen)*LIGHT_DISTANCE;
				}

				glm::vec3 nnormal;
				Ray ray;
				ray.origin = vec4(position, 1.0f);
				ray.direction = normalize(vec4(P - position, 0.0f));

				double tt = 1e9;
				SceneNode *debug;
				for(int i = 0; i < root->totalSceneNodes(); i++) {
					SceneNode *temp = search(*root, i);
					if(temp == NULL)
						continue;

					Ray rray;
					rray.origin = temp->invtrans*ray.origin;
					rray.direction = normalize(temp->invtrans*(ray.origin + ray.direction) - temp->invtrans*ray.origin);
						
					double s = getIntersect(rray, *temp, nnormal);
				
					if(s > 0) {
						s = glm::distance(vec3(temp->trans*(rray.origin + s*rray.direction)), position);
					}
					if(s > 0 && s < tt) {
						tt = s;
						debug = temp;
					}
				}

				if(debug != node || fabs(tt - glm::distance(position, P))/fabs(glm::distance(position, P)) > 0.01) {
					continue;
				}
					
				glm::vec3 L = normalize(position - P);
				glm::vec3 N = normal;
				glm::vec3 R = normalize(2*dot(L, N)*N - L);
				glm::vec3 V = normalize(eye - P);
				float q = glm::distance(position, P);

				glm::vec3 kd = geometry->m_material->get_kd();
				glm::vec3 ks = geometry->m_material->get_ks();
				float alpha = geometry->m_material->get_alpha();

				if(!geometry->texture) {
					if(geometry->bump) {
						int type = geometry->m_primitive->getType();
						double u = 0, v = 0;
						if(type == 1) {
							glm::vec3 d = normalize(vec3(node->trans*vec4(geometry->m_primitive->getPos(), 1.0f)) - P);

							u = 0.5 - atan2(d.z, d.x)/2/PI;
							v = 0.5 + asin(d.y)/PI;
						}
						else {
							glm::vec4 pos = node->invtrans*vec4(P, 1.0f);
							if(distance(origin_normal, vec3(0.0f, 0.0f, 1.0f)) < 1e-3) {
								u = 1 - pos.x;
								v = 1 - pos.y;	
							}
							else if(distance(origin_normal, vec3(0.0f, 0.0f, -1.0f)) < 1e-3) {
								u = pos.x;
								v = pos.y;
							}
							else if(distance(origin_normal, vec3(0.0f, 1.0f, 0.0f)) < 1e-3) {
								u = pos.x;
								v = pos.z;
							}
							else if(distance(origin_normal, vec3(0.0f, -1.0f, 0.0f)) < 1e-3) {
								u = 1 - pos.x;
								v = 1 - pos.z;
							}
							else if(distance(origin_normal, vec3(1.0f, 0.0f, 0.0f)) < 1e-3) {
								u = pos.y;
								v = pos.z;
							}
							else {
								u = 1 - pos.y;
								v = 1 - pos.z;
							}
						}
						
						glm::vec3 X = normalize(vec3(0.0f, 0.0f, 1.0f));
						glm::vec3 Pu = normalize(cross(X, N));
						glm::vec3 Pv = -normalize(cross(N, Pu));

						double E1 = 1.0/geometry->heightImage.width();
						double E2 = 1.0/geometry->heightImage.height();
						double hu1 = fval(geometry->heightImage, u - E1, v);
						double hu2 = fval(geometry->heightImage, u + E1, v);
						double hv1 = fval(geometry->heightImage, u, v - E2);
						double hv2 = fval(geometry->heightImage, u, v + E2);
						double Fu = (hu2 - hu1)/(2*E1);
						double Fv = (hv2 - hv1)/(2*E2);

						glm::vec3 Nnew = normalize(N + Fu*cross(N, Pv) - Fv*cross(N, Pu));
						glm::vec3 Rnew = normalize(2*dot(L, Nnew)*Nnew - L);
						col += (1/float(light->falloff[0] + light->falloff[1]*q + light->falloff[2]*q*q)*(std::max(dot(L, Nnew), 0.0f)*kd*light->colour + pow(dot(Rnew, V), alpha)*ks*light->colour))/NUM_OF_LIGHTS;
					}
					else {
						col += (1/float(light->falloff[0] + light->falloff[1]*q + light->falloff[2]*q*q)*(std::max(dot(L, N), 0.0f)*kd*light->colour + pow(dot(R, V), alpha)*ks*light->colour))/NUM_OF_LIGHTS;
					}
				}
				else {	
					int type = geometry->m_primitive->getType();
					double u = 0, v = 0;
					if(type == 1) {
						glm::vec3 d = normalize(vec3(node->trans*vec4(geometry->m_primitive->getPos(), 1.0f)) - P);

						u = 0.5 - atan2(d.z, d.x)/2/PI;
						v = 0.5 + asin(d.y)/PI;
					}
					else {
						glm::vec4 pos = node->invtrans*vec4(P, 1.0f);
						if(distance(origin_normal, vec3(0.0f, 0.0f, 1.0f)) < 1e-3) {
							u = 1 - pos.x;
							v = 1 - pos.y;	
						}
						else if(distance(origin_normal, vec3(0.0f, 0.0f, -1.0f)) < 1e-3) {
							u = pos.x;
							v = pos.y;
						}
						else if(distance(origin_normal, vec3(0.0f, 1.0f, 0.0f)) < 1e-3) {
							u = pos.x;
							v = pos.z;
						}
						else if(distance(origin_normal, vec3(0.0f, -1.0f, 0.0f)) < 1e-3) {
							u = 1 - pos.x;
							v = 1 - pos.z;
						}
						else if(distance(origin_normal, vec3(1.0f, 0.0f, 0.0f)) < 1e-3) {
							u = pos.y;
							v = pos.z;
						}
						else {
							u = 1 - pos.y;
							v = 1 - pos.z;
						}
					}
					
					kd.r = uvInterpolate(geometry->image, 0, u, v);
					kd.g = uvInterpolate(geometry->image, 1, u, v);
					kd.b = uvInterpolate(geometry->image, 2, u, v);

					if(geometry->bump) {
						glm::vec3 X = normalize(vec3(0.0f, 0.0f, 1.0f));
						glm::vec3 Pu = normalize(cross(X, N));
						glm::vec3 Pv = -normalize(cross(N, Pu));

						double E1 = 1.0/geometry->heightImage.width();
						double E2 = 1.0/geometry->heightImage.height();
						double hu1 = fval(geometry->heightImage, u - E1, v);
						double hu2 = fval(geometry->heightImage, u + E1, v);
						double hv1 = fval(geometry->heightImage, u, v - E2);
						double hv2 = fval(geometry->heightImage, u, v + E2);
						double Fu = (hu2 - hu1)/(2*E1);
						double Fv = (hv2 - hv1)/(2*E2);

						glm::vec3 Nnew = normalize(N + Fu*cross(N, Pv) - Fv*cross(N, Pu));
						glm::vec3 Rnew = normalize(2*dot(L, Nnew)*Nnew - L);
						col += (1/float(light->falloff[0] + light->falloff[1]*q + light->falloff[2]*q*q)*(std::max(dot(L, Nnew), 0.0f)*kd*light->colour + pow(dot(Rnew, V), alpha)*ks*light->colour))/NUM_OF_LIGHTS;
					}
					else {
						col += (1/float(light->falloff[0] + light->falloff[1]*q + light->falloff[2]*q*q)*(std::max(dot(L, N), 0.0f)*kd*light->colour + pow(dot(R, V), alpha)*ks*light->colour))/NUM_OF_LIGHTS;
					}
				}
			}
		}

		if(maxhit < MAXHIT) {
			Ray rray;
			rray.origin = vec4(P, 1.0f);
			glm::vec3 N = normal;
			glm::vec3 V = normalize(eye - P);
			rray.direction = vec4(normalize(2*dot(V, N)*N - V), 0.0f);
		
			const GeometryNode *geometry;
			if(node->copy == NULL)
				geometry = static_cast<const GeometryNode*>(node);
			else
				geometry = static_cast<const GeometryNode*>(node->copy);
		
			float ior = geometry->m_material->get_ior();
			glm::vec3 I = normalize(P - eye);
			//float kr = fresnel(I, N, ior);
			float kr = 0.5;
			
			if(REFLECTION == 1) {
				if(REFRACTION)
					col += ambient*kr*ggColor(root, eye, ambient, lights, rray, maxhit + 1);
				else
					col += ambient*ggColor(root, eye, ambient, lights, rray, maxhit + 1);
			}
			else if(REFLECTION == 2){
				glm::vec3 s = vec3(0.0f, 0.0f, 0.0f);
				for(int k = 0; k < NUM_OF_REFLECTIONS; k++) {
					double alpha = acos(pow(0.5 - dis(gen)/2, 1.0/(GLOSS + 1)));
					double beta = PI*(dis(gen) + 1);

					glm::vec3 a = vec3(rray.direction);
					glm::vec3 b = vec3(1.0f, 0.0f, 0.0f);
					double theta = acos(dot(a, b));
					glm::vec3 n = normalize(cross(a, b));
					glm::mat4 rot = rotate(mat4(), float(theta), n);

					glm::vec4 Q = rotate(mat4(), float(alpha), vec3(0.0f, 0.0f, 1.0f))*vec4(b, 1.0f);
					Q = inverse(rot)*Q;
					Q = rotate(mat4(), float(beta), a)*Q;
					Q = translate(mat4(), vec3(rray.origin))*Q;

					Ray gloss_ray = rray;
					gloss_ray.direction = normalize(Q - rray.origin);
					s += ambient*kr*ggColor(root, eye, ambient, lights, gloss_ray, maxhit + 1);
				}
				col += s/NUM_OF_REFLECTIONS;
			}
			
			float eta = 1/ior;
			float cosi;
			if((cosi = dot(I, N)) < 0) {
				cosi = -cosi;
			}
			else {
				eta = 1/eta;
				N = -N;
			}

			float k = 1 - eta*eta*(1 - cosi*cosi);
			if(k > 0) {
				Ray rray;
				rray.origin = vec4(P, 1.0f);
				rray.direction = vec4(normalize(eta*I + (eta*cosi - sqrtf(k))*N), 0.0f);
				if(REFRACTION) {
					if(!REFLECTION) {
						col += ambient*ggColor(root, eye, ambient, lights, rray, maxhit + 1);
					}
					else
						col += ambient*(1 - kr)*ggColor(root, eye, ambient, lights, rray, maxhit + 1);
				}
			}
			else {
				return col;
			}
		}

		return col;
	}
	else {
		glm::vec2 v = normalize(vec2(ray.direction));
		double t = (v.y + 1.0f)/2;
		return t*vec3(0.9f, 0.9f, 0.9f) + (1 - t)*(vec3(0.5f, 0.7f, 1.0f));
	}

}

double color_distance(glm::vec3 col1, glm::vec3 col2) {
	return (fabs(col1.r - col2.r) + fabs(col1.g - col2.g) + fabs(col1.b - col2.b))/(col1.r + col1.g + col1.b);
}

void A4_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {

  // Fill in raytracing code here...

  /*std::cout << "Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t h = image.height();
	size_t w = image.width();

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			// Red: increasing from top to bottom
			image(x, y, 0) = (double)y / h;
			// Green: increasing from left to right
			image(x, y, 1) = (double)x / w;
			// Blue: in lower-left and upper-right corners
			image(x, y, 2) = ((y < h/2 && x < w/2)
						  || (y >= h/2 && x >= w/2)) ? 1.0 : 0.0;
		}
	}*/

	system("setterm -cursor off");
	transTree(root);

	int ny = image.height() - 1;
	int nx = image.width() - 1;
	float d = glm::distance(eye, view);
	
	float h = 2*d*tan(fovy*PI/180/2);
	float wd = nx*h/ny;
	
	glm::mat4 T1 = translate(mat4(), vec3(-nx/2.0, -ny/2.0, d));
	glm::mat4 S2 = scale(mat4(), vec3(-wd/nx, -h/ny, 1));
	
	glm::vec3 w = normalize(view - eye);
	glm::vec3 u = normalize(cross(up, w));
	glm::vec3 v = cross(w, u);
	glm::mat4 R3 = mat4(vec4(u, 0.0f), vec4(v, 0.0f), vec4(w, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f));

	glm::mat4 T4 = mat4(vec4(1.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f), vec4(eye, 1.0f));

	for(int y = 0; y <= ny; y++) {
		for(int x = 0; x <= nx; x++) {
			Ray ray;
			image(x, y, 0) = 0;
			image(x, y, 1) = 0;
			image(x, y, 2) = 0;

			for(int k = 0; k < NUM_OF_RAYS; k++) {
				ray.origin = vec4(eye, 1.0f);
				double xx = 0, yy = 0;
				if(NUM_OF_RAYS > 1) {
					do {
						xx = dis(gen);
						yy = dis(gen);
					} while(xx*xx + yy*yy >= 1.0f);

					ray.origin.x += xx*LENS_RADIUS;
					ray.origin.y += yy*LENS_RADIUS;
				}

				glm::vec3 col = vec3(0.0f, 0.0f, 0.0f);
				ray.direction = normalize(T4*R3*S2*T1*vec4(x, y, 0.0f, 1.0f) - ray.origin);

				col += ggColor(root, eye, ambient, lights, ray, 0);
				image(x, y, 0) += col.r;
				image(x, y, 1) += col.g;
				image(x, y, 2) += col.b;
			}

			image(x, y, 0) /= NUM_OF_RAYS;
			image(x, y, 1) /= NUM_OF_RAYS;
			image(x, y, 2) /= NUM_OF_RAYS;
			
			std::cout << std::fixed << setprecision(4) << "\r" << "Progress: " << 100*double(x + 1 + y*(ny + 1))/((nx + 1)*(ny + 1)) << "%";
		}
	}

	std::cout << std::endl;
	system("setterm -cursor on");

	if(ALIASE) {
	Image copy(image);
	for(int y = 0; y <= ny; y++) {
		for(int x = 0; x <= nx; x++) {
			if(x - 1 >= 0) {
				glm::vec3 col1 = vec3(image(x, y, 0), image(x, y, 1), image(x, y, 2));
				glm::vec3 col2 = vec3(image(x - 1, y, 0), image(x - 1, y, 1), image(x - 1, y, 2));

				if(color_distance(col1, col2) > ANTI_RATIO) {
					Ray ray;
					ray.origin = vec4(eye, 1.0f);
					glm::vec3 col = vec3(0.0f, 0.0f, 0.0f);

					for(int i = 0; i < 3; i++) {
						for(int j = 0; j < 3; j++) {
							ray.direction = normalize(T4*R3*S2*T1*vec4(x + i/3.0f, y + j/3.0f, 0.0f, 1.0f) - ray.origin);
							col += ggColor(root, eye, ambient, lights, ray, 0);
						}
					}
							
					//copy(x, y, 0) = 1;
					//copy(x, y, 1) = 0;
					//copy(x, y, 2) = 0;
					copy(x, y, 0) = col.r/9;
					copy(x, y, 1) = col.g/9;
					copy(x, y, 2) = col.b/9;
				}
			}
			
			if(y + 1 <= ny) {
				glm::vec3 col1 = vec3(image(x, y, 0), image(x, y, 1), image(x, y, 2));
				glm::vec3 col3 = vec3(image(x, y + 1, 0), image(x, y + 1, 1), image(x, y + 1, 2));

				if(distance(col1, col3) > ANTI_RATIO) {
					Ray ray;
					ray.origin = vec4(eye, 1.0f);
					glm::vec3 col = vec3(0.0f, 0.0f, 0.0f);

					for(int i = 0; i < 3; i++) {
						for(int j = 0; j < 3; j++) {
							ray.direction = normalize(T4*R3*S2*T1*vec4(x + i/3.0f, y + j/3.0f, 0.0f, 1.0f) - ray.origin);
							col += ggColor(root, eye, ambient, lights, ray, 0);
						}
					}
							
					//copy(x, y, 0) = 1;
					//copy(x, y, 1) = 0;
					//copy(x, y, 2) = 0;
					copy(x, y, 0) = col.r/9;
					copy(x, y, 1) = col.g/9;
					copy(x, y, 2) = col.b/9;
				}
			}
			
			if(y - 1 >= 0) {
				glm::vec3 col1 = vec3(image(x, y, 0), image(x, y, 1), image(x, y, 2));
				glm::vec3 col3 = vec3(image(x, y - 1, 0), image(x, y - 1, 1), image(x, y - 1, 2));

				if(distance(col1, col3) > ANTI_RATIO) {
					Ray ray;
					ray.origin = vec4(eye, 1.0f);
					glm::vec3 col = vec3(0.0f, 0.0f, 0.0f);

					for(int i = 0; i < 3; i++) {
						for(int j = 0; j < 3; j++) {
							ray.direction = normalize(T4*R3*S2*T1*vec4(x + i/3.0f, y + j/3.0f, 0.0f, 1.0f) - ray.origin);
							col += ggColor(root, eye, ambient, lights, ray, 0);
						}
					}
							
					//copy(x, y, 0) = 1;
					//copy(x, y, 1) = 0;
					//copy(x, y, 2) = 0;
					copy(x, y, 0) = col.r/9;
					copy(x, y, 1) = col.g/9;
					copy(x, y, 2) = col.b/9;
				}
			}
			
			if(x + 1 <= nx) {
				glm::vec3 col1 = vec3(image(x, y, 0), image(x, y, 1), image(x, y, 2));
				glm::vec3 col2 = vec3(image(x + 1, y, 0), image(x + 1, y, 1), image(x + 1, y, 2));

				if(distance(col1, col2) > ANTI_RATIO) {
					Ray ray;
					ray.origin = vec4(eye, 1.0f);
					glm::vec3 col = vec3(0.0f, 0.0f, 0.0f);

					for(int i = 0; i < 3; i++) {
						for(int j = 0; j < 3; j++) {
							ray.direction = normalize(T4*R3*S2*T1*vec4(x + i/3.0f, y + j/3.0f, 0.0f, 1.0f) - ray.origin);
							col += ggColor(root, eye, ambient, lights, ray, 0);
						}
					}
							
					//copy(x, y, 0) = 1;
					//copy(x, y, 1) = 0;
					//copy(x, y, 2) = 0;
					copy(x, y, 0) = col.r/9;
					copy(x, y, 1) = col.g/9;
					copy(x, y, 2) = col.b/9;
				}
			}
		}
	}

	image = copy;
	}
}
