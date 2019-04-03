#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"
#include "Material.hpp"
#include "Image.hpp"

class GeometryNode : public SceneNode {
public:
	GeometryNode( const std::string & name, Primitive *prim, 
		Material *mat = nullptr );

	void setMaterial( Material *material );
	void setTexture(const std::string & filename);
	void setBump(const std::string & filename);

	Material *m_material;
	Primitive *m_primitive;
	bool texture;
	Image image;

	bool bump;
	Image heightImage;
};
