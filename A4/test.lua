-- A simple scene with five spheres

mat1 = gr.material({0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 0)
mat2 = gr.material({0.7, 0.2, 0.1}, {0.3, 0.5, 0.5}, 0)

scene_root = gr.node('root')

s1 = gr.nh_sphere('s1', {0, 0, 0}, 200)
scene_root:add_child(s1)
s1:set_material(mat1)

s2 = gr.nh_sphere('s2', {350, 0, -100}, 150)
scene_root:add_child(s2)
s2:set_material(mat2)

white_light = gr.light({-100.0, 150.0, 400.0}, {0.9, 0.9, 0.9}, {1, 0, 0})

gr.render(scene_root, 'simple.png', 256, 256,
	  {0, 0, 800}, {0, 0, 400}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light})
