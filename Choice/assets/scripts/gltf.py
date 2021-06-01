import bpy
import os

def main():
    
    #Open Temporary File Containing Model Path To Convert
    Temporary = open("Temporary")

    #Read Contained Path In Temporary
    ModelToExtract = Temporary.read()

    #Extract Extension To Use Similar Import Function
    FilepathData, Extension = os.path.splitext(ModelToExtract)
    Extension = Extension.lower()

    #Import Scenes
    if Extension == ".obj":
        bpy.ops.import_scene.obj(filepath = ModelToExtract)
    elif Extension == ".fbx":
        Directory = os.path.dirname(ModelToExtract)
        bpy.ops.import_scene.fbx(filepath = ModelToExtract, directory = Directory)
    elif Extension == ".x3d":
        bpy.ops.import_scene.x3d(filepath = ModelToExtract, axis_forward = '-Z')
    elif Extension == ".dae":
        bpy.ops.wm.collada_import(filepath = ModelToExtract)
    
    #Delete The Default Cube
    ToDelete = bpy.data.objects['Cube']
    bpy.data.objects.remove(ToDelete, do_unlink = True)
    #Delete The Default Light
    ToDelete = bpy.data.objects['Light']
    bpy.data.objects.remove(ToDelete, do_unlink = True)
    #Delete The Default Camera
    ToDelete = bpy.data.objects['Camera']
    bpy.data.objects.remove(ToDelete, do_unlink = True)

    #Export Gltf Scene
    ExportPath = FilepathData + ".glb"
    bpy.ops.export_scene.gltf(filepath = ExportPath)

if __name__ == '__main__':
    main()