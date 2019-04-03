mat1 = gr.material({1.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, 10, 0.5)
mat2 = gr.material({1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 3, 1)

scene_root = gr.node('root')

s1 = gr.cube('s1')
scene_root:add_child(s1)
s1:set_material(mat1)
s1:set_texture('wood.png')
s1:scale(600, 10, 800)
s1:translate(-300, -5, -400)

s2 = gr.cube('s2')
scene_root:add_child(s2)
s2:set_material(mat1)
s2:set_texture('wood.png')
s2:scale(50, 500, 50)
s2:translate(-25, -500, -25)

s3 = gr.sphere('s3')
scene_root:add_child(s3)
s3:set_material(mat2)
s3:scale(100, 100, 100)
s3:translate(-150, 100, 200)
s3:set_texture('earth.png')

s4 = gr.cube('s4')
scene_root:add_child(s4)
s4:set_material(mat2)
s4:set_texture('panda.png')
s4:scale(150, 150, 150)
s4:translate(-75, -75, -75)
s4:rotate('y', 45)
s4:rotate('x', 30)
s4:translate(150, 120, 200)

s7 = gr.cube('s7')
scene_root:add_child(s7)
s7:set_material(mat2)
s7:set_texture('coast.png')
s7:scale(2500, 3000, 20)
s7:translate(-1250, -1500, -1500)

white_light = gr.light({400, 400, 0}, {0.7, 0.7, 0.7}, {1, 0, 0})
gr.render(scene_root, 'objective2.png', 256, 256,
	  {0, 200, 800}, {0, 0, -400}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light})
