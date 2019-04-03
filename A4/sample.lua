stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20)
yellow = gr.material({1.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, 0)
red = gr.material({1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 50)

scene = gr.node('scene')
scene:rotate('X', 10)

cow_poly = gr.mesh('cow', 'cow.obj')
factor = 2.0/(2.76+3.637)
cow_poly:set_material(hide)
cow_poly:translate(0.0, 3.637, 0.0)
cow_poly:scale(factor, factor, factor)
cow_poly:translate(0.0, -1.0, 0.0)

plane = gr.mesh('plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(grass)
plane:scale(40, 30, 30)
plane:rotate('Y', 10)
plane:rotate('Z', -2)
plane:translate(0.0, -3.0, 0.0)

buckyball = gr.mesh( 'buckyball', 'buckyball.obj' )
scene:add_child(buckyball)
buckyball:set_material(red)
buckyball:translate(0.0, 5.0, 0.0)

box = gr.cube('box')
scene:add_child(box)
box:set_material(stone)
box:translate(-0.5, -0.5, -0.5)
box:scale(5, 5, 5)

moon = gr.sphere('moon')
scene:add_child(moon)
moon:set_material(yellow)
moon:translate(10, 10, -10)

cow_number = 1
for _, pt in pairs({60, 180, 300}) do
   cow_instance = gr.node('cow' .. tostring(cow_number))
   scene:add_child(cow_instance)
   cow_instance:add_child(cow_poly)
   cow_instance:rotate('Y', -90)
   cow_instance:scale(3.0, 3.0, 3.0)
   cow_instance:translate(0.0, 0.0, -10.0)
   cow_instance:rotate('Y', pt)
   
   cow_number = cow_number + 1
end

gr.render(scene,
	  'sample.png', 500, 500,
	  {10, 10, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, 
	  {gr.light({10, 10, -5}, {1.0, 1.0, 0.0}, {1, 0, 0})})
