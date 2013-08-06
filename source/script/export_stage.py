import bpy

import math
from math import *

def export_stage():
	
	text = ""
	
	os = bpy.context.selected_objects
	
	for o in os:
		name = o.data.name
		name = name.replace( "-line", "" )

		# if name[-2:] == "-0":
		#   name = name[:-2]
		
		if name == "player" or name == "goal":
			text += name + " "
			text += str( o.location[ 0 ] ) + " "
			text += str( o.location[ 2 ] ) + " "
			text += str( o.location[ 1 ] )
			
			if name == "player" and o.location[ 2 ] >= 30:
				text += " 60"
			
			text += "\n"
			continue
		
		r = int( floor( degrees( o.rotation_euler[2] ) + 0.5 ) )
		r2 = int( floor( degrees( o.rotation_euler[1] ) + 0.5 ) )
		r3 = int( floor( degrees( o.rotation_euler[0] ) + 0.5 ) )
		
		while r < 0:
			r += 360
		while r >= 360:
			r -= 360
		
		if r > 0:
			r = 360 - r;
		
		if name == "area-switch":
			text += name + " "
			text += str( o.location[ 0 ] ) + " "
			text += str( o.location[ 2 ] ) + " "
			text += str( o.location[ 1 ] ) + " "
			text += str( o.dimensions[ 0 ] ) + " "
			text += str( o.dimensions[ 2 ] ) + " "
			text += str( o.dimensions[ 1 ] ) + " "
		elif name == "robot" or name == "balloon" or name == "rocket" or name == "umbrella" or name == "medal" or name == "stone":
			text += name + " "
			text += str( o.location[ 0 ] ) + " "
			text += str( o.location[ 2 ] ) + " "
			text += str( o.location[ 1 ] ) + " "
			text += str( r ) + " " + str( r2 ) + " " + str( r3 )
		else:
			if o.name.startswith( "dynamic" ):
				text += "dynamic-object "
			else:
				text += "object "
	
			text += name + " "
			text += str( o.location[ 0 ] ) + " "
			text += str( o.location[ 2 ] ) + " "
			text += str( o.location[ 1 ] ) + " "
			text += str( r )
	
			if o.name.startswith( "dynamic" ):
				text += " " + str( r2 ) + " " + str( r3 )
		
		text += "\n"
	
	commands = o.name.split( ', ' )[1:]
	
	for c in commands:
		text += c + "\n"
	
	try:
		to = bpy.data.texts[ "exported_stage" ]
	except:
		to = bpy.data.texts.new( "exported_stage" )
	
	to.clear()
	to.write( text )

export_stage()