import bpy

def export_model():
	obj = bpy.context.selected_objects[ 0 ]
	
	mesh = obj.data
	mesh.update( calc_tessface = True )
	
	output = ""
	
	# v
	for vertex in mesh.vertices:
		output += "v %f %f %f\n" % ( vertex.co.x, vertex.co.y, vertex.co.z )
	
	# uv
	for uv in mesh.tessface_uv_textures.active.data:
		output += "uv %f %f\n" % ( uv.uv1[0], uv.uv1[ 1 ] )
	
	# f
	for polygon in mesh.polygons:
		output += "f %d, %d, %d\n" % ( polygon.vertices[ 0 ], polygon.vertices[ 1 ], polygon.vertices[ 2 ] )
	
	try:
		to = bpy.edit_text.get( "exported_model" )
	except:
		to = bpy.edit_text.new( "exported_model" )
			
	to.clear()
	to.write( text )
	
export_model()
