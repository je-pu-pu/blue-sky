import bpy

def export_model( filepath ):
	f = open(filepath, 'w')
	obj = bpy.context.selected_objects[0]
	mesh = obj.data
	
	if obj.mode == 'OBJECT':
		bpy.ops.object.editmode_toggle()
		
	bpy.ops.mesh.quads_convert_to_tris()
	bpy.ops.object.editmode_toggle()
	
	# v
	for vertex in mesh.vertices:
		f.write( "v %f %f %f\n" % ( vertex.co.x, vertex.co.y, vertex.co.z ) )
	
	# f
	for polygon in mesh.polygons:
		f.write( "f %d, %d, %d\n" % ( polygon.vertices[ 0 ], polygon.vertices[ 1 ], polygon.vertices[ 2 ] ) )

	# UVs
	if mesh.uv_textures:
		index = 0
		for face in mesh.uv_textures[0].data:
			f.write( "%d { %f, %f }\n" % ( index, face.uv1.x, 1.0 - face.uv1.y ) )
			index += 1
			f.write( "%d { %f, %f }\n" % ( index, face.uv2.x, 1.0 - face.uv2.y ) )
			index += 1
			f.write( "%d { %f, %f }\n" % ( index, face.uv3.x, 1.0 - face.uv3.y ) )
			index += 1
	
	f.close()

export_model( "robot.txt" )