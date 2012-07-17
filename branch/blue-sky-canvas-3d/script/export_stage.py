import bpy

import math
from math import *

def export_stage():
	
	text = ""
	
	os = bpy.context.selected_objects
	
	for o in os:
		name = o.data.name
		
		if name[-2:] == "-0":
			name = name[:-2]
		
		if name == "player" or name == "goal":
			text += name + " "
			text += str( int( floor( o.location[ 0 ] * 10 + 0.5 ) ) ) + " "
			text += str( int( floor( o.location[ 2 ] * 10 + 0.5 ) ) ) + " "
			text += str( int( floor( o.location[ 1 ] * 10 + 0.5 ) ) )
			
			if name == "player" and o.location[ 2 ] >= 30:
				text += " 60"
			
			text += "\n"
			continue
		
		r = int( floor( degrees( o.rotation_euler[2] ) + 0.5 ) )
		
		while r < 0:
			r += 360
		while r >= 360:
			r -= 360
		
		if r > 0:
			r = 360 - r;
		
		if name == "enemy" or name == "balloon" or name == "rocket" or name == "umbrella" or name == "medal":
			text += name + " "
			text += "%f " % ( o.location[ 0 ] * 10 )
			text += str( o.location[ 2 ] * 10 ) + " "
			text += str( o.location[ 1 ] * 10 ) + " "
			text += str( r );
			text += "\n"
			continue
			
		text += "object "
		text += name + " "
		text += str( int( floor( o.location[ 0 ] * 10 + 0.5 ) ) ) + " "
		text += str( int( floor( o.location[ 2 ] * 10 + 0.5 ) ) ) + " "
		text += str( int( floor( o.location[ 1 ] * 10 + 0.5 ) ) ) + " "
		text += str( r )
		
		if o.name == "s":
			text += " 1.0 0.666 0.066"
		elif o.name == "g":
			text += " 0.666 1.0 0.666"
		
		text += "\n"
	
	try:
		to = bpy.data.texts[ "exported_stage" ]
	except:
		to = bpy.data.texts.new( "exported_stage" )
	
	to.clear()
	to.write( text )

export_stage()