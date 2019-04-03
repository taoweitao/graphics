#pragma once

#include <glm/glm.hpp>

#include "Material.hpp"

class PhongMaterial : public Material {
public:
  PhongMaterial(const glm::vec3& kd, const glm::vec3& ks, double shininess, double ior);
  virtual ~PhongMaterial();
  virtual glm::vec3 get_kd();
  virtual glm::vec3 get_ks();
  virtual double get_alpha();
  virtual double get_ior();

private:
  glm::vec3 m_kd;
  glm::vec3 m_ks;

  double m_shininess;
  double m_ior;
};
