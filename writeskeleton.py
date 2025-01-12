#Write Skeleton script, for Total War (Warhammer 3), by Lt. Rocky and Hexagon
#Purpose: Writing out the bones of a skeleton in Blender into a format that can be serialized into a .anim skeleton file

import bpy
import math
import mathutils
import numpy
#from math import sqrt

obj = bpy.context.active_object
if obj.type == 'ARMATURE':

	#print(obj.name)		
	
	TWBones=open(bpy.path.abspath("//Asset SRC\\"+obj.name+".txt"),'w')
	
	#Write the header
	TWBones.write('8\n1\n20\n'+obj.name+'\n0\n0.1\n')
	
	#get the total number of bones in the skeleton
	numBones = 0
	boneList = []
	for bone in obj.data.bones:
		numBones+=1
		boneList.append(str(bone.name))

	#Print the bone list
	TWBones.write(str(numBones)+'\n')
	boneIndex = 0
	for bone in obj.data.bones:
		#print('%s, %s' %(bone.name, bone.parent.name))
		boneParentIndex = 0
		if bone.parent is None:
			boneParentIndex = -1
		else:
			for ind, x in enumerate(boneList):
				#print(str(bone.parent.name))
				#print(str(p))
				if str(bone.parent.name) == x:
					#print('found a parent match')
					boneParentIndex = ind
					
		TWBones.write(str(bone.name)+' '+str(boneIndex)+' '+str(boneParentIndex)+'\n')
		boneIndex+=1
	
	TWBones.write(str(numBones)+'\n')
	TWBones.write(str(numBones)+'\n') 

	TWBones.write('3\n')
	
	quaternionList = []
	
	#Create new skeleton
	#bpy.ops.object.mode_set(mode='EDIT',toggle=True)
	bpy.ops.object.armature_add(radius=1, enter_editmode=False, align='WORLD', location=(0, 0, 0), scale=(1, 1, 1))
	
	#Set the new skeleton as the active object
	#QuatSkele = bpy.data.armatures[-1]
	QuatSkele = bpy.context.active_object
	
	#give the skeleton a 90 degree tilt on X
	QuatSkele.rotation_euler.x = -1.5708
	bpy.ops.object.transform_apply(location=False, rotation=True, scale=False)
	QuatSkele.rotation_euler.x = 1.5708
	QuatSkele.scale[0] = 0.0254
	QuatSkele.scale[1] = 0.0254
	QuatSkele.scale[2] = 0.0254
	
	
	
	#set to Edit Mode; can't add bones otherwise
	bpy.ops.object.mode_set(mode='EDIT',toggle=False)
	
	#make new bones in new skeleton
	for bone in obj.data.bones:
		#rename the first bone created with the armature to animroot, instead of making a new one
		if bone.name == "animroot":
			QuatSkele.data.edit_bones[0].name = "animroot"
		else:
			newBone = QuatSkele.data.edit_bones.new(str(bone.name))
			newBone.tail.y = 39.3701
			if bone.parent is not None:
				newBone.parent = QuatSkele.data.edit_bones[bone.parent.name]
	
		
	bpy.ops.object.mode_set(mode='OBJECT',toggle=False)
	
	# Deselect all objects
	bpy.ops.object.select_all(action='DESELECT')
	
	#select both original and new skeleton
	for o in bpy.data.objects:
		# Check for given object names
		if o.name in (QuatSkele.name,obj.name):
			o.select_set(True)
			
	#set original skeleton as active object again
	bpy.context.view_layer.objects.active = obj	
	objSelected = bpy.context.active_object
	
	#Add constraints to all bones in the new skeleton to bind them to the old skeleton
	for objs in bpy.context.selected_objects: #Go through all selected objects
		if objs.name != objSelected.name: #Check that the object in the cycle is not active object
			for objBones in objs.pose.bones: #Cycle through every bone in the current armature
				for selBones in objSelected.pose.bones: #Cycle through every bone in the active armature 
					if (objBones.name == selBones.name): #Check if the two bones have the same name
						cLocation = objBones.constraints.new('COPY_LOCATION')
						cLocation.target = objSelected
						cLocation.subtarget = selBones.name
						#cLocation.invert_x = True

						
						cRotation = objBones.constraints.new('COPY_ROTATION')
						cRotation.target = objSelected
						cRotation.subtarget = selBones.name
						#cRotation.invert_y = True
						#cRotation.invert_z = True
						
	# Deselect all objects, again
	bpy.ops.object.select_all(action='DESELECT')
	#set new skeleton as active object again
	bpy.context.view_layer.objects.active = QuatSkele
	
	#visually apply the transformation for the bones
	bpy.ops.object.mode_set(mode='POSE',toggle=False)
	bpy.ops.pose.select_all()
	bpy.ops.pose.visual_transform_apply()

	#record Quaternions into new skele, finally		
	for bones in QuatSkele.pose.bones:
		quaternionList.append(bones.rotation_quaternion)
		#print('Bone: '+bones.name+'; X: '+str(bones.rotation_quaternion.x)+' Y: '+str(bones.rotation_quaternion.y)+' Z: '+str(bones.rotation_quaternion.z)+' W: '+str(bones.rotation_quaternion.w))
		
	
	bpy.ops.object.mode_set(mode='OBJECT',toggle=False)
	bpy.ops.object.delete(use_global=False)

	
	#set old skeleton as active object again
	bpy.context.view_layer.objects.active = obj

	
	currentFrame = 0
	for frames in range(3):	
		
		#the fun part; print the 3d vectors and orientation for each bone
		boneIndex = 0
		for bone in obj.data.bones:
			#print('printing pos for ['+str(boneIndex)+']:'+str(bone.name))
			# -X; Y; Z
			TWBones.write(str(format((-bone.head.x*0.0254),'.6f'))+' '+str(format((bone.head.y*0.0254),'.6f'))+' '+str(format((bone.head.z*0.0254),'.6f'))+'\n')
			boneIndex+=1

		boneIndex = 0
		bpy.ops.object.mode_set(mode='EDIT',toggle=True)
		for bone in obj.data.edit_bones:
		#for bone in obj.pose.bones:
			#print('printing rot for ['+str(boneIndex)+']:'+str(bone.name))
			
			
			#q = mathutils.Quaternion()
			#q = bone.rotation_quaternion
			#q = bone.head.cross(bone.tail)
			#w = math.sqrt((bone.head.length**2) * (bone.tail.length**2)) + numpy.dot(bone.head, bone.tail)
			
			#q = mathutils.Quaternion(bone.vector, math.radians(bone.roll)) # dunno how you'd find roll, but this is how you'd get the vector
			q = bone.matrix.to_quaternion()
			
			TWBones.write(str(format(quaternionList[boneIndex].x,'.6f'))+' '+str(format(-quaternionList[boneIndex].y,'.6f'))+' '+str(format(-quaternionList[boneIndex].z,'.6f'))+' '+str(format(quaternionList[boneIndex].w,'.6f'))+'\n')
			boneIndex+=1
			# X -> Y, Y -> -X, Z  ?
		currentFrame+=1
		bpy.ops.object.mode_set(mode='OBJECT',toggle=True)
