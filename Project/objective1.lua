mat1 = gr.material({1.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, 10, 0.5)
mat2 = gr.material({1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 3, 1)

scene_root = gr.node('root')

s1 = gr.cylinder('s1')
scene_root:add_child(s1)
s1:set_material(mat1)
s1:rotate('x', 90)
s1:scale(300, 10, 400)

s2 = gr.cube('s2')
scene_root:add_child(s2)
s2:set_material(mat1)
s2:scale(50, 500, 50)
s2:translate(-25, -500, -25)

s3 = gr.cylinder('s3')
scene_root:add_child(s3)
s3:set_material(mat2)
s3:rotate('x', 90)
s3:scale(50, 50, 50)
s3:translate(0, 50, 0)

s4 = gr.cone('s4')
scene_root:add_child(s4)
s4:set_material(mat2)
s4:rotate('x', 90)
s4:scale(50, 75, 50)
s4:translate(0, 175, 0)

s5 = gr.cone('s5')
scene_root:add_child(s5)
s5:set_material(mat2)
s5:scale(50, 50, 100)
s5:rotate('y', -120)
s5:rotate('z', -30)
s5:translate(-50, 10, 300)

s6 = gr.cylinder('s6')
scene_root:add_child(s6)
s6:set_material(mat2)
s6:scale(30, 30, 50)
s6:rotate('y', -40)
s6:translate(100, 40, 300)

s7 = gr.cube('s7')
scene_root:add_child(s7)
s7:set_material(mat2)
s7:set_texture('coast.png')
s7:scale(2500, 3000, 20)
s7:translate(-1250, -1500, -1500)

white_light = gr.light({400, 400, -100}, {0.7, 0.7, 0.7}, {1, 0, 0})
gr.render(scene_root, 'objective1.png', 256, 256,
	  {0, 200, 800}, {0, 0, -400}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light})
