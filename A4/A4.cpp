#include <glm/ext.hpp>
#include <stack>
#include <iomanip>

#include "A4.hpp"
#include "GeometryNode.hpp"
using namespace std;
using namespace glm;

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

glm::vec3 ggColor(SceneNode *root, glm::vec3 eye, const glm::vec3 ambient, const std::list<Light*> &lights, Ray &ray, int maxhit) {
	glm::vec3 normal;
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
		double s = getIntersect(rray, *temp, normal);
		normal = normalize(transpose(mat3(temp->invtrans))*normal);

		if(s > 0) {
			s = distance(vec3(temp->trans*(rray.origin + s*rray.direction)), vec3(ray.origin));
		}
				
		if(s > 0.1 && s < t) {
			t = s;
			node = temp;
		}
		else
			normal = old_normal;
	}
	
	if((node != NULL) && (t > 0.1)) {
		const GeometryNode *geometry;
		if(node->copy == NULL)
			geometry = static_cast<const GeometryNode*>(node);
		else
			geometry = static_cast<const GeometryNode*>(node->copy);
				
		glm::vec3 col = vec3(0.0f, 0.0f, 0.0f); 
		col += geometry->m_material->get_kd()*ambient;
		glm::vec3 P = vec3(ray.origin + t*ray.direction);
		for(Light *light: lights) {
			glm::vec3 nnormal;
			Ray ray;
			ray.origin = vec4(light->position, 1.0f);
			ray.direction = normalize(vec4(P - light->position, 0.0f));

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
					s = distance(vec3(temp->trans*(rray.origin + s*rray.direction)), light->position);
				}
				if(s > 0 && s < tt) {
					tt = s;
					debug = temp;
				}
			}

			if(debug != node || fabs(tt - distance(light->position, P))/fabs(distance(light->position, P)) > 0.005) {
				continue;
			}
					
			glm::vec3 L = normalize(light->position - P);
			glm::vec3 N = normal;
			glm::vec3 R = normalize(2*dot(L, N)*N - L);
			glm::vec3 V = normalize(eye - P);
			float q = distance(light->position, P);

			glm::vec3 kd = geometry->m_material->get_kd();
			glm::vec3 ks = geometry->m_material->get_ks();
			float alpha = geometry->m_material->get_alpha();

			col += 1/float(light->falloff[0] + light->falloff[1]*q + light->falloff[2]*q*q)*(std::max(dot(L, N), 0.0f)*kd*light->colour + pow(dot(R, V), alpha)*ks*light->colour);
		}
		
		if(maxhit < 5) {
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
		
			glm::vec3 ks = geometry->m_material->get_ks();
			col += ks*ggColor(root, eye, ambient, lights, rray, maxhit + 1);
			
			glm::vec3 I = normalize(P - eye);

			float eta = 1/1.3;
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
				col += 0.8*ggColor(root, eye, ambient, lights, rray, maxhit + 1);
			}
			else
				return col;
		}

		return col;
	}
	else {
		glm::vec2 v = normalize(vec2(ray.direction));
		double t = (v.y + 1.0f)/2;
		return t*vec3(1.0f, 1.0f, 1.0f) + (1 - t)*(vec3(0.5f, 0.7f, 1.0f));
	}

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
	float d = distance(eye, view);
	
	float h = 2*d*tan(fovy*3.1415926/180/2);
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
			ray.origin = vec4(eye, 1.0f);

			glm::vec3 col = vec3(0.0f, 0.0f, 0.0f);
			for(int i = 0; i < 1; i++) {
				for(int j = 0; j < 1; j++) {
					ray.direction = normalize(T4*R3*S2*T1*vec4(x + i/3.0f, y + j/3.0f, 0.0f, 1.0f) - ray.origin);
					col += ggColor(root, eye, ambient, lights, ray, 0);
				}
			}

			image(x, y, 0) = col.r/1;
			image(x, y, 1) = col.g/1;
			image(x, y, 2) = col.b/1;
			
			std::cout << std::fixed << setprecision(4) << "\r" << "Progress: " << 100*double(x + 1 + y*(ny + 1))/((nx + 1)*(ny + 1)) << "%";
		}
	}

	std::cout << std::endl;
	system("setterm -cursor on");
}
