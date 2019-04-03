#pragma once
#include <glm/glm.hpp>

class Material {
public:
  virtual ~Material();
  virtual glm::vec3 get_kd() = 0;
  virtual glm::vec3 get_ks() = 0;
  virtual double get_alpha() = 0;

protected:
  Material();
};
