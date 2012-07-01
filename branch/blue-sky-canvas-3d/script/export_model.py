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
	if ( mesh.tessface_uv_textures.active != None ):
		for uv in mesh.tessface_uv_textures.active.data:
			output += "uv %f %f\n" % ( uv.uv1[0], 1 - uv.uv1[ 1 ] )
	else:
		for vertex in mesh.vertices:
			output += "uv 0 0\n"
	
	# f
	for polygon in mesh.polygons:
		output += "f %d/%d %d/%d %d/%d\n" % (
			polygon.vertices[ 0 ] + 1, polygon.vertices[ 0 ] + 1,
			polygon.vertices[ 1 ] + 1, polygon.vertices[ 2 ] + 1,
			polygon.vertices[ 2 ] + 1, polygon.vertices[ 2 ] + 1 )
	
	try:
		to = bpy.data.texts[ "exported_model" ]
	except:
		to = bpy.data.texts.new( "exported_model" )			
		
	to.clear()
	to.write( output )
	
export_model()