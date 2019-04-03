#include "PhongMaterial.hpp"

PhongMaterial::PhongMaterial(
	const glm::vec3& kd, const glm::vec3& ks, double shininess, double ior )
	: m_kd(kd)
	, m_ks(ks)
	, m_shininess(shininess)
	, m_ior(ior)
{}

PhongMaterial::~PhongMaterial()
{}
  
glm::vec3 PhongMaterial::get_kd() { return m_kd; }
glm::vec3 PhongMaterial::get_ks() { return m_ks; }
double PhongMaterial::get_alpha() { return m_shininess; }
double PhongMaterial::get_ior() { return m_ior; }
